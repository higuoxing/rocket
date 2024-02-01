#include <stdio.h>
#include <stdlib.h>

#include <readline/history.h>
#include <readline/readline.h>

#define PROMPT_STYLE "> "

/*
 * Strip whitespace from the start and end of str.  Return a pointer
 * into str.
 */
char *stripwhite(char *str) {
  char *s, *t;

  for (s = str; whitespace(*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen(s) - 1;
  while (t > s && whitespace(*t))
    t--;
  *++t = '\0';

  return s;
}

static int rocket_main(int argc, char **argv) {
  char *line = NULL;

  rl_initialize();
  for (;;) {
    char *stripped;
    line = readline(PROMPT_STYLE);
    if (!line) {
      /* Handle ctrl-d */
      break;
    }
    stripped = stripwhite(line);
    if (stripped[0]) {
      fprintf(stdout, "%s\n", stripped);
    }

    free(line);
    line = NULL;
  }

  exit(0);
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
