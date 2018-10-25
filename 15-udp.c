#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

/*

static int arg_ip(const char* ip_str, struct sockaddr_in* addr)
{
    struct hostent* he = gethostbyname(ip_str);
    if(!he) {
        perror(ip_str);
        return show_usage(4);
    }

    memcpy(&addr->sin_addr, he->h_addr_list[0], sizeof(addr->sin_addr));

    return 0;
}
static int client_send_reg(struct sockaddr_in* addr, int id)
{
    uint8_t buff[2] = {CMD_REG, id};
    if(sizeof(buff) != sendto(g_sock, buff, sizeof(buff), 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in))) {
        perror("sendto");
        return 6;
    }
    return 0;
}
*/

static void ctrl_c(int sig)
{
    fprintf(stderr, "\nSIGINT (%d)\n", getpid());
    close(0);
}

static int send_loop()
{
    char buff[256];
    while(fgets(buff, sizeof(buff), stdin)) {
    }
    fprintf(stderr, "End of send loop (%d)\n", getpid());

    wait(0);
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
        fprintf(stderr, ">> %d\n", getpid());
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

