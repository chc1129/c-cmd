#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int bflag, eflag, fflag, lflag, nflag, sflag, tflag, vflag;
static size_t bsize;
static int rval;
static const char *filename;

int main(int argc, char *argv[]) {

  int ch;
  struct flock stdout_lock;

  setprogram( argv[0] );
printf ( "argv[0] = %s", argv[0] );
  // 全てのロケールを環境変数依存に設定
  ( void )setlocale( LC_ALL, "" );

  // getoptでコマンドオプションを判定
  while (( ch = getopt(argc, argv, "B:beflnstuv" )) != -1) {
      switch ( ch ) {
      case 'B':
          bsize = (size_t) strtol(optarg, NULL, 0);
          break;
      case 'b':
          bflag = nflag = 1; /* -b は -n と同義 */
          break;
      case 'e':
          eflag = vflag = 1; /* -e は -v と同義 */
          break;
      case 'f':
          fflag = 1;
          break;
      case 'l':
          lflag = 1;
          break;
      case 'n':
          nflag = 1;
          break;
      case 's':
          sflag = 1;
          break;
      case 't':
          tflag = 1;
          break;
      case 'u':
          uflag = 1;
          break;
      case 'v':
          vflag = 1;
          break;
      default:
      case '?':
      // default と ?の位置は逆がセオリーでは?
      // ってかコレなら'?'は明記しなくても同一ルート通るのだが
      // 以下L66-L68は別関数にする方が見やすい
          ( void )fprintf( stderr, "Usage: %s [-beflnstuv] [-B bsize] [-] " "[file ...]\n", getprogname() );
          return EXIT_FAILURE;
      }
// optindは外部変数
printf("optind: %d\n", optind);
      argv += optind;
  }

// if (a) という様な書き方好きになれないテクがないだけなんだろうけど
// if ( a != FALSE ) とかにして欲しい
    if ( lflag ) {
      stdout_lock.l_len    = 0;
      stdout_lock.l_start  = 0;
      stdout_lock.l_type   = F_WRLCK;
      stdout_lock.l_whence = SEEK_SET;
      if (fcntl(STDOUT_FILENO, F_SETLKW, &stdout_lock ) == -1) {
          err ( EXIT_FAILURE, "stdout" );
      }
    }

    if ( bflag || eflag || nflag || sflag || tflag || vflag ) {
        cook_args( argv );
    } else {
        raw_args( argv );
    }
    if ( fclose( stdout ) ) {
          err( EXIT_FAILURE, "stdout" );
    }

  return rval;
}

void cook_args( char **argv ) {
  FILE *fp;

  fp = stdin;
  filename = "stdin";
  do {
      if ( *argv ) {
        if ( !strcmp( *argv, "-" ) ) {
          fp = stdin;
        } else if (( fp = fopen( * argv, fflag ? "rf" : "r")) == NULL ) {
          warn( "%s", *argv );
          rval = EXIT_FAILURE;
          ++argv;
          continue;
        }
        filename = *argv++;
      }
      cook_buf( fp );
      if ( fp != stdin ) {
        ( void )fclose( fp );
      } else {
        clearerr( fp );
      }
  } while ( *argv );
}

void cook_buf( FILE *fp ) {
  int ch, gobble, line, prev;

  line = gobble = 0;
  for ( prev = '\n'; ( ch = getc( fp ) ) != EOF; prev = ch ) {
    if ( prev == '\n' ) {
      if ( sflag ) {
        if ( ch == '\n' ) {
          if ( gobble ) {
            continue;
          }
          gobble = 1;
        } else {
          gobble = 0;
        }
        if ( nflag ) {
          if ( !bflag || ch != '\n' ) {
            ( void )fprintf( stdout, "%6d\t", ++line );
            if ( ferror( stdout ) ) {
              break;
            }
          } else if ( eflag ) {
            ( void )fprintf( stdout, "%6s\t", "" );
            if ( ferror( stdout ) ) {
              break;
            }
          }
        }
        if ( ch == '\n' ) {
          if ( eflag ) {
            if ( putchar( '$' ) == EOF ) {
              break;
            }
          }
        } else if ( ch == '\t' ) {
          if ( tflag ) {
            if ( putchar( '^' ) == EOF || putchar( 'I' ) == EOF ) {
              break;
            }
            continue;
          }
        } else if ( vflag ) {
          if ( !isascii( ch ) ) {
            if ( putchar( 'M' ) == EOF || putchar( '-' ) == EOF ) {
              break;
            }
            ch = toascii( ch );
          }
          if ( iscntrl( ch )) {
            if ( putchar( '^' ) == EOF || putchar( ch == '177' ? '?' : ch | 0100 ) == EOF ) {
              break;
            }
            continue;
          }
        }
        if ( putchar( ch ) == EOF ) {
          break;
        }
      }
      if ( ferror( fp ) ) {
        warn( "%s", filename );
        rval = EXIT_FAILURE;
        clearerr( fp );
      }
      if ( ferror( stdout )) {
        err( EXIT_FAILURE, "stdout" ) ;
      }
    }
  }
}


