#include "sex.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define VALCHARS_MAX 512

/**********************************************************************/

typedef void (*FreeFn)(void*);

typedef struct Reader {
  struct Reader *next;
  SexReader read;
  FreeFn free;
  char sigil;
} Reader;

static Reader *readers = NULL;

/**********************************************************************/

static const char *pcur;

char sexpeek(int n) { return *(pcur + n); }
char sexnext(int n) { return *(pcur += n); }

int sexterm(char c) { return isspace(c) || c == '(' || c == ')' || !c; }

/**********************************************************************/

static int as_dec(char *str, float *out) {
  char *ptr = NULL;
  float f = strtod(str, &ptr);
  if (!*ptr) *out = f;
  return !*ptr;
}

static int as_int(char *str, long *out) {
  char *ptr = NULL;
  long i = strtol(str, &ptr, 10);
  if (!*ptr) *out = i;
  return !*ptr;
}

static void read_num(SexNode *node) {
  char *val = sexscan(sexterm);
  if (as_int(val, &node->vint))
    node->type = SEX_INTEGER;
  else if (as_dec(val, &node->vdec))
    node->type = SEX_DECIMAL;
  else {
    node->type = SEX_INTEGER;
    node->vint = atoi(val);
  }
}

static void read_sym(SexNode *node) {
  node->vsym = sexscan(sexterm);
}

static void read_str(SexNode *node) {
  int len=0, cap=32;
  char *buf = malloc((cap+1) * sizeof(char));
  while (sexpeek(1) && (sexpeek(1) != '"')) {
    buf[len++] = sexnext(1);
    if (len >= cap) buf = realloc(buf, ((cap <<= 1) + 1) * sizeof(char));
  }
  sexnext(1); /* consume close quote */
  buf[len] = '\0';
  node->vstr = realloc(buf, (len+1) * sizeof(char));
}

static SexNode *read_cur(void);

static void read_list(SexNode *node) {
  SexNode *tail = NULL;
  while (*pcur && sexnext(1)) {
    if (*pcur == ')') break;
    SexNode *next = read_cur();
    if (!node->list)
      node->list = tail = next;
    else
      tail = tail->next = next;
    if (!next) break;
  }
}

static SexNode *read_cur(void) {
  char c = *pcur;
  while (isspace(c)) c = sexnext(1);

  if (!c) return NULL;

  SexNode *node = sexnode(0);

  if (c == '(') {
    node->type = SEX_LIST;
    read_list(node);
  }
  else if (c == '"') {
    node->type = SEX_STRING;
    read_str(node);
  }
  else if (isalpha(c)) {
    node->type = SEX_SYMBOL;
    read_sym(node);
  }
  else if (isdigit(c) || c=='-') {
    read_num(node); /* read_num will set node->type */
  }
  else {
    for (Reader *r=readers; r; r = r->next) {
      if (r->sigil == c) {
        node->type = c;
        r->read(node);
      }
    }
    if (node->type == 0) {
      node->type = SEX_SYMBOL;
      read_sym(node);
    }
  }

  return node;
}

/**********************************************************************/

SexNode *sexnode(char type) {
  SexNode *n = calloc(1, sizeof(SexNode));
  n->type = type;
  return n;
}

char *sexscan(int (*is_term)(char)) {
  char buffer[VALCHARS_MAX + 1];
  int len = 0;
  do {
    buffer[len++] = sexcur();
    if (is_term(sexpeek(1))) break;
  } while (sexnext(1));
  buffer[len] = '\0';
  char *str = malloc((len+1)*sizeof(char));
  return strcpy(str, buffer);
}

SexNode *sexread(const char *str) {
  pcur = str;
  SexNode *node = read_cur();
  pcur = NULL;
  return node;
}

void sexfree(SexNode *node) {
  while (node) {
    switch (node->type) {
      case SEX_INTEGER: break;
      case SEX_DECIMAL: break;
      case SEX_LIST: sexfree(node->list); break;
      case SEX_STRING: free(node->vstr); break;
      case SEX_SYMBOL: free(node->vsym); break;
      default: {
        for (Reader *r=readers; r; r = r->next) {
          if (r->sigil == node->type)
            r->free(node->vusr);
        }
      }
    }
    SexNode *tmp = node;
    node = node->next;
    free(tmp);
  }
}

void sexreader(char sigil, SexReader read, FreeFn free) {
  Reader *r = calloc(1, sizeof(Reader));
  *r = (Reader){.sigil=sigil, .read=read, .free=free};
  Reader *itr = readers;
  if (!itr) {
    readers = r;
  } else {
    while (itr->next) itr = itr->next;
    itr->next = r;
  }
}

