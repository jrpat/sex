#include <stdio.h>
#include <stdlib.h>

#include "../sex.h"


const char *lisp =
  "(root                                                   \n"
  " a                                                      \n"
  " ()                                                     \n"
  " b #                                                    \n"
  " (abc #123-456-7890)                                    \n"
  " (((1 2 3) a (4 5 6) c) d)                              \n"
  " (dog -1 -1.25 -abd .abc .123)                          \n"
  " (panel \"1234567890123456789012345678901234567890\"    \n"
  "  (box a B c D e)                                       \n"
  "  (menu \"asdf\"                                        \n"
  "   (choices                                             \n"
  "    (choice Play \"asdf\")                              \n"
  "    (choice Quit))                                      \n"
  "   (config)                                             \n"
  "   config                                               \n"
  "   ((   ) ( \t\v ) ( \v\t ) x)                          \n"
  "   (<a> [b] {c} 'd')))                                  \n"
  " (♠ ♥ ♦ ♣)                                              \n"
  " z                                                      \n"
  " (a . b . c))                                           \n"
  "                                                        \n"
  "foobar                                                  \n"
  "                                                        \n"
  "(root2 (a b c d))                                       \n"
;


typedef struct PhoneNum {
  char areacode[4];
  char seg1[4], seg2[5];
} PhoneNum;


void read_phonenum(SexNode *node) {
  int i=0, seg=0;
  PhoneNum *p = calloc(1, sizeof(PhoneNum));
  char c;
  while (!sexterm(sexpeek())) {
    c = sexnext();
    if (c == '-') { seg++; i=0; continue; }
    switch (seg) {
      case 0: p->areacode[i++] = c; break;
      case 1: p->seg1[i++] = c; break;
      case 2: p->seg2[i++] = c; break;
    }
  }
  node->vusr = p;
}


int main(int argc, const char *argv[]) {
  (void)argc; (void)argv;

  sexreader('#', read_phonenum, free);
  SexNode *node = sexread(lisp);

  printf("\n------------------------------------------------------------\n");
  printf("%s\n", lisp);
  printf("------------------------------------------------------------\n\n");

  sexprint(stdout, node);

  sexfree(node);
}


/* vim: set tw=72 sts=2 ts=2 sw=2 et : */

