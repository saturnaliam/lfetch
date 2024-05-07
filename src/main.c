#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

typedef struct {
  long seconds;
  long minutes;
  long hours;
} Time;

static void remove_newline(char **input, ssize_t length);
static Time get_time(long seconds);

static long get_uptime();
static char *get_user_hostname();
static char *get_model();
static char *get_os();
static char *get_kernel();

int main(void) {
  char *user_hostname = get_user_hostname();
  char *model = get_model();
  Time time = get_time(get_uptime());

  printf("%s\nos %s\nmodel %s\nkernel %s\nuptime %ld:%02ld:%02ld\n",
         user_hostname, get_os(), model, get_kernel(), time.hours, time.minutes,
         time.seconds);

  free(user_hostname);
  free(model);
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

// caller takes control of memory, and must free it!
static char *get_model() {
  int product_name_file =
      open("/sys/devices/virtual/dmi/id/product_name", O_RDONLY);
  int product_version_file =
      open("/sys/devices/virtual/dmi/id/product_version", O_RDONLY);

  if (product_name_file == -1 || product_version_file == -1) {
    perror("error getting model");
    exit(1);
  }

  char *model_full = (char *)malloc(256 * 2);
  char *product_name = (char *)malloc(256);
  char *product_version = (char *)malloc(256);

  ssize_t bytes_name = read(product_name_file, product_name, 256);
  ssize_t bytes_version = read(product_version_file, product_version, 256);

  remove_newline(&product_name, bytes_name);
  remove_newline(&product_version, bytes_version);

  if (bytes_name == -1 || bytes_version == -1) {
    perror("error reading model files");
    exit(1);
  }

  snprintf(model_full, 256 * 2, "%s %s", product_name, product_version);

  close(product_name_file);
  close(product_version_file);
  free(product_name);
  free(product_version);
  return model_full;
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

// BUG THIS MIGHT LEAK MEMORY
static char *get_os() {
  int os_file = open("/etc/os-release", O_RDONLY);

  if (os_file == -1) {
    perror("error getting operating system");
    exit(1);
  }

  char *os = (char *)malloc(1024);
  ssize_t bytes_read = read(os_file, os, 1024);

  if (bytes_read == -1) {
    perror("error reading os");
    exit(1);
  }

  char *token = strtok(os, "\"=\n");
  while ((token = strtok(0, "\"=\n")) != 0) {
    if (strcmp(token, "PRETTY_NAME") == 0) {
      break;
    }
  }

  token = strtok(0, "\"=\n");

  free(os);
  return token;
}

// the caller MUST free the memory returned!!
static char *get_kernel() {
  struct utsname info;
  int result = uname(&info);

  if (result == -1) {
    perror("error getting kernel");
    exit(1);
  }

  char *output = (char *)malloc(65 * 2);
  snprintf(output, 65 * 2, "%s %s", info.sysname, info.release);

  return output;
}

// some utility functions i use in the program

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

static void remove_newline(char **input, ssize_t length) {
  for (ssize_t i = 0; i < length; i++) {
    if ((*input)[i] == '\n') {
      (*input)[i] = '\0';
      break;
    }
  }
}
