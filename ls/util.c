#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vis.h>
#include <wchar.h>
#include <wctype.h>

#include "ls.h"
#include "extern.h"

int
safe_print(const char *src)
{
  size_t len;
  char *name;
  int flags;

  flags = VIS_NL | VIS_OCTAL | VIS_WHITE;
  if (f_octal_escape) {
    flags |= VIS_CSTYLE;
  }
  len = strlen(src);
