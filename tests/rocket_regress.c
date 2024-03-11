#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAXPATH 1024
#define BLKSZ 1024

static int flag_base_dir = 0;
static char *base_dir = NULL;

static void parse_command_args(int argc, char **argv);
static int walk_dir(char *dir_name, char *pattern, int spec);
static char *read_file(const char *filename);
static int exec_test_case(const char *path);

enum {
  WALK_OK = 0,
  WALK_BADPATTERN,
  WALK_NAMETOOLONG,
  WALK_BADIO,
  WALK_TEST_FAILED,
};

static int exec_test_case(const char *path) {
  char *test_content = read_file(path);
  char **rest = &test_content;
  char *line;
  int res = 0;
  int pathlen = strlen(path);
  while ((line = strsep(rest, "\n")) != NULL) {
    char *command_line;
    char command[MAXPATH] = {'\0'};
    if ((command_line = strstr(line, "RUN:")) != NULL) {
      char *commandp = &command_line[sizeof("RUN:")];
      int curr_res = 0;
      char *p = commandp;
      int i = 0;
      while (*commandp && (*commandp == ' ' || *commandp == '\t'))
        ++commandp;

      /* Replace %s with the current file name. */
      while (*p) {
        if (strncmp(p, "%s", 2) == 0) {
          strncpy(&command[i], path, pathlen);
          p += 2;
          i += pathlen;
        } else {
          command[i++] = *p;
          ++p;
        }
      }

      if ((curr_res = system(command)) != 0)
        res = -1;
    }
  }
  return res;
}

int main(int argc, char **argv) {
  int res;

  parse_command_args(argc, argv);

  res = walk_dir(base_dir ? base_dir : "", ".\\.scm$", 0);
  switch (res) {
  case WALK_OK:
    break;
  case WALK_BADPATTERN:
    errx(1, "bad pattern");
  case WALK_NAMETOOLONG:
    errx(1, "file name too long");
  case WALK_TEST_FAILED:
    errx(1, "test failed");
  default:
    errx(1, "unknown error");
  }

  return 0;
}

static void parse_command_args(int argc, char **argv) {
  int c;
  int option_index = 0;
  static struct option long_options[] = {
      {"base-dir", optional_argument, &flag_base_dir, 1},
      {0, 0, 0, 0},
  };

  while (1) {
    c = getopt_long(argc, argv, "", long_options, &option_index);

    /* Detect the end of options. */
    if (c == -1) {
      break;
    }

    switch (c) {
    case 0: {
      if (optarg)
        base_dir = strdup(optarg);
      break;
    }
    default:
      abort();
    }
  }
}

static int walk_recur(const char *dir_name, regex_t *regex, int spec) {
  struct dirent *dir_ent;
  DIR *dir;
  struct stat st;
  char fn[MAXPATH];
  int res = WALK_OK;
  int len = strlen(dir_name);

  if (len >= MAXPATH) {
    return WALK_NAMETOOLONG;
  }

  strcpy(fn, dir_name);
  fn[len++] = '/';

  if (!(dir = opendir(dir_name))) {
    warnx("could not open directory \"%s\"", dir_name);
    return WALK_BADIO;
  }

  errno = 0;

  while ((dir_ent = readdir(dir))) {
    if (strcmp(dir_ent->d_name, ".") == 0 || strcmp(dir_ent->d_name, "..") == 0)
      continue;

    strncpy(fn + len, dir_ent->d_name, MAXPATH - len);
    if (lstat(fn, &st) == -1) {
      warnx("could not stat file \"%s\"", fn);
      res = WALK_BADIO;
      continue;
    }

    if (S_ISDIR(st.st_mode)) {
      walk_recur(fn, regex, spec);
    }

    if (regexec(regex, fn, 0, 0, 0) == 0) {
      if (exec_test_case(fn) != 0) {
        fprintf(stderr, "test: \"%s\" failed\n", fn);
        res = WALK_TEST_FAILED;
        continue;
      }
    }
  }

  if (dir)
    closedir(dir);

  return res ? res : errno ? WALK_BADIO : WALK_OK;
}

static int walk_dir(char *dir_name, char *pattern, int spec) {
  regex_t r;
  int res;
  if (regcomp(&r, pattern, REG_EXTENDED | REG_NOSUB))
    return WALK_BADPATTERN;
  res = walk_recur(dir_name, &r, spec);
  regfree(&r);
  return res;
}

static char *read_file(const char *filename) {
  char *script = malloc(BLKSZ);
  FILE *script_file = NULL;
  uint32_t total_read_size = 0;
  uint32_t curr_read_size = 0;
  uint32_t curr_buffer_size = BLKSZ;

  if (!script)
    errx(1, "OOM! %m");

  script_file = fopen(filename, "r");
  if (!script_file)
    errx(1, "cannot open script file: \"%s\" %m\n", filename);

  while ((curr_read_size = fread(&script[total_read_size], sizeof(char), BLKSZ,
                                 script_file)) > 0) {
    total_read_size += curr_read_size;

    if (total_read_size == curr_buffer_size) {
      curr_buffer_size *= 2;
      script = realloc(script, curr_buffer_size);
    }
  }

  script[total_read_size] = '\0';

  fclose(script_file);
  return script;
}
