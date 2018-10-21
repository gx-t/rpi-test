#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static char* argv_0 = 0;

static int show_usage(int err)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s client <params>\n", argv_0);
    fprintf(stderr, "\t%s server <params>\n", argv_0);
    return err;
}

static int arg_id(const char* id_str)
{
    int id = atoi(id_str);

    if(0 > id)
        return 0;
    if(15 < id)
        return 15;

    return id;
}

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

static int arg_port(const char* port_str)
{
    int port = atoi(port_str);

    if(1024 > port)
        return 1024;
    if(65535 < port)
        return 65535;

    return port;
}

enum {
    CMD_REG = 0,
    CMD_BROADCAST = 1,
};

static int g_sock = -1;

static int client_send_reg(struct sockaddr_in* addr, int id)
{
    uint8_t buff[2] = {CMD_REG, id};
	if(sizeof(buff) != sendto(g_sock, buff, sizeof(buff), 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in))) {
        perror("sendto");
        return 6;
    }
    return 0;
}

static int client_receive_loop()
{
    return 0;
}

//... <client id> <server ip> <server port>
static int client_main(int argc, char* argv[])
{
    int res = 0;
    if(argc != 4) {
        fprintf(stderr, "Missing arguments for client subcommand.\n");
        return show_usage(3);
    }
    int id = arg_id(*++argv);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;

    if((res = arg_ip(*++argv, &addr))) {
        return res;
    }
    addr.sin_port = htons(arg_port(*++argv));

    g_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(0 > g_sock) {
        perror("socket");
        return 5;
    }

    res = client_send_reg(&addr, id);
    if(res)
        return res;
    return client_receive_loop(g_sock);
}

//... <server port>
static int server_main(int argc, char* argv[])
{
    return 0;
}

int main(int argc, char* argv[])
{
    argv_0 = argv[0];
    if(2 > argc) {
        fprintf(stderr, "Missing argument.\n");
        return show_usage(1);
    }
    argc --;
    argv ++;
    if(!strcmp("client", *argv))
        return client_main(argc, argv);
    if(!strcmp("server", *argv))
        return server_main(argc, argv);

    fprintf(stderr, "Unknown subcommand: %s.\n", *argv);
    return show_usage(2);
}

