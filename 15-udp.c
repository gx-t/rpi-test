#include <stdio.h>
#include <string.h>

static char* argv_0 = 0;

static int show_usage(int err)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s client <params>\n", argv_0);
    fprintf(stderr, "\t%s server <params>\n", argv_0);
    return err;
}

static int client_main(int argc, char* argv[])
{
    return 0;
}

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
