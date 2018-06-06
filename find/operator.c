#include <sys/cdefs.h>

#include <sys/types.h>

#include <err.h>
#include <fts.h>
#include <stdio.h>

#include "find.h"

static PLAN *yanknode(PLAN **);
static PLAN *yankexpr(PLAN **);

/*
 * yanknode --
 *      destructively removes the top from the plan
 */
static PLAN *
yanknode(PLAN **planp)        /* pointer to top of plan (modified) */
{
  PLAN *node;                 /* top noed removed from the plan */

  if ((node = (*planp)) == NULL) {
    return (NULL);
  }
  (*planp) = (*planp)->next;
  node->next = NULL;
  return (node);
}

/*
 * yankexpr --
 *      Removes one expression from the plan. This is used mainly by
 *      paren_squish. In comments below, an expression is either a
 *      simple node or a N_EXPR node containing a list of simple nodes.
 */
static PLAN *
yankexpr(PLAN **planp)        /* pointer to top of plan (modified) */
{
  PLAN *next;                 /* temp node holding subexpression results */
  PLAN *node;                 /* pointer to returned node or expression */
  PLAN *tail;                 /* pointer to tail of subplan */
  PLAN *subplan;              /* pointer to head of ( ) expression */

  /* first pull the top node from the plan */
  if ((node = yanknode(planp)) == NULL) {
    return (NULL);
  }

  /*
   * If the node is an '(' then we recursively slurp up expressions
   * until we find its associated ')'. If it's a closing paren we
   * just return it and unwind our recursion; all other nodes are
   * complete expressions, so just return them.
   */
  if (node->type == N_OPENPAREN) {
    for (tail = subplan = NULL;;) {
      if ((next = yankexpr(planp)) == NULL) {
        err(1, "(: missing closing ')'");
      }
      /*
       * If we find a closing ')' we store the collected
       * subplan in our '(' node and convert the node to
       * a N_EXPR. The ')' we found is ignored. Otherwise,
       * we just continue to add whatever we get to our
       * subplan.
       */
      if (next->type == N_CLOSEPAREN) {
        if (subplan == NULL) {
          errx(1, "(): empty inner expression");
        }
        node->p_data[0] = subplan;
        node->type = N_EXPR;
        node->eval = f_expr;
        break;
      } else {
        if (subplan == NULL) {
          tail = subplan = next;
        } else {
          tail->next = next;
          tail = next;
        }
        tail->next = NULL;
      }
    }
  }
  return (node);
}
