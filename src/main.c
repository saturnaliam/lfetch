#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

static long get_uptime();
static char* get_user_hostname();

int main(void) {
    char* user_hostname = get_user_hostname();
    long uptime = get_uptime();
    printf("%ld\n", uptime);

    free(user_hostname);
}

// caller takes control of memory, and must free it!
static char* get_user_hostname() {
    char* output = malloc(HOST_NAME_MAX + LOGIN_NAME_MAX);

    char login_name[LOGIN_NAME_MAX];
    int login_res = getlogin_r(login_name, LOGIN_NAME_MAX);

    char hostname[HOST_NAME_MAX];
    int host_res = gethostname(hostname, HOST_NAME_MAX);

    if (login_res != 0 || host_res != 0) {
        perror("error while getting the hostname");
        exit(1);
    }

    snprintf(output, HOST_NAME_MAX + LOGIN_NAME_MAX, "%s@%s", login_name, hostname);

    return output;
}

static long get_uptime() {
    struct sysinfo sys;

    int res = sysinfo(&sys);

    if (res != 0) {
        perror("error while getting sysinfo");
        exit(1);
    }

    return sys.uptime;
}
