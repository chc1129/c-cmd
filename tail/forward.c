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


