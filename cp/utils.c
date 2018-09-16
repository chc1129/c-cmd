#include <sys/cdefs.h>

#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/extattr.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#define MMAP_MAX_SIZE   (8 * 1048576)
#define MMAP_MAX_WRITE  (64 * 1024)

int
set_utimes(const char *file, struct stat *fs)
{
  struct timespec ts[2];

  ts[0] = fs->st_atimespec;
  ts[1] = fs->st_mtimespec;

  if (lutimens(file, ts)) {
    warn("lutimes: %s", file);
    return (1);
  }
  return (0);
}

struct finfo {
  const char *from;
  const char *to;
  off_t size;
};

static void
progress(const struct finfo *fi, off_t written)
{
  int pcent = (int)((100.0 * written) / fi->size);

  pinof = 0;
  (void)fprintf(stderr, "%s => %s %llu/%llu bytes %d%% written\n", fi->from, fi->to, (unsigned long long)written, (unsigned long long)fi->size, pcent);
}

int
copy_file(FTSENT *entp, int dne)
{
  static char buf[MAXBSIZE];
  struct stat to_stat, *fs;
  int ch, checkch, from_fd, rcount, rval, to_fd, tolnk, wcount;
  char *p;
  off_t ptotal = 0;

  /* if hard linking then simply link and return */
  if (lflag) {
    (void)unlink(to.p_path);
     if (link(entp->fts_path, to.p_path)) {
       warn("%s", to.p_path);
       return (1);
     }
     return (0);
  }

  if ((from_fd = open(entp->fts_path, O_RDONLY, 0)) == -1) {
    warn("%s", entp->fts_path);
    return (1);
  }

  to_fd = -1;
  fs = entp->fts_statp;
  tolnk = ((Rflag && !(Lflag || Hflag)) || Pflag);

  /*
   * If the file exists and we're interactive, verify with the user.
   * If the file DNE, set the mode to be the from file, minus setuid
   * bits, modified by the umask; arguably wrong, but it makes copyin
   * executables work right and it's been that way forever. (The
   * other choice is 666 or'ed with the execute bits on the from file
   * modified by the umask.)
   */
  if (!dne) {
    struct stat sb;
    int sval;

    if (iflag) {
      (void)fprintf(stderr, "overwrite %s?", to.p_path);
      checkch = ch = getchar();
      while (ch != '\n' && ch != EOF) {
        ch = getchar();
      }
      if (checkch != 'y' && checkch != 'Y') {
        (void)close(from_fd);
        return (0);
      }
    }

    sval = tolnk ? lstat(to.p_path, &sb) : stat(to.p_path, &sb);
    if (sval == -1) {
      warn("stat: %s", to.p_path);
      (void)close(from_fd);
      return (1);
    }
    if (!(tolnk && S_ISLNK(sb.st_mode))) {
      to_fd = open(to.p_path, O_WRONLY | O_TRUNC, 0);
    }
  } else {
    to_fd = open(to.p_path, O_WRONLY | O_TRUNC | O_CREAT, fs->st_mode & =(S_ISUID | S_ISGID));
  }

