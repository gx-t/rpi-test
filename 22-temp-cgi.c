#include <unistd.h>
#include <stdio.h>

static int __argc__;
static char** __argv__;

static const char* sql_select_time_value = "select \"-- \"||time||\"  \"||value||\"\" from data";
static const char* sql_order_by_time_desc = "order by time desc";

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
    const char* cond = "order by rowid desc limit 1";
    fprintf(ff, "%s %s;\n", sql_select_time_value, cond);
}

static void sql_global_min(FILE* ff)
{
    sql_title(ff, "Բոլոր չափումների ամենացուրտը");
    const char* cond = "where value == (select min(value) from data)";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void sql_global_max(FILE* ff)
{
    sql_title(ff, "Բոլոր չափումների ամենատաքը");
    const char* cond = "where value == (select max(value) from data)";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void sql_24_hour_min(FILE* ff)
{
    sql_title(ff, "վերջին 24 ժամի ամենացուրտը");
    const char* cond = "where value == (select min(value) from data where time > datetime('now', 'localtime', '-1 day')) and time > datetime('now', 'localtime', '-1 day')";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void sql_24_hour_max(FILE* ff)
{
    sql_title(ff, "վերջին 24 ժամի ամենատաքը");
    const char* cond = "where value == (select max(value) from data where time > datetime('now', 'localtime', '-1 day')) and time > datetime('now', 'localtime', '-1 day')";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void sql_24_hour_negative(FILE* ff)
{
    sql_title(ff, "վերջին 24 ժամի 0-ից ցածրները");
    const char* cond = "where value < 0 and time > datetime('now', 'localtime', '-1 day')";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void sql_24_hour_all(FILE* ff)
{
    sql_title(ff, "վերջին 24 ժամի բոլոր չափումները");
    const char* cond = "where time > datetime('now', 'localtime', '-1 day')";
    fprintf(ff, "%s %s %s;\n", sql_select_time_value, cond, sql_order_by_time_desc);
}

static void f_dump_general(FILE* ff)
{
    sql_start_end_date(ff);
    sql_last(ff);
    sql_24_hour_min(ff);
    sql_24_hour_max(ff);
    sql_global_min(ff);
    sql_global_max(ff);
//    sql_24_hour_negative(ff);
    sql_24_hour_all(ff);
}

static void prepare_sql(char* buff, size_t buff_size, void (*proc)(FILE*))
{
    FILE* ff = fmemopen(buff, buff_size, "w");
    proc(ff);
    fclose(ff);
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
    prepare_sql(arg_buff, sizeof(arg_buff), f_dump_general);
    char *const sql_args[] = { "sqlite3", "-batch", "/home/pi/data/sensor.db", arg_buff, NULL };
    execvp(sql_args[0], sql_args);

    return 0;
}

