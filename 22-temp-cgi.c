#include <unistd.h>
#include <stdio.h>

static int __argc__;
static char** __argv__;

static void sql_select_title(FILE* ff, const char* title)
{
    fprintf(ff, "select \"\n- %s՝\";\n", title);
}

static void sql_global_min(FILE* ff)
{
    sql_select_title(ff, "Բոլոր չափումների ամենացուրտը");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select min(value) from data) order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_global_max(FILE* ff)
{
    sql_select_title(ff, "Բոլոր չափումների ամենատաքը");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select max(value) from data) order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_24_hour_min(FILE* ff)
{
    sql_select_title(ff, "Վերջին 24 ժամի ամենացուրտը");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select min(value) from data where time > datetime(\'now\', \'localtime\', \'-1 day\')) and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_24_hour_max(FILE* ff)
{
    sql_select_title(ff, "Վերջին 24 ժամի ամենատաքը");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select max(value) from data where time > datetime(\'now\', \'localtime\', \'-1 day\')) and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_24_hour_negative(FILE* ff)
{
    sql_select_title(ff, "Վերջին 24 ժամի 0-ից ցածրները");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where value < 0 and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_24_hour_all(FILE* ff)
{
    sql_select_title(ff, "Վերջին 24 ժամի բոլոր չափումները");
    const char sql[] = "select \"-- \"||time||\"  \"||value||\"\" from data where time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void sql_quit(FILE* ff)
{
    const char sql[] = ".quit\n";
    fwrite(sql, sizeof(sql) - 1, 1, ff);
}

static void f_dump_general(FILE* ff)
{
    sql_global_min(ff);
    sql_global_max(ff);
    sql_24_hour_min(ff);
    sql_24_hour_max(ff);
    sql_24_hour_negative(ff);
    sql_24_hour_all(ff);
}

static void prepare_sql(void (*proc)(FILE*))
{
    int fds[2] = {0, 0};
    pipe(fds);
    close(STDIN_FILENO);
    dup2(fds[0], STDIN_FILENO);
    FILE* ff = fdopen(fds[1], "a");
    proc(ff);
    sql_quit(ff);
    fclose(ff);
}

static void http_header()
{
    fprintf(stdout, "Content-type: text/plain; charset=UTF-8\n");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    __argc__ = argc;
    __argv__ = argv;
    char *const sql_args[] = { "sqlite3", "-batch", "/home/pi/data/sensor.db", NULL };
    http_header();
    prepare_sql(f_dump_general);
    execvp(sql_args[0], sql_args);

    return 0;
}

