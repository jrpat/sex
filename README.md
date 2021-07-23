# A Simple S-Expression Parser

### Supported datatypes

- List: `(a b c)`
- Symbol: `abcd` `ab-cd`
- String: `"abc"`
- Integer: `123` `-123`
- Decimal: `12.3` `-12.3`
- Custom: *see below*


## Parsing

Recursively parse an s-expression into a tree of `SexNode`s:

```c
SexNode *sexread(const char *str);
```

Recursively free a `SexNode`'s memory:

```c
void sexfree(SexNode *node);
```


## Node Structure

The parser generates a tree of nodes, which have the following
structure:

```c
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
```


## Custom Readers

You can add a custom datatype by adding a reader which parses its
literal representation. Custom literals begin with a single character (a
"sigil"), which tells the parser to invoke the reader for that literal.

To register a new reader, call:
```c
void sexreader(char sigil, SexReader, void(*ufree)(void*));
```

The first parameter is the first character of the new literal. Sigils
can be any ascii character, except:
  - alphanumeric characters
  - whitespace as defined by `isspace()`
  - the characters `(` `)` `"` `-`
  - the characters used by `SexNodeType`:
      `\a` `\b` `\t` `\n` `\v`

The second parameter is the reader function, described below.

The last parameter will be called by `sexfree()` to free the returned
pointer.

### Reader Function

To implement a reader, create a function of type:

```c
void my_reader(SexNode* node);
```

A Reader function consumes a number of characters, converts them into
a datatype, and stores a pointer to the data in `node->vusr`.

The parser stores a pointer to the current character being processed,
which we'll call `cur`. `cur` is not accessible directly, but you can
use the following functions to read characters and move `cur`:

```c
/* Get the char at cur+n without moving cur */
char sexlook(int n);

/* Move cur by n and return the new character */
char sexmove(int n);
```

The following forms are provided to make the common cases more
ergonomic:

```c
char sexcur(void);  /* sexlook(0) */
char sexpeek(void); /* sexlook(1) */
char sexnext(void); /* sexmove(1) */
char sexprev(void); /* sexmove(-1) */
```

When the reader is called, `cur` will be pointing at the
sigil. (ie. `sexcur()` will return the sigil). This allows the same
function to handle multiple datatypes.

The reader should consume only as many characters as it uses. That
is, when the reader is finished, `sexcur()` should return the last
character used by the reader. 

### Reader Helpers

The following functions may be useful when implementing a reader:

```c
/*
** Get a string of the value pointed to by cur. That is, every character
** before the next whitespace, (, or )
*/
char *sexscan(void);

/*
** Returns 1 if the character is null, whitespace, (, or ).
*/
int sexterm(char c);

/*
** Allocate and return a new SexNode of type `type`
** Memory will be initialized to zero.
*/
SexNode *sexnode(char type);

```


## Printing

`sexprint.c` can produce pretty-printed output to a tty or file:

```c
void sexprint(FILE *out, SexNode *node);
```

To disable printing:

```c
#define SEX_ENABLE_PRINT 0
```

If printing is disabled, Sex will not `#include <stdio.h>`. You can
then exclude `sexprint.c` from your build, but it won't hurt to leave it
in (the whole file is wrapped in `#if SEX_ENABLE_PRINT` anyway).


## Configuration

You can use your own allocator by defining the following macros. If you
define one, you must define them all.

```c
#define SEX_MALLOC  my_malloc
#define SEX_CALLOC  my_calloc
#define SEX_REALLOC my_realloc
#define SEX_FREE    my_free
```

To change the maximum number of characters in a symbol or number, define
the following macro:

```c
#define SEX_VALCHARS_MAX 1024  /* default is 512 */
```

By default `sexprint()` prints the stems of the tree in grey if printing
to a tty. To change the color, define the following macro:

```c
#define SEX_COLOR_STEM "31"    /* ansi red */
/* or */
#define SEX_COLOR_STEM "31;1"  /* ansi bright red */
/* or */
#define SEX_COLOR_STEM "0"     /* ansi disable */
```


## Limitations

- The maximum number of characters in a number or symbol is 512 by
  default.
- The node struct uses anonymous unions for interface simplicity, so
  a C11 compiler is required. *(You could easily change this by
  giving the union a name.)*
- Escape characters in strings are not parsed.


### TODO:

- [ ] Parse escape characters in strings


