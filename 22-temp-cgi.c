#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static int __argc__;
static char** __argv__;

static void sql_title(FILE* ff, const char* title)
{
    fprintf(ff, "select \"\n- %s՝\";\n", title);
}

static void sql_start_end_date(FILE* ff)
{
    const char* sql = "select '\n-- '||date(min(time), 'localtime')||'...'||date(max(time), 'localtime')||'' from data;\n";
    fprintf(ff, sql);
}

static void sql_last(FILE* ff)
{
    sql_title(ff, "վերջին չափումը");
    const char* sql = "select '-- '||time||'  '||value||'' from data order by time desc limit 1;";
    fprintf(ff, sql);
}

static void sql_global_min(FILE* ff)
{
    sql_title(ff, "Բոլոր չափումների ամենացուրտը");
    const char* sql = "select '-- '||time||'  '||value||'' from data where value == (select min(value) from data);";
    fprintf(ff, sql);
}

static void sql_global_max(FILE* ff)
{
    sql_title(ff, "Բոլոր չափումների ամենատաքը");
    const char* sql = "select '-- '||time||'  '||value||'' from data where value == (select max(value) from data);";
    fprintf(ff, sql);
}

static void sql_24_hour_min(FILE* ff, int offset)
{
    sql_title(ff, "24 ժամի ամենացուրտը");
    const char* sql = "select '-- '||time||'  '||value||'' from data where value == (select min(value) from data where time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day')) and time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day') order by time desc;";
    fprintf(ff, sql, offset - 1, offset, offset - 1, offset);
}

static void sql_24_hour_max(FILE* ff, int offset)
{
    sql_title(ff, "24 ժամի ամենատաքը");
    const char* sql = "select '-- '||time||'  '||value||'' from data where value == (select max(value) from data where time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day')) and time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day') order by time desc;";
    fprintf(ff, sql, offset - 1, offset, offset - 1, offset);
}

static void sql_24_hour_negative(FILE* ff, int offset)
{
    sql_title(ff, "24 ժամի 0-ից ցածրները");
    const char* sql = "select '-- '||time||'  '||value||'' from data where value < 0 and time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day') order by time desc;";
    fprintf(ff, sql, offset - 1, offset, offset - 1, offset);
}

static void sql_24_hour_all(FILE* ff, int offset)
{
    sql_title(ff, "24 ժամի բոլոր չափումները");
    const char* sql = "select '-- '||time||'  '||value||'' from data where time between datetime('now', 'localtime', '%d day') and datetime('now', 'localtime', '%d day') order by time desc;";
    fprintf(ff, sql, offset - 1, offset, offset - 1, offset);
}

static void f_dump_general(FILE* ff)
{
    char* query = getenv("QUERY_STRING");
    if(query && *query)
    {
        int offset = atoi(query);
        sql_24_hour_min(ff, offset);
        sql_24_hour_max(ff, offset);
        sql_24_hour_all(ff, offset);
        return;
    }
    sql_start_end_date(ff);
    sql_last(ff);
    sql_24_hour_min(ff, 0);
    sql_24_hour_max(ff, 0);
    sql_global_min(ff);
    sql_global_max(ff);
    sql_24_hour_all(ff, 0);
}

static int prepare_sql(char* buff, size_t buff_size, void (*proc)(FILE*))
{
    FILE* ff = fmemopen(buff, buff_size, "w");
    if(!ff)
    {
        perror("fmemopen");
        return 1;
    }
    proc(ff);
    fclose(ff);
    return 0;
}

static void http_header()
{
    fprintf(stdout, "Content-type: text/plain; charset=UTF-8\n");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    char arg_buff[0x1000];
    __argc__ = argc;
    __argv__ = argv;
    http_header();
    if(prepare_sql(arg_buff, sizeof(arg_buff), f_dump_general))
        return 1;
    char *const sql_args[] = { "sqlite3", "-batch", "/home/pi/data/sensor.db", arg_buff, NULL };
    return execvp(sql_args[0], sql_args);
}

