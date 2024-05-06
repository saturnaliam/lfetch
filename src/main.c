#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

typedef struct {
  long seconds;
  long minutes;
  long hours;
} Time;

static Time get_time(long seconds);

static long get_uptime();
static char *get_user_hostname();
static char *get_model();

int main(void) {
  char *user_hostname = get_user_hostname();

  free(user_hostname);
}

// caller takes control of memory, and must free it!
static char *get_user_hostname() {
  char *output = malloc(HOST_NAME_MAX + LOGIN_NAME_MAX);

  char login_name[LOGIN_NAME_MAX];
  int login_res = getlogin_r(login_name, LOGIN_NAME_MAX);

  char hostname[HOST_NAME_MAX];
  int host_res = gethostname(hostname, HOST_NAME_MAX);

  if (login_res != 0 || host_res != 0) {
    perror("error while getting the hostname");
    exit(1);
  }

  snprintf(output, HOST_NAME_MAX + LOGIN_NAME_MAX, "%s@%s", login_name,
           hostname);

  return output;
}

static char *get_model() {}

static long get_uptime() {
  struct sysinfo sys;

  int res = sysinfo(&sys);

  if (res != 0) {
    perror("error while getting sysinfo");
    exit(1);
  }

  return sys.uptime;
}

static Time get_time(long seconds) {
  long seconds_remaining = seconds % 3600;

  seconds -= seconds_remaining;
  Time time = {.hours = seconds / 3600};

  seconds = seconds_remaining;
  seconds_remaining = seconds % 60;

  time.minutes = (seconds - seconds_remaining) / 60;
  time.seconds = seconds_remaining;

  return time;
}
