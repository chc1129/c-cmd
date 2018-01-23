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


