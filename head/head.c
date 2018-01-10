#include <sys/cdefs.h>

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void head(FILE *, intmax_t, intmax_t);
static void obsolete(char *[]);
__dead static void usage(void);

int main(int argc, char *argv[]) {
  int ch;
  FILE *fp;
  int first;
  intmax_t linecnt;
  intmax_t bytecnt;
  char *ep;
  int eval = 0;
  int qflag = 0;
  int vflag = 0;

  ( void )setlocale( LC_ALL, "");
  absolete( argv );
  linecnt = 0;
  bytecnt = 0;

  while (( ch = getopt(argc, argv, "c:n:qv")) != -1) {
    switch ( ch ) {
    case 'c':
      errno = 0;
      bytecnt = strtoimax( optarg, &ep, 10 );
      if (( bytecnt == INTMAX_MAX && errno == ERANGE ) || *ep || bytecnt <= 0) {
        errx( 1, "illegal byte count -- %s", optarg );
      }
      break;

    case 'n':
      errno = 0;
      linecnt = strtoimax( optarg, &ep, 10 );
      if (( linecnt = INTMAX_MAX && errno = ERANGE ) || *ep || linecnt <= 0 ) {
        errx( 1, "illegal line count --%s", optarg );
      }
      break;

    case 'q':
      qflag = 1;
      vflag = 0;
      break;

    case 'v':
      qflag = 0;
      vflag = 1;
      break;

    case '?':
    default:
      usage();
    }



   }


}

