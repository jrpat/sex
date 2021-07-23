#if SEX_ENABLE_PRINT

#include <stdio.h>
#include <unistd.h>

#ifndef SEX_PRINT_DEPTH_MAX
#define SEX_PRINT_DEPTH_MAX 32
#endif

#ifndef SEX_COLOR_STEM
#define SEX_COLOR_STEM "30;1"
#endif

static int depth;
static SexNode *chain[SEX_PRINT_DEPTH_MAX];
static FILE *out;
static tty;

#define CLR_STEM() if(tty) { fprintf(out, "\033[" SEX_COLOR_STEM "m"); }
#define CLR_OFF() if(tty) { fprintf(out, "\033[m"); }

#define PRN(...) fprintf(out, __VA_ARGS__)
#define PRNSTEM(...) CLR_STEM(); PRN(__VA_ARGS__); CLR_OFF();

static void prtree(SexNode*, int);

static void prindent(void) {
  CLR_STEM();
  for (int i=1; i < depth; i++) {
    /* If the deepest item is a list, it's about to be drawn */
    if (chain[depth]->type == SEX_LIST) {
      if (chain[depth]->next) goto drawvline;
      if (chain[depth]->list && chain[depth]->list->next)
        goto drawvline;
    } else {
      for (int j=i; j <= depth; j++) {
        if (chain[j]->next) goto drawvline;
      }
    }
    PRN("└─");
    continue;
drawvline:
    PRN("│ ");
  }
  CLR_OFF();
}

static void prbullet(SexNode *n) {
  CLR_STEM();
  if (n->type == SEX_LIST) {
    if (n->next || (n->list && n->list->next))
      PRN("├ ");
    else
      PRN("└ ");
  } else {
    PRN(n->next ? "├ " : "└ ");
  }
  CLR_OFF();
}

static void prnode(SexNode *n) {
  switch (n->type) {
    case SEX_LIST:
      if (!n->list) {
        PRNSTEM("⊏ ∅\n");
        depth++;
        prtree(n->list, 1);
      } else if (n->list->type == SEX_LIST) {
        PRNSTEM("┌─");
        depth++;
        prtree(n->list, 0);
      } else {
        PRNSTEM(n->list->next ? "┌ " : "⊏ ");
        prnode(n->list);
        PRN("\n");
        depth++;
        prtree(n->list->next, 1);
      }
      break;
    case SEX_STRING:
      PRN("\"%s\"", n->vstr);
      break;
    case SEX_SYMBOL:
      PRN("%s", n->vsym);
      break;
    case SEX_INTEGER:
      PRN("%ld", n->vint);
      break;
    case SEX_DECIMAL:
      PRN("%f", n->vdec);
      break;
    default:
      PRN("<type: %c>", n->type);
      break;
  }
}

static void prtree(SexNode *n, int indent) {
  if (!n) { chain[depth--] = NULL; return; }

  chain[depth] = n;

  if (indent) {
    prindent();
    prbullet(n);
  }

  prnode(n);

  if (n->type != SEX_LIST)
    PRN("\n");

  prtree(n->next, 1);
}

void sexprint(FILE *out_, SexNode *n) {
  out = out_;
  tty = isatty(fileno(out));
  depth = 0;

  prtree(n, 0);

  out = NULL;
  tty = 0;
  for (int i=0; i < 32; i++) chain[i] = NULL;
}

#endif /* SEX_ENABLE_PRINT */

