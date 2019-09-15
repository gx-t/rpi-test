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

static int g_run = 1;

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    g_run = 0;
}

//static void l0_fill_rand(struct SOCK_ACTION_CTX* ctx)
//{
//    int i = 0;
//    for(; i < sizeof(ctx->buff); i += 4) {
//        *(int*)(ctx->buff + i) = rand();
//    }
//}
//

//static void dump_buff(uint8_t buff[0x20])
//{
//    int i = 0;
//    for(; i < 0x20; i ++) {
//        fprintf(stderr, "%02X", buff[i]);
//    }
//    fprintf(stderr, "\n");
//}

int client_main(int argc, char* argv[])
{
    int res = 0;
    if(argc != 3)
        return 1;

    argc --;
    argv ++;
    int ss = socket(AF_INET, SOCK_DGRAM, 0);
    if(ss < 0) {
        perror("socket");
        return 3;
    }
    struct timeval tv = {3, 0};
    if(0 > setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval))) {
        perror("setsockopt");
        close(ss);
        return 4;
    }

    int port = atoi(argv[1]);
    if(port < 1024 || port > (1 << 16) - 1)
        port = 27727;

    while(g_run) {

        struct sockaddr_in addr = {.sin_family = AF_INET};
        struct hostent* he = gethostbyname(argv[0]);
        if(!he) {
            perror("gethostbyname");
            res = 5;
            break;
        }

        addr.sin_addr.s_addr = *(uint32_t*)he->h_addr_list[0];
        addr.sin_port = htons(port);

        uint8_t buff[32];
        socklen_t addr_len = sizeof(addr);
        if(sizeof(buff) != sendto(ss, buff, sizeof(buff), 0, (struct sockaddr *)&addr, addr_len)) {
            if(!g_run) {
                perror("sendto");
                res = 6;
            }
            break;
        }

        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if(sizeof(buff) != recvfrom(ss, buff, sizeof(buff), 0, (struct sockaddr*)&addr, &addr_len))
            continue;

        fprintf(stderr, "%s:%d:\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    close(ss);
    return res;
}

int server_main(int argc, char* argv[])
{
    int res = 0;
    if(argc != 2)
        return 1;

    argc --;
    argv ++;

    struct sockaddr_in addr = {.sin_family = AF_INET};

    int ss = socket(AF_INET, SOCK_DGRAM, 0);
    if(ss < 0) {
        perror("socket");
        return 3;
    }

    struct timeval tv = {3, 0};
    if(0 > setsockopt(ss, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval))) {
        perror("setsockopt");
        close(ss);
        return 4;
    }

    int port = atoi(*argv);
    if(port < 1024 || port > (1 << 16) - 1)
        port = 27727;

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(ss, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(ss);
        return 4;
    }

    while(g_run) {

        uint8_t buff[32];
        socklen_t addr_len = sizeof(addr);
        if(sizeof(buff) != recvfrom(ss, buff, sizeof(buff), 0, (struct sockaddr*)&addr, &addr_len))
            continue;
        fprintf(stderr, "%s:%d:\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }

    close(ss);
    return res;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
        return 1;
    argc --;
    argv ++;
    signal(SIGINT, ctrl_c);
    srand(time(0));
    if(!strcmp("client", *argv))
        return client_main(argc, argv);
    if(!strcmp("server", *argv))
        return server_main(argc, argv);
    return 2;
}

