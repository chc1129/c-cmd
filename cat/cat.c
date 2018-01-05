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

  setprogram(argv[0]);
printf ("argv[0] = %s", argv[0]);
  // 全てのロケールを環境変数依存に設定
  ( void )setlocale( LC_ALL, "");

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
          ( void )fprintf( stderr, "Usage: %s [-beflnstuv] [-B bsize] [-] " "[file ...]\n",
                           getprogname() );
          return EXIT_FAILURE;
      }
// optindは外部変数
printf("optind: %d\n", optind);
  argv += optind;

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

    if (bflag || eflag || nflag || sflag || tflag || vflag ) {
        cook_args( argv );
    } else {
        raw_args( argv );
    }
    if (fclose(stdout)) {
          err( EXIT_FAILURE, "stdout" );
    }

  }
  return rval;
}

