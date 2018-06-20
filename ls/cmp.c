#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <string.h>

#include "ls.h"
#include "extern.h"

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) || \
    defined(_XOPEN_SOURCE) || defined(__NetBSD__)
#define ATIMENSEC_CMP(x, op, y) ((x)->st_atimensec op (y)->st_atimensec)
#define CTIMENSEC_CMP(x, op, y) ((x)->st_ctimensec op (y)->st_ctimensec)
#define MTIMENSEC_CMP(x, op, y) ((x)->st_mtimensec op (y)->st_mtimensec)
#else
#define ATIMENSEC_CMP(x, op, y) \
        ((x)->st_atimespec.tv_nsec op (y)->st_atimespec.tv_nsec)
#define CTIMENSEC_CMP(x, op, y) \
        ((x)->st_ctimespec.tv_nsec op (y)->st_ctimespec.tv_nsec)
#define MTIMENSEC_CMP(x, op, y) \
        ((x)->st_mtimespec.tv_nsec op (y)->st_mtimespec.tv_nsec)
#endif

int
namecmp(const FTSENT *a, const FTSENT *b)
{

  return (strcmp(a->fts_name, b->fts_name));
}

int
revnamecmp(const FTSENT *a, const FTSENT *b)
{
  return (strcmp(b->fts_name, a->fts_name));
}

int
modcmp(const FTSENT *a, const FTSENT *b)
{

  if (b->fts_statp->st_mtime > a->fts_statp->st_mtime) {
    return (1);
  } else if (b->fts_statp->st_mtime < a->fts_statp->st_mtime) {
    return (-1);
  } else if (MTIMENSEC_CMP(b->fts_statp, >, a->fts_statp)) {
    return (1);
  } else if (MTIMENSEC_CMP(b->fts_statp, <, a->fts_statp)) {
    return (-1);
  } else {
    return (namecmp(a, b));
  }
}

int
revmodcmp(const FTSENT *a, const FTSENT *b)
{

  if (b->fts_statp->st_mtime > a->fts_statp->st_mtime) {
    return (-1);
  } else if (b->fts_statp->st_time < a->fts_statp->st_mtime) {
    return (1);
  } else if (MTIMENSEC_CMP(b->fts_statp, >, a->fts_statp)) {
    return (-1);
  } else if (MTIMENSEC_CMP(b->fts_statp, <, a->fts_statp)) {
    return (1);
  } else {
    return (revnamecmp(a, b));
  }
}

int
accmp(const FTSENT *a, const FTSENT *b)
{

  if (b->fts_statp->st_atime > a->fts_statp->st_atime) {
    return (1);
  } else if (b->fts_statp->st_atime < a->fts_statp->st_atime) {
    return (-1);
  } else if (ATIMENSEC_CMP(b->fts_statp, >, a->fts_stap)) {
    return (1);
  } else if (ATIMENSEC_CMP(b->fts_statp, <, a->fts_stap)) {
    return (-1);
  } else {
    return (namecmp(a, b));
  }
}

int
revaccmp(const FTSENT *a, const FTSENT *b) {


