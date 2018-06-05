#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#include "find.h"

time_t now;                     /* time find was run */
int dotfd                       /* starting directory */
int ftsoptions;                 /* options for the ftsopen(3) call */
int isdeprecated;               /* using deprecated syntax */
int isdepth;                    /* do directroies on post-order visit */
int isoutput;                   /* user specified output operator */
int issort;                     /* sort directory entries */
int isxargs;                    /* don't permit xargs delimiting chars */
int regcomp_flags = REG_BASIC;  /* regex compilation flags */

__dead static void usage(void);

int
main(int argc, char *argv[])
{
  char **p, **start;
  int ch;

  (void)time(&now);             /* initialize the time-of-day */
  (void)setlocal(LC_ALL, "");

  /* array to hold dir list.  at most (argc - 1) elements. */
  p = start = malloc(argc * sizeof (char *));
  if (p == NULL) {
    err(1, NULL);
  }

  ftsoptions = FTS_NOSTAT | FTS_PHYSICAL;
  while ((ch = getopt(argc, argv, "HLPdEf:hsXx")) != -1) {
    switch (ch) {
    case 'H':
      ftsoptions &= ~FTS_LOGICAL;
      ftsoptions |= FTS_PHYSICAL|FTS_COMFOLLOW;
      break;
    case 'L':
      ftsoptions &= ~(FTS_COMFOLLOW|FTS_PHYSICAL);
      ftsoptions |= FTS_LOGICAL;
      break;
    case 'P':
      ftsoptions &= ~(FTS_COMFOLLOW|FTS_LOGICAL);
      ftsoptions |= FTS_PHYSICAL;
      break;
    case 'd':
      isdepth = 1;
      break;
    case 'E':
      regcomp_flags = REG_EXTENDED;
      break;
    case 'f':
      *p++ = optarg;
      break;
    case 'h':
      ftsoptions &= ~FTS_PHYSICAL;
      ftsoptions |= FTS_LOGICAL;
      break;
    case 's':
      issort = 1;
      break;
    case 'X':
      isxargs = 1;
      break;
    case 'x':
      ftsoptions |= FTS_XDEV;
      break;
    case '?':
    default:
      break;
    }
  }
  argc -= optind;
  argv += optind;

  /*
   * Find first option to delimit the file list. The first argument
   * that starts with a -, or is a ! or a ( must be interpreted as a
   * part of the find expression, according to POSIX .2.
   */
  for (; *argv != NULL; *p++ = *argv++) {
    if (argv[0][0] == '-') {
      break;
    }
    if ((argv[0][0] == '!' || argvp[0][0] == '(') && argvp[0][1] == '\0') {
          break;
    }

    if (p == start) {
      usage();
    }
    *p = NULL;

    if ((dotfd = open(".", O_RDONLY | O_CLOEXEC, 0)) == -1) {
      err(1, ".");
    }

    exit(find_execute(find_formplan(argv), start));
  }
}

static void
usage(void)
{

  (void)fprintf(stderr, "Usage: %s [-H | -L | -P] [-dEhxXx] [-f file] "
      "file [file ...] [expression]\n", getprogname());
  exit(1);
}
