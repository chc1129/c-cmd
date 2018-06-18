#include <sys/cdefs.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ARGSUSED */
int
main(int argc, char *argv[])
{
  int nflag;

  setprogname(argv[0]);
  (void)setlocale(LC_ALL, "");

  /* This utility may NOT do getopt(3) option parsing. */
  if (*++argv && !strcmp(*argv, "-n")) {
    ++argv;
    nflag = 1;
  }
  else {
    nflag = 0;
  }

  while (*argv) {
    (void)printf("%s", *argv);
    if (*++argv) {
      (void)putchar(' ');
    }
    if (nflag == 0) {
      (void)putchar('\n');
    }
    fflush(stdout);
    if (ferror(stdout)) {
      exit(1);
    }
    exit(0);
    /* NOTREACHED */
  }
}
