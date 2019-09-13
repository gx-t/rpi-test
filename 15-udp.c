#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

struct SOCK_ACTION_CTX {
    uint8_t buff[0x100];
    int sock;
    struct sockaddr_in addr;
    int argc;
    char** argv;
};

static int g_run = 1;

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    g_run = 0;
}

static void fill_response(uint8_t buff[0x100])
{
    int i = 0;
    for(; i < 0x100; i += 4) {
        *(int*)&buff[i] = rand();
    }
}

static void dump_buff(uint8_t buff[0x100])
{
    int i = 0;
    for(; i < 0x100; i ++) {
        fprintf(stderr, "%02X", buff[i]);
    }
    fprintf(stderr, "\n");
}

static void set_sock_timeout(int sock, int sec)
{
    struct timeval tv = {sec, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval));
}

static int sock_action(int (*socket_proc)(struct SOCK_ACTION_CTX* ctx), int argc, char* argv[])
{
    struct SOCK_ACTION_CTX ctx = {
        .sock = socket(AF_INET, SOCK_DGRAM, 0),
        .addr.sin_family = AF_INET,
        .argc = argc,
        .argv = argv
    };
    if(0 > ctx.sock) {
        perror("socket");
        return 2;
    }
    int res = socket_proc(&ctx);
    close(ctx.sock);
    return res;
}

static int client_proc(struct SOCK_ACTION_CTX* ctx)
{
    if(2 != ctx->argc)
        return 1;

    while(g_run) {

        socklen_t addr_len = sizeof(ctx->addr);
        struct hostent* he = gethostbyname(ctx->argv[1]);
        if(!he) {
            perror(ctx->argv[1]);
            return 3;
        }

        ctx->addr.sin_addr.s_addr = *(uint32_t*)he->h_addr_list[0];
        ctx->addr.sin_port = htons(27727);
        set_sock_timeout(ctx->sock, 1);

        if(0 != sendto(ctx->sock, ctx->buff, 0, 0, (struct sockaddr *)&ctx->addr, addr_len)) {
            perror("sendto");
            return 4;
        }
        int res = recvfrom(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr*)&ctx->addr, &addr_len);
        if(0 > res)
            continue;

        if(sizeof(ctx->buff) != res) {
            fprintf(stderr, "Proto error: %s:%d:\n", inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port));
            return 5;
        }
        dump_buff(ctx->buff);
    }
    return 0;
}

static int server_proc(struct SOCK_ACTION_CTX* ctx)
{
    socklen_t addr_len = sizeof(ctx->addr);
    ctx->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ctx->addr.sin_port = htons(27727);
    if(bind(ctx->sock, (struct sockaddr *)&ctx->addr, sizeof(ctx->addr)) < 0)
    {
        perror("bind");
        return 3;
    }
    set_sock_timeout(ctx->sock, 3);
    int res = recvfrom(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr*)&ctx->addr, &addr_len);
    if(0 > res) { //probably timout
        return 4;
    }
    if(0 != res) {
        return 5;
    }
    fill_response(ctx->buff);
    if(sizeof(ctx->buff) != sendto(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr *)&ctx->addr, sizeof(ctx->addr))) {
        perror("sendto");
        return 6;
    }
    return 0;
}

static char* norm_cmd(char* argv)
{
    char* res = argv;
    while(*argv) {
        if('/' == *argv ++)
            res = argv;
    }
    return res;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, ctrl_c);
    char* cmd = norm_cmd(*argv);
    srand(time(0));
    if(!strcmp("dev-client", cmd))
        return sock_action(client_proc, argc, argv);
    if(!strcmp("dev-server", cmd))
        return sock_action(server_proc, argc, argv);
    return 1;
}

