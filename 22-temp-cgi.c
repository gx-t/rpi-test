#include <unistd.h>

int main(int argc, char **argv)
{
    int fds[2] = {0, 0};
    char *const args[] = { "sqlite3", "-batch", "/home/pi/data/sensor.db", NULL };
    const char cmd[] = "select \"Content-type: text/plain; charset=UTF-8\n\"\n;"

    "select \"- Բոլոր չափումների ամենացուրտը՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select min(value) from data) order by time desc;\n"

    "select \"\n- Բոլոր չափումների ամենատաքը՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select max(value) from data) order by time desc;\n"

    "select \"\n- Վերջին 24 ժամի ամենացուրտը՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select min(value) from data where time > datetime(\'now\', \'localtime\', \'-1 day\')) and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n"

    "select \"\n- Վերջին 24 ժամի ամենատաքը՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where value == (select max(value) from data where time > datetime(\'now\', \'localtime\', \'-1 day\')) and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n"

    "select \"\n- Վերջին 24 ժամի 0-ից ցածրները՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where value < 0 and time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n"

    "select \"\n- Վերջին 24 ժամի բոլոր չափումները՝\";\n"
    "select \"-- \"||time||\"  \"||value||\"\" from data where time > datetime(\'now\', \'localtime\', \'-1 day\') order by time desc;\n"

    ".quit\n";

    pipe(fds);
    close(STDIN_FILENO);
    dup2(fds[0], STDIN_FILENO);
    write(fds[1], cmd, sizeof(cmd) - 1);
    close(fds[1]);
    execvp(args[0], args);

    return 0;
}
