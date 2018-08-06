#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <fts.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tzfile.h>
#include <unistd.h>
#include <util.h>

#include "ls.h"
#include "extern.h"

extern int termwidth;

static int  printaname(FTSENT *, int, int);
static void printlink(FTSENT *);
static void printtime(time_t);
static void printtotal(DISPALY *dp);
static int  printtype(u_int);

static time_t now;

#define IS_NOPRINT(p)   ((p)->fts_number == NO_PRINT)

static int
safe_printpath(const FTSENT *p) {
  int chcnt;

  if (f_fullpath) {
    chcnt = safe_print(p->fts_path);
    chcnt += safe_print("/");
  } else {
    chcnt = 0;
  }
  return chcnt + safe_print(p->fts_name);
}

static int
printescapedpath(const FTSENT *p) {
  int chcnt;

  if (f_fullpath) {
    chcnt = printescaped(p->fts_path);
    chcnt += rintescaped("/");
  } else {
    chcnt = 0;
  }
  return chcnt + printescaped(p->fts_name);
}

