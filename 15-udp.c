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

struct SOCK_OPER_CTX {
    uint8_t buff[0x100];
    int sock;
    struct sockaddr_in addr;
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

static int socket_action(int (*socket_proc)(struct SOCK_OPER_CTX* ctx))
{
    struct SOCK_OPER_CTX ctx = {
        .sock = socket(AF_INET, SOCK_DGRAM, 0),
        .addr.sin_family = AF_INET
    };
    if(0 > ctx.sock) {
        perror("socket");
        return 3;
    }
    int res = socket_proc(&ctx);
    close(ctx.sock);
    return res;
}

static int client_proc(struct SOCK_OPER_CTX* ctx)
{
    const char* server_ip = "205.166.94.4";

    while(g_run) {

        socklen_t addr_len = sizeof(ctx->addr);
        struct hostent* he = gethostbyname(server_ip);
        if(!he) {
            perror(server_ip);
            return 4;
        }

        ctx->addr.sin_addr.s_addr = *(uint32_t*)he->h_addr_list[0];
        ctx->addr.sin_port = htons(27727);
        set_sock_timeout(ctx->sock, 1);

        if(0 != sendto(ctx->sock, ctx->buff, 0, 0, (struct sockaddr *)&ctx->addr, addr_len)) {
            perror("sendto");
            return 5;
        }
        int res = recvfrom(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr*)&ctx->addr, &addr_len);
        if(0 > res)
            continue;

        if(sizeof(ctx->buff) != res) {
            fprintf(stderr, "Proto error: %s:%d:\n", inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port));
            return 6;
        }
        dump_buff(ctx->buff);
    }
    return 0;
}

static int server_proc(struct SOCK_OPER_CTX* ctx)
{
    socklen_t addr_len = sizeof(ctx->addr);
    ctx->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ctx->addr.sin_port = htons(27727);
    if(bind(ctx->sock, (struct sockaddr *)&ctx->addr, sizeof(ctx->addr)) < 0)
    {
        perror("bind");
        return 4;
    }
    set_sock_timeout(ctx->sock, 3);
    int res = recvfrom(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr*)&ctx->addr, &addr_len);
    if(0 > res) { //probably timout
        return 5;
    }
    if(0 != res) {
        return 6;
    }
    fill_response(ctx->buff);
    if(sizeof(ctx->buff) != sendto(ctx->sock, ctx->buff, sizeof(ctx->buff), 0, (struct sockaddr *)&ctx->addr, sizeof(ctx->addr))) {
        perror("sendto");
        return 7;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if(2 != argc)
        return 1;
    argc --;
    argv ++;
    signal(SIGINT, ctrl_c);
    srand(time(0));
    if(!strcmp("client", *argv))
        return socket_action(client_proc);
    if(!strcmp("server", *argv))
        return socket_action(server_proc);
    return 2;
}

