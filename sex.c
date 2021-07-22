#include "sex.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define VALCHARS_MAX 512

/**********************************************************************/


static const char *pcur;

char sexpeek(int n) { return *(pcur + n); }
char sexnext(int n) { return *(pcur += n); }


static int as_float(char *str, float *out) {
  char *ptr = NULL;
  float f = strtod(str, &ptr);
  if (!*ptr) *out = f;
  return !*ptr;
}

static int as_long(char *str, long *out) {
  char *ptr = NULL;
  long i = strtol(str, &ptr, 10);
  if (!*ptr) *out = i;
  return !*ptr;
}

static int is_valterm(char c) {
  return isspace(c) || c == '(' || c == ')';
}

static char *read_value(void) {
  char buffer[VALCHARS_MAX + 1];
  int len = 0;
  do {
    buffer[len++] = sexcur();
    if (is_valterm(sexpeek(1))) break;
  } while (sexnext(1));
  buffer[len] = '\0';
  char *str = malloc((len+1)*sizeof(char));
  return strcpy(str, buffer);
}

static char *read_string(void) {
  int len=0, cap=32;
  char *buf = malloc((cap+1) * sizeof(char));
  while (sexpeek(1) && (sexpeek(1) != '"')) {
    if (len >= cap) buf = realloc(buf, (cap *= 2) * sizeof(char) + 1);
    buf[len++] = sexnext(1);
  }
  sexnext(1); /* consume close quote */
  buf[len] = '\0';
  return realloc(buf, (len+1) * sizeof(char));
}

static SexNode *newnode(SexNodeType type) {
  SexNode *n = calloc(1, sizeof(SexNode));
  n->type = type;
  return n;
}

SexNode *sexparse(const char *str) {
  if (*str != '(') return NULL;

  SexNode *tail, *head = NULL;

  for (pcur=str; *pcur && sexnext(1);) {
    SexNode *node = NULL;
    char cur = *pcur;

    while (isspace(cur)) cur = sexnext(1);

    if (!cur) break;        /* end of input */
    if (cur == ')') break;  /* end of list */

    if (cur == '(') {       /* start of list */
      node = newnode(SEX_LIST);
      node->list = sexparse(pcur);
    }
    else if (cur == '"') {  /* start of string */
      node = newnode(SEX_STRING);
      node->vstr = read_string();
    }
    else {                  /* start of value */
      node = newnode(0);
      char *val = read_value();

      if (as_long(val, &node->vint))
        node->type = SEX_INTEGER;
      else if (as_float(val, &node->vdec))
        node->type = SEX_DECIMAL;
      else {
        node->type = SEX_SYMBOL;
        node->vsym = val;
      }
    }

    node->next = NULL;
    if (head == NULL)
      head = tail = node;
    else
      tail = tail->next = node;
  }

  return head;
}


void sexfree(SexNode *node) {
  while (node) {
    switch (node->type) {
      case SEX_LIST: sexfree(node->list); break;
      case SEX_STRING: free(node->vstr); break;
      case SEX_SYMBOL: free(node->vsym); break;
      default: break;
    }
    SexNode *tmp = node;
    node = node->next;
    free(tmp);
  }
}

