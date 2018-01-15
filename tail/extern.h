#include <stdio.h>

#define WR(p, size) \
  if ( write(STDOUT_FILENO, p, size) != size ) \
    oerr();

enum STYLE {
  NOTSET = 0,
  FBYTES,
  FLINES,
  RBYTES,
  RLINES,
  REVERSE
};

void forward( FILE *, enum STYLE, off_t, struct stat *);
void reverse( FILE *, enum STYLE, off_t, struct stat *);

int displaybytes( FILE *, off_t );
int displaylines( FILE *, off_t )

void xerr( int fatal, const char *fmt, ...);
void xerrx( int fatal, const char *fmt, ...);
void ierr( void );
void oerr( void );

extern int fflag, rflag, rval;
extern const char *fname;
