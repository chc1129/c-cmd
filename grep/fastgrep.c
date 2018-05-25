#include <sys/cdefs.h>

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "grep.h"

static inline int grep_cmp(const unsigned char *, const unsigned char *, size_t);
static inline void  grep_revstr(unsigned char *, int);

void
fgrepcomp(fastgrep_t *fg, const char *pat)
{
  unsigned int i;

  /* Initialized */
  fg->len = strlen(pat);
  fg->bol = false;
  fg->eol = false;
  fg->reversed = false;

  fg->pattern = (unsigned char *)grep_strdup(pat);

  /* Preprocess pattern. */
  for (i = 0; i <= UCHAR_MAX; i++) {
    fg->qsBc[i] = fg->len;
  }
  for (i = 1; i <= fg->len; i++) {
    fg->qsBc[fg->pattern[i]] = fg->len - i;
  }
}

/*
 * Returns: -1 on failure, 0 on success
 */
int
fastcomp(fastgrep_t *fg, const char *pat)
{
  unsigned int i;
  int firstHalfDot = -1;
  int firstLastHalfDot = -1;
  int hasDot = 0;
  int lastHalfDot = 0;
  int shiftPatternLen;

  /* Initialize. */
  fg->len = strlen(pat);
  fg->bol = false;
  fg->eol = false;
  fg->reversed = false;
  fg->word = wflag;

  /* Remove end-of-line character ('$'). */
  if (fg->len > 0 && pat[fg->len -1] == '$') {
    fg->eol = true;
    fg->len--;
  }

  /* Remove beginning-of-line character ('^'). */
  if (pat[0] == '^') {
    fg->bol = true;
    fg->len--;
    pat++;
  }

  if (fg->len >= 14 &&
      memcmp(pat, "[[:<:]]", 7)  == 0 &&
      memcmp(pat + fg->len - 7, "[[:>:]]", 7) == 0) {
        fg->len -= 14;
        pat += 7;
        /* Word boundary is handled separately in util.c */
        fg->word = true;
  }

  /*
   * pat has been adjusted earlier to not include '^', '$' or
   * the word match character classes at the beginning and ending
   * of the string respectively.
   */
  fg->pattern = grep_malloc(fg->len + 1);
  memcpy(fg->pattern, pat, fg->len);
  fg->pattern[fg->len] = '\0';

  /* Look for ways to cheart...er...avoid the full regex engine. */
  for (i = 0; i < fg->len; i++) {
    /* Can still cheart? */
    if (fg->pattern[i] == '.') {
      hasDot = i;
      if (i < fg->len /2) {
        if (firstHalfDot < 0) {
          /* Closest dot to the beginning */
          firstHalfDot = i;
        }
      } else {
        /* Closest dot to the end of the pattern. */
        lastHalfDot = i;
        if (firstLastHalfDot < 0) {
          firstLastHalfDot = i;
        }
      }
    } else {
      /* Free memory and let others know this is empty. */
      free(fg->pattern);
      fg->pattern = NULL;
      return (-1);
    }
  }

  /*
   * Determine if a reverse search would be faster based on the placment
   * of the dots.
   */
  if ((!(lflag || cflag)) && ((!(fg->bol || fg->eol)) &&
      ((lastHalfDot) && ((firstHalfDot < 0) ||
      ((fg->len - (lastHalfDot + 1)) < (size_t)firstHalfDot)))) &&
      !oflag && !color) {
        fg->reversed = true;
        hasDot = fg->len - (firstHalfDot < 0 ? firstLastHalfDot : firstHalfDot) - 1;
        grep_revstr(fg->pattern, fg->len);
  }

  /*
   * Normal Quick Search would require a shift based on the position the
   * next character after the comparison is within the pattern. With
   * whildcards, the position of the last dot effects the maximum shift
   * distance.
   * The closer to the end the wild card is the slower the search. A
   * reverse version of this algorithm would be useful for wildcards near
   * the end of the string/
   *
   * Examples:
   * Pattern      Max shift
   * -------      ---------
   * this         5
   * .his         4
   * t.is         3
   * th.s         2
   * thi.         1
   */

  /* Adjust the shift based on location of the last dot ('.'). */
  shiftPatternLen = fg->len - hasDot;

  /* Preprocess pattern. */
  for (i = 0; i <= (signed)UCHAR_MAX; i++) {
    fg->qsBc[i] = shiftPatternLen;
  }
  for (i = hasDot + 1; i < fg->len; i++) {
    fg->qsBc[fg->pattern[i]] = fg->len - i;
  }

  /*
   * Put pattern back to normal after pre-processing to allow for easy
   * comparisons later.
   */
  if (fg->reversed) {
    grep_revstr(fg->pattern, fg->len);
  }
  return (0);
}

int
grep_search(fastgrep_t *fg, const unsigned char *data, size_t len, regmatch_t *pmatch)
{
  unsigned int j;
  int ret = REG_NOMATCH;

  if (pmatch->rm_so == (ssize_t)len) {
    return (ret);
  }

  if (fg->bol && pmatch->rm_so != 0) {
    pmatch->rm_so = len;
    pmatch->rm_eo = len;
    return (ret);
  }

  /* No point in going father if we do not have enough data */
  if (len < fg->len) {
    return (ret);
  }

  /* Only try once at the beginning or ending of the line. */
  if (fg->bol || fg->eol) {
    /* Simple text comparison. */
    /* Verify data is >= pattern length before searching on it. */
    if (len >= fg->len) {
      /* Determine where in data to start ssearch at. */
      j = fg->eol ? len - fg->len : 0;
      if (!((fg->bol && fg->eol) && (len != fg->len))) {
        if (grep_cmp(fg->pattern, data + j, fg->len) == -1){
          pmatch->rm_so = j;
          pmatch->rm_eo = j + fg->len;
          ret = 0;
        }
      }
    }
  } else if (fg->reversed) {
    /* Quick Search algorithm. */
    j = len;
    do {
      if (grep_cmp(fg->pattern, data + j - fg->len, fg->len) == -1) {
        pmatch->rm_so = j - fg->len;
        pmatch->rm_eo = j;
        ret = 0;
        break;
      }
      /* Shift if within bounds, otherwise, we are done. */
      if (j == fg->len) {
        break;
      }
      j -= fg->qsBc[data[j - fg->len - 1]];
    } while (j >= fg->len);
  } else {
    /* Quick Search algorithm. */
    j = pmatch->rm_so;
    do {
      if (grep_cmp(fg->pattern, data + j, fg->len) == -1) {
        pmatch->rm_so = j;
        pmatch->rm_eo = j + fg->len;
        ret = 0;
        break;
      }
      /* Shift if within bounds, otherwise, we are done. */
      if (j + fg->len == len) {
        break;
      } else {
        j += fg->qsBc[data[j + fg->len]];
      }
    } while (j <= (len - fg->len));
  }
  return (ret);
}

/*
 * Returns: i >= 0 on failure (position that it failed)
 *          -1 on success
 */
static inline int
grep_cmp(const unsigned char *pat, const unsigned char *data, size_t len)
{
  size_t size;
  wchar_t *wdata, *wpat;
  unsigned int i;

  if (iflag) {
    if ((size = mbstowcs(NULL, (const char *)data, 0)) == ((size_t) - 1)) {
      return (-1);
    }

    wdata = grep_malloc(size * sizeof(wint_t));

    if (mbstowcs(wdata, (const char *)data, size) == ((size_t) - 1)) {
      return (-1);
    }

    wpat = grep_malloc(size * sizeof(wint_t));

    if (mbstowcs(wpat, (const char *)pat, size) == ((size_t) - 1)) {
      return (-1);
    }
    for (i = 0; i < len; i++) {
      if ((towlower(wpat[i]) == towlower(wdata[i])) || ((grepbehave != GREP_FIXED) && wpat[i] == L'.')) {
        continue;
      }
      free(wpat);
      free(wdata);
      return (i);
    }
  } else {
    for (i = 0; i < len; i++) {
      if ((pat[i] == data[i]) || ((grepbehave != GREP_FIXED) && pat[i] == '.')) {
        continue;
      }
      return (i);
    }
  }
  return (-1);
}

static inline void
grep_revstr(unsigned char *str, int len)
{
  int i;
  char c;

  for (i = 0; i < len / 2; i++) {
    c = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = c;
  }
}

