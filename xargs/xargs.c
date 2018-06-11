#include <sys/cdefs.h>

#include <sys/parm.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <langinfo.h>
#include <locale.h>
#include <paths.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unsitd.h>

#include "pathnames.h"

static void     parse_input(int, char *[]);
static void     prerun(int, char *[]);
static int      prompt(void);
static void     run(char **);
static void     usage(void) __dead;
void            strnsubst(char **, const char *, const char *, size_t);
static void     waitchildren(const char *, int);

