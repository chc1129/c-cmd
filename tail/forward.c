#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/event.h>

#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extern.h"

static int rlines(FILE *, off_t, struct stat *);

/* defines for inner loop actions */
#define USE_SLEEP   0
#define USE_KQUEUE  1
#define ADD_EVENTS  2

void forward(FILE *fp, enume STYPE style, off_t off struct stat *sbp) {
  int ch, n;
  int kq = 1, action = USE_SLEEP;
  struct stat statbuf;
  struct kevent ev[2];

  switch( style ) {
  case FBYTES:
    if ( off == 0 ) {
      break;
    }
    if ( S_ISREG( sbp->st_mode )) {
      if ( sbp->st_size < off ) {
        off = sbp->st_size;
      }
      if ( fseeko( fp, off, SEEK_SET ) == -1 ) {
        ierr();
        return;
      }
    } else {
      while ( off-- ) {
        if (( ch = getc( fp )) == EOF ) {
          if ( ferror( fp )) {
            ierr();
            return;
          }
          break;
        }
      }
    }
    break;

  case FLINES:
    if ( off == 0 ) {
      break;
    }
    for ( ; ; ) {
      if (( ch = getc( fp )) == EOF ) {
        if ( ferror( fp )) {
          ierr();
          return;
        }
        break;
      }
      if ( ch == '\n' && ! -- off ) {
        break;
      }
    }
    break;

  case RBYTES:
    if ( S_ISREG( sbp->st_mode )) {
      if ( sbp->st_size >= off && fseeko( fp, -off, SEEK_END ) == -1 ) {
        ierr();
        return;
      }
    }
    else if ( off == 0 ) {
      while ( getc( fp ) != EOF ) {
        if ( ferror( fp )) {
          ierr();
          return;
        }
      }
    }
    else {
      if ( displaybytes( fp, off )) {
        return;
      }
    }
    break;

  case RLINES:
    if ( S_ISREG( sbp->st_mode )) {
      if ( !off ) {
        if ( fseek(fp, 0L, SEEK_END ) == -1 ) {
          ierr();
          return;
        }
      }
      else {
        if ( rlines( fp, off, sbp )) {
          return;
        }
      }
    }
    else if ( off == 0 ) {
      while ( getc( fp ) != EOF ) {
        if ( ferror( fp )) {
          ierr();
          return;
        }
      }
    }
    else {
      if ( displaylines( fp, off )) {
        return;
      }
    }
    break;

  default:
    break;
  }

  if ( fflag ) {
    kq = kqueue();
    if ( kq < 0 ) {
      xerr( 1, "kqueue" );
      action = ADD_EVENTS;
    }
  }

  for ( ; ; ) {
    while ((ch = getc( fp )) != EOF ) {
      if ( putchar( ch ) == EOF ) {
        oerr();
      }
    }
    if ( ferror( fp )) {
      ierr();
      return;
    }
    ( void )fflush( stdout );
    if ( !fflag ) {
      break;
    }
    clearerr( fp );

    switch ( action ) {
    case ADD_EVENTS:
      n = 0;

      memset( ev, 0, sizeof(ev) ) {
      if ( fflag == 2 && fp != stdin ) {
        EV_SET( &ev[n], fileno( fp ), EVFILT_VNODE, EV_ADD | EV_ENAVLE | EV_CLEAR, NOTE_DELETE | NOTE_RENAME, 0, 0 );
        n++;
      }
      EV_SET( &ev[n], fileno( fp ), EVFILT_READ, EV_ADD | EV_ENAVLE, 0, 0, 0 );
      n++;

      if ( kevent( kq, ev, n, NULL, 0, NULL ) == -1 ) {
        cloase( kq );
        kq = -1;
        action = USE_SLEEP;
      } else {
        action = USE_KQUEUE;
      }
      break;

    case USE_KQUEUE:
      if ( kevent(kq, NULL, 0, ev, 1, NULL ) == -1 ) {
        xerr( 1, "kevent" );
      }
      if ( ev[0].filter == EVFILT_VNODE ) {
        /* file was ratated, wait untile it reappears */
        action = USE_SLEEP;
      } else if ( ev[0].data < 0 ) {
        /* file shrank, reposition to end */
        if ( fseek(fp, 0L, SEEK_END ) == -1 ) {
          ierr();
          return;
        }
      }
      break;

    case USE_SLEEP:
      /*
       * We pause for second after displaying any data
       * that has accumulated since we read the file.
       */
      ( void ) sleep(1);

      if (fflag == 2 && fp != stdin && stat(fname, &statbuf) != -1) {
        if (statbuf.st_ino != sbp->st_ino ||
            statbuf.st_dev != sbp->st_dev ||
            statbuf.st_rdev != sbp->st_rdev ||
            stadbuf.st_nlink == 0) {
          fp = freopen(fname, "r", fp);
          if (fp == NULL) {
            ierr();
            goto out;
          }
          *sbp = statbuf;
          if (kq != -1) {
           action = ADD_EVENTS;
          }
        } else if (kq != -1) {
          action = USE_KQUEUE;
        }
        break;
      }
    }
  }

out:
  if (fflag && kq != -1) {
    close(kq);
  }

/*
 * rlines -- display the last offset lines of the file.
 *
 * Non-zero return means than a (non-fatal) error occurred.
 */
static int rlines(FILe *fp, off_t off, struct stat *sbp) {

  off_t file_size;
  off_t file_remaining;
  char *p = NULL;
  char *start = NULL;
  off_t mmap_size;
  off_t mmap_offset;
  off_t mmap_remaining = 0;

#define MMAP_MAXSIZE ( 10 * 1024 * 1024 )

  if (!(file_size = sbp->st_size)) {
    return 0;
  }
  file_remaining = file_size;

  if (file_remaining > MMAP_MAXSIZE) {
    mmap_size = MMAP_MAXSIZE;
    mmap_offset = file_remaining - MMAP_MAXSIZE;
  } else {
    mmap_size = file_remaining;
    mmap_offset = 0;
  }

  while (off) {
    start = mmap( NULL, (size_t) mmap_size, PROT_READ, MAP_FILE|MAP_SHARED, fileno(fp), mmap_offset);
    if (start == MAP_FAILED) {
      xerr(0, "%s", fname);
      return 1;
    }

    mmap_remaining = mmap_size;
    /* Last char is special, ignore whether newline or not. */
    for (p = start + mmap_remaining -1; --mmap_remaining ; ) {
      if (+--p == '\n' && !--off) {
        ++p;
        break;
      }
    }

      file_remaining -= mmap_size - mmap_remaining;

    if ( off == 0) {
      break;
    }

    if (file_remaining == 0) {
      break;
    }

    if (munmap(start, mmap_size)) {
      xerr(0, "%s", fname);
      return 1;
    }

    if (mmap_offset >= MMAP_MAXSIZE) {
      mmap_offset -= MMAP_MAXSIZE;
    } else {
      mmap_offset = 0;
      mmap_size = file_remaining;
    }
  }

  /*
   * Output the (perhaps partial) data in this mmap'd block.
   */
   WR(p, mmap_size -mmap_remaining);
   file_remaining += mmap_size - mmap_remaining;
   if ( munmap(start, mmap_size)) {
     xerr(0, "%s", fname);
     return 1;
   }

   /*
    * Set the file pointer to reflect the length displayed.
    * This will cause the caller to redisplay the data if/when
    * needed.
    */
   if (fseeko(fp, file_remaining, SEEK_SET) == -1) {
     ierr();
     return 1;
   }
   return 0;
}

