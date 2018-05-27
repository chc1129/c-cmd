#include <regex.h>
#include <time.h>

/* node type */
enum ntype {
  N_AND = 1,
  N_AMIN, N_ANEWER, N_ASINCE, N_ATIME, N_CLOSEPAREN, N_CMIN, N_CNEWER,
  N_CSINCE, N_CTIME, N_DEPTH, N_EMPTY, N_EXEC, NEXECDIR, N_EXIT,
  N_EXPR, N_FALSE, N_FLAGS, N_FOLLOW, N_FPRINT, N_FSTYPE, N_GROUP,
  N_INAME, N_INUM, N_IREGEX, N_LINKS, N_LS, N_MINDEPTH, N_MAXDEPTH,
  N_MMIN, N_MTIME, N_NAME, N_NEWER, N_NOGROUP, N_NOT, N_NOUSER, N_OK,
  N_OPENPAREN, N_OR, N_PATH, N_PERM, N_PRINT, N_PRINT0, N_PRINTX,
  N_PRUNE, N_REGEX, N_SINCE, N_SIZE, N_TYPE, N_USER, N_XDEV, N_DELETE
};

/* node definition */
typedef struct _plandata {
  struct _plandata *next;                         /* next node */
  int (*eval)(struct _plandata *, FTSENT *);      /* node evaluation function */
#define F_EQUAL     1
#define F_LESSTHAN  2
#define F_GREATER   3
#define F_NEEDOK    1                             /* exec ok */
#define F_PLUSSET   2                             /* -exec ... {} + */
#define F_MTFLAG    1                             /* fstype */
#define F_MTTYPE    2
#define F_ATLEAST   1                             /* perm */
        int flags;                                /* private flags */
        enum ntype type;                          /* plan node type */
        union {
                u_int32_t _f_data;                /* flags */
                gid_t _g_data;                    /* gid */
                ino_t _i_data;                    /* inode */
                mode_t _m_data;                   /* mode mask */
                nlink_t _ldata;                   /* link count */
                off_t _o_data;                    /* file size */
                time_t _t_data;                   /* time value */
                struct timespec _ts_data;         /* time value */
                uid_t _u_data;                    /* uid */
                short _mt_data;                   /* mount flags */
                struct _plandata *_p_data[2];     /* PLAN trees */
                struct _ex {
                        char **_e_argv;           /* argv array */
                        char **_e_orig;           /* original strings */
                        int *_e_len;              /* allocated lenght */
                        char **_ep_bxp;           /* ptr to 1st addt'l arg */
                        char *_ep_p;              /* current buffer pointer */
                        char *_ep_bbp;            /* begin buffer pointer */
                        char *_ep_ebp;            /* end buffer pointer */
                        int _ep_maxargs;          /* max #args */
                        int _ep_narg;             /* # addt'l args */
                        int _ep_rval;             /* return value */
                } ex;
                char *_a_data[2];                 /* array of char pointers */
                char *_c_data;                    /* cahr pointer */
                int _exit_val;                    /* exit value */
                int _max_data;                    /* tree depth */
                int _min_data;                    /* tree depth */
                regex_t _regexp_data;             /* compiled regexp */
                FILE *_fprint_file;               /* file stream for -fprint */
        } p_un;
} PLAN;

