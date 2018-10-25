#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    close(0);
}

static int send_loop()
{
    char buff[256];
    while(fgets(buff, sizeof(buff), stdin)) {
        char* data = 0;
        char* addr_str = strtok_r(buff, " \t", &data);
        char* port_str = strtok_r(0, " \t", &data);
        struct hostent* he = gethostbyname(addr_str);

        if(!he) {
            fprintf(stderr, "Cannot find host: %s\n", addr_str);
            continue;
        }

        int port = atoi(port_str);
        if(port < 1024 || port > 65535)
            port = 23456;

        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = *(uint32_t*)he->h_addr_list[0],
            .sin_port = htons(port)
        };

        int len = strlen(data);
        if(len < 1)
            continue;

        if(len != sendto(1, data, len, 0, (struct sockaddr *)&addr, sizeof(addr)))
            fprintf(stderr, "Error sending data\n");

    }

    fprintf(stderr, "End of send loop (%d)\n", getpid());
    wait(0);
    fprintf(stderr, "After wait (%d)\n", getpid());
    return 0;
}

static int recv_loop()
{
    struct sockaddr_in addr = {0};
    uint8_t buff[0x400] = {0};
    int len = 0;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(23456);
    if(bind(0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 3;
    }
    while(-1 != (len = recvfrom(0, buff, sizeof(buff), 0, (struct sockaddr*)&addr, &addr_len))) {
        fprintf(stderr, "%s:%d:\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if(len != write(1, buff, len))
            perror("Write stdout");
    }

    fprintf(stderr, "End of recv loop (%d)\n", getpid());
    return 0;
}

int main()
{
    int ss = -1;
    pid_t pp = 0;
    ss = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 > ss) {
        perror("socket");
        return 1;
    }
    signal(SIGINT, ctrl_c);
    pp = fork();
    if(-1 == pp) {
        perror("fork");
        return 2;
    }
    if(0 == pp) {
        dup2(ss, 0);
        close(ss);
        return recv_loop();
    }
    dup2(ss, 1);
    close(ss);
    return send_loop();
}

