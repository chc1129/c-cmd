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
static void usage(void);

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
  obsolete( argv );
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
      if (( linecnt == INTMAX_MAX && errno == ERANGE ) || *ep || linecnt <= 0 ) {
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

    argc -= optind;
    argv += optind;
  }

  if ( *argv ) {
    for ( first = 1; *argv; ++argv ) {
      if (( fp = fopen( *argv, "r" )) == NULL ) {
        warn( "%s", *argv );
        eval = 1;
        continue;
      }
      if ( vflag || ( qflag == 0 && argc > 1 )) {
        ( void ) printf( "%s==> %s <==\n", first ? "" : "\n", *argv );
        first = 0;
      }
      head( fp, linecnt, bytecnt );
      ( void )fclose( fp );
    }
  } else {
    head( stdin, linecnt, bytecnt );
    exit( eval );
  }
}

static void head( FILE *fp, intmax_t cnt, intmax_t bytecnt ) {
  char buf[65536];
  size_t len, rv, rv2;
  int ch;

  if ( bytecnt ) {
    while ( bytecnt ) {
      len = sizeof( buf );
      if ( bytecnt > ( intmax_t )sizeof( buf )) {
        len = sizeof( buf );
      } else {
        len = bytecnt;
      }
      rv = fread( buf, 1, len, fp );
      if ( rv == 0 ) {
        break; /* Distinguish EOF and error? */
      }
      rv2 = fwrite( buf, 1, rv, stdout );
      if ( rv2 != rv ) {
        if ( feof( stdout )) {
          errx( 1, "EOF on stdout" );
        } else {
          err(1, "failure writing to stdout");
        }
        bytecnt -= rv;
      }
    }
  } else {
    while (( ch = getc( fp )) != EOF ) {
      if ( putchar( ch ) == EOF ) {
        err( 1, "stdout" );
      }
      if ( ch == '\n' && --cnt == 0 ) {
        break;
      }
    }
  }
}

static void obsolete( char *argv[] ) {
  char *ap;

  while (( ap = *++argv )) {
    /* REturn if "--" or not "-[0-9]*" . */
    if ( ap[0] != '-' || ap[1] == '-' || !isdigit(( unsigned char ) ap[1])) {
      return;
    }
    if (( ap = malloc( strlen( *argv ) + 2 )) == NULL ) {
      err( 1, NULL );
    }
    ap[0] = '-';
    ap[1] = '-';
    ( void )strcpy( ap + 2, *argv + 1 );
    *argv = ap;
  }
}

static void usage( void ) {
//  ( void )fprintf( stderr, "usage: %s [-n lines] [file ...]\n", getprogname());
  ( void )fprintf( stderr, "usage: program [-n lines] [file ...]\n");
  exit(1);
}

