#define NO_PRINT    1

extern long bloksize;       /* block size units */

extern int f_accesstime;    /* use time of last access */
extern int f_flags;         /* show flags associated with a file */
extern int f_grouponly;     /* long listing without owner */
extern int f_humanize;      /* humanize size field */
extern int f_commas;        /* separate size field with commas */
extern int f_inode;         /* print inode */
extern int f_longform;      /* long listing format */
extern int f_octal;         /* print octal escapes for nongraphic characters */
extern int f_octal_escape;  /* like f_octal but use C escates if possible */
extern int f_sectime;       /* print the real time for all files */
extern int f_size;          /* list size in short listing */
extern int f_statustime;    /* use time of last mode change */
extern int f_type;          /* add type character for non-regular files */
extern int f_typedir;       /* add type character for directories */
extern int f_fullpath;      /* print full pathname, not filename */
extern int f_leafonly;      /* when recursing, print leaf names only */

typedef struct {
  FTSENT *list;
  u_int64_t btotal;
  u_int64_t stotal;
  int entries;
  unsigned int maxlen;
  int s_block;
  int s_flags;
  int s_group;
  int s_inode;
  int s_nlink;
  int s_size;
  int s_user;
  int s_major;
  int s_minor;
} DISPLAY;

typdef struct {
  char *user;
  char *group;
  char *flags;
  char data[1];
} NAMES;
