/*
**
** S-Expression Parser
**
*/

/*
** Define these macros to use a different allocator
*/
#ifndef SEX_MALLOC
#define SEX_MALLOC malloc
#define SEX_FREE free
#endif


/**********************************************************************/
/*
** Parse S-Expressions
*/

typedef enum SexNodeType SexNodeType;
typedef struct SexNode SexNode;

enum SexNodeType {
  SEX_LIST    = '\a',
  SEX_STRING  = '\b',
  SEX_SYMBOL  = '\t',
  SEX_INTEGER = '\n',
  SEX_DECIMAL = '\v',
};

struct SexNode {
  SexNode *next;
  SexNodeType type;
  union {
    SexNode *list;
    long  vint;
    float vdec;
    char *vstr;
    char *vsym;
    void *vusr;
  };
};

/*
** Recursively parse an s-expression
*/
SexNode *sexparse(const char *str);

/*
** Recursively free an s-expression's memory
*/
void sexfree(SexNode *node);



/**********************************************************************/
/*
** Custom Readers
*/

/*
** Current character manipulation
*/
char sexnext(int);  /* returns current char and increments cur by n */
char sexpeek(int);  /* peek char at cur+n without moving cur */
#define sexcur() sexpeek(0)

/*
** A Reader function consumes a number of characters and returns
** a pointer which will be stored in node->vusr
**
** The reader should consume only as many characters as it uses. That
** is, when the reader is finished, sexpeek(0) should return the _last_
** character used by the reader. 
*/
typedef void *(*SexReader)(void);

/*
** Add a reader
**  
**  Readers are distinguished by their first byte (their sigil), which
**  will also end up being their node->type.
**  
**  Reader sigils can be any ascii character, except:
**    - alphanumeric characters
**    - whitespace as defined by isspace()
**    - the characters ( ) " .
**    - the characters used by the SexNodeType enum:
**        \a \b \t \n \v
**
**  The last parameter optional. If provided, it will be called by
**  sexfree() to free the returned pointer. If not provided, free() will
**  be used.
*/
int sexread(char sigil, SexReader, void(*ufree)(void*));

