#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bzlib.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <zlib.h>

#include "grep.h"

#define MAXBUFSIZ     (32 * 1024)
#define LNBUFBUMP     80  // BUG:BUMP -> DUMP?


