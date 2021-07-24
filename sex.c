#include "sex.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**********************************************************************/

typedef void (*FreeFn)(void*);

typedef struct Reader {
  SexReader read;
  FreeFn free;
  char sigil;
} Reader;

static Reader *readers[127] = {0};

/**********************************************************************/

static const char *pcur;

char sexlook(int n) { return *(pcur + n); }
char sexmove(int n) { return *(pcur += n); }

int sexterm(char c) { return isspace(c) || c == '(' || c == ')' || !c; }

/**********************************************************************/

static int as_dec(char *str, double *out) {
  char *ptr = NULL;
  double f = strtod(str, &ptr);
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
  char *buf = SEX_MALLOC((cap+1) * sizeof(char));
  while (sexpeek() && (sexpeek() != '"')) {
    buf[len++] = sexnext();
    if (len >= cap) buf = SEX_REALLOC(buf, ((cap <<= 1) + 1) * sizeof(char));
  }
  sexnext(); /* consume close quote */
  buf[len] = '\0';
  node->vstr = SEX_REALLOC(buf, (len+1) * sizeof(char));
}

static SexNode *read_cur(void);

static void read_list(SexNode *node) {
  SexNode *tail = NULL;
  while (*pcur && sexnext()) {
    sexskip();
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
  char c = sexskip();

  if (!c) return NULL;

  SexNode *node = NULL;

  if (c == '(') {
    read_list(node = sexnode(SEX_LIST));
  }
  else if (c == '"') {
    read_str(node = sexnode(SEX_STRING));
  }
  else if (isalpha(c)) {
    read_sym(node = sexnode(SEX_SYMBOL));
  }
  else if (isdigit(c) || c=='-') {
    read_num(node = sexnode(0)); /* read_num will set node->type */
  }
  else {
    Reader *r = readers[(unsigned char)c];
    if (r) {
      r->read(node = sexnode(c));
    } else {
      read_sym(node = sexnode(SEX_SYMBOL));
    }
  }

  return node;
}

/**********************************************************************/

SexNode *sexnode(char type) {
  SexNode *n = SEX_CALLOC(1, sizeof(SexNode));
  n->type = type;
  return n;
}

char sexskip(void) {
  while (*pcur && isspace(*pcur)) sexnext();
  return *pcur;
}

char *sexscan(int (*is_term)(char)) {
  if (is_term(sexcur())) return "";
  char buffer[SEX_VALCHARS_MAX + 1];
  int len = 0;
  do {
    buffer[len++] = sexcur();
    if (is_term(sexpeek())) break;
  } while (sexnext() && (len < SEX_VALCHARS_MAX));
  buffer[len] = '\0';
  char *str = SEX_MALLOC((len+1)*sizeof(char));
  return strcpy(str, buffer);
}

SexNode *sexread(const char *str) {
  pcur = str;
  SexNode *head, *tail;
  head = tail = read_cur();
  while (tail && *(++pcur))
    tail = tail->next = read_cur();
  pcur = NULL;
  return head;
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
        Reader *r = readers[(unsigned char)node->type];
        if (r) r->free(node->vusr);
      }
    }
    SexNode *tmp = node;
    node = node->next;
    free(tmp);
  }
}

void sexreader(char sigil, SexReader read, FreeFn free) {
  Reader *r = SEX_CALLOC(1, sizeof(Reader));
  *r = (Reader){.sigil=sigil, .read=read, .free=free};
  readers[(unsigned char)sigil] = r;
}

/**********************************************************************/


