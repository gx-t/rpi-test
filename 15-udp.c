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
    CMD_BROADCAST,

    CMD_END
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

static int server_bind_socket(int port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(g_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 34;
    }
    return 0;
}

struct CLIENT_INFO {
    uint8_t flags;
    struct sockaddr_in addr;
};

static int server_cmd_reg(const uint8_t* buff, int len, const struct sockaddr_in* client_addr, struct CLIENT_INFO* client_db)
{
    if(1 != len) {
        fprintf(stderr, "Protocol error, invalid data length: %d for CMD_REG.\n", len);
        return 35;
    }
    if(15 < *buff) {
        fprintf(stderr, "Protocol error, invalid client id: 0x%02X.\n", *buff);
        return 35;
    }
    client_db[*buff].flags = 1;
    memcpy(&client_db[*buff].addr, client_addr, sizeof(*client_addr));
    return 0;
}

static int server_cmd_broadcast(const uint8_t* buff, int len, const struct sockaddr_in* client_addr, struct CLIENT_INFO* client_db)
{
    return 0;
}

static int server_dispatch_cmd(const uint8_t* buff, int len, const struct sockaddr_in* client_addr, struct CLIENT_INFO* client_db)
{
    int (*cmd_handler_arr[])(const uint8_t*, int, const struct sockaddr_in*, struct CLIENT_INFO*) = {
        [CMD_REG] = server_cmd_reg,
        [CMD_BROADCAST] = server_cmd_broadcast
    };

    if(CMD_END <= *buff) {
        fprintf(stderr, "Unknown command: 0x%02X\n", *buff);
        return 0;
    }

    return cmd_handler_arr[buff[0]](&buff[1], len - 1, client_addr, client_db);
}

static int server_receive(struct CLIENT_INFO* client_db)
{
    struct sockaddr_in client_addr = {.sin_family = AF_INET};
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buff[0x400];
    int len = recvfrom(g_sock, buff, sizeof(buff), 0, (struct sockaddr*)&client_addr, &addr_len);
    if(0 >= len) {
        perror("recvfrom");
        return 36;
    }
    return server_dispatch_cmd(buff, len, &client_addr, client_db);
}

static int client_receive_loop()
{
    return 0;
}

static int server_receive_loop()
{
    struct CLIENT_INFO client_db[16] = {0};
    int res = 0;
    while(!(res = server_receive(client_db)));
    return res;
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

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;

    if((res = arg_ip(*++argv, &server_addr))) {
        return res;
    }
    server_addr.sin_port = htons(arg_port(*++argv));

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 > g_sock) {
        perror("socket");
        return 5;
    }

    do {
        if((res = client_send_reg(&server_addr, id)))
            break;
        if((res = client_receive_loop(g_sock)))
            break;
    } while(0);
    close(g_sock);
    return res;
}

//... <server port>
static int server_main(int argc, char* argv[])
{
    int res = 0;
    if(argc != 2) {
        fprintf(stderr, "Missing arguments for server subcommand.\n");
        return show_usage(32);
    }

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(0 > g_sock) {
        perror("socket");
        return 33;
    }

    do {
        if((res = server_bind_socket(arg_port(*++argv))))
            break;
        if((res = server_receive_loop(g_sock)))
            break;
    } while(0);
    close(g_sock);
    return res;
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

