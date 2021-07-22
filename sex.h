/*
**
** S-Expression Parser
**
*/

#ifndef SEX__H
#define SEX__H

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
  SEX_SYMBOL  = '\b',
  SEX_STRING  = '\t',
  SEX_INTEGER = '\n',
  SEX_DECIMAL = '\v',
};

struct SexNode {
  SexNode *next;
  union {
    SexNode *list;  /* type SEX_LIST */
    char    *vsym;  /* type SEX_SYMBOL */
    char    *vstr;  /* type SEX_STRING */
    long     vint;  /* type SEX_INTEGER */
    float    vdec;  /* type SEX_DECIMAL */
    void    *vusr;  /* custom type */
  };
  char type;
};

/*
** Recursively parse an s-expression into a tree of SexNodes
*/
SexNode *sexread(const char *str);

/*
** Recursively free a SexNode's memory
*/
void sexfree(SexNode *node);



/**********************************************************************/
/*
** Custom Readers
*/

/*
** A reader function consumes some characters, interprets them into
** a datatype, and stores a pointer to the data in node->vusr.
**
** When the reader is called, cur is pointing at the sigil. This allows
** the same function to read multiple datatypes.
**
** The reader should consume only as many characters as it uses. That
** is, when the reader is finished, sexcur() should return the _last_
** character used by the reader. 
*/
typedef void (*SexReader)(SexNode *node);

/*
** Add a reader
**  
**  Readers are distinguished by their first byte (their sigil), which
**  will also end up being their node->type.
**  
**  Reader sigils can be any ascii character, except:
**    - alphanumeric characters
**    - whitespace as defined by isspace()
**    - the characters ( ) " -
**    - the characters used by the SexNodeType enum:
**        \a \b \t \n \v
**
**  The last parameter will be called by sexfree() to free the returned
**  pointer.
*/
void sexreader(char sigil, SexReader, void(*ufree)(void*));



/**********************************************************************/
/*
** Reader Utilities
*/

/*
** Allocate and return a new SexNode of type `type`
** Memory will be initialized to zero.
*/
SexNode *sexnode(char type);

/*
** Current character manipulation
*/
char sexnext(int n);  /* move cur by n and return the new character */
char sexpeek(int n);  /* peek char at cur+n without moving cur */
#define sexcur() sexpeek(0)

/*
** Get a string of the value pointed to by cur. That is, every character
** before the next whitespace, (, or )
*/
char *sexscan(int (*is_term)(char));

/*
** Returns 1 if the character is null, whitespace, (, or ).
*/
int sexterm(char c);


#endif /* SEX__H */

