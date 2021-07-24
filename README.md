# A Simple S-Expression Parser

**Datatypes:**
- List: `(a b c)`
- Symbol: `abcd` `ab-cd` `#abcd` `/abcd/` ...
- String: `"abc"`
- Integer: `123` `-123`
- Decimal: `12.3` `-12.3`
- Custom: *see below*

**Dependencies:**
- A C11 compiler *(see [Limitations](#Limitations) section for
  a workaround)*
- POSIX *(unless you disable pretty-printing. See [Printing](#Printing).)*


## Parsing

**Recursively parse a string into a tree of `SexNode`s:**

```c
SexNode *sexread(const char *str);
```

Multiple top-level s-expressions are allowed. The first is returned, and
the rest are available via `->next`.

**Recursively free a `SexNode`'s memory:**

```c
void sexfree(SexNode *node);
```

This will also free any siblings of `node`.


## Node Structure

The parser generates a tree of nodes, which have the following
structure:

```c
struct SexNode {
  SexNode *next;
  union {
    SexNode *list;  /* type == SEX_LIST */
    char    *vsym;  /* type == SEX_SYMBOL */
    char    *vstr;  /* type == SEX_STRING */
    long     vint;  /* type == SEX_INTEGER */
    float    vdec;  /* type == SEX_DECIMAL */
    void    *vusr;  /* custom type */
  };
  char type;
};
```
To get a node's value, check its type and get the appropriate `vxxx`
field.

If `node->type == SEX_LIST`, `node->list` is the first child (`NULL` for
empty lists).



## Custom Readers

You can add a custom datatype by adding a reader which parses its
literal representation. Custom literals begin with a single character (a
"sigil"), which tells the parser to invoke the reader for that literal.

**Register a new reader:**
```c
void sexreader(char sigil, SexReader read, void(*ufree)(void*));
```

- The first parameter is the sigil. It can be any ascii character,
  except:
  - alphanumeric characters
  - whitespace (as defined by `isspace()`)
  - the characters `(` `)` `"` `-`
  - the characters used by `SexNodeType`:
      `\a` `\b` `\t` `\n` `\v`

- The second parameter is the reader function, described below.

- The last parameter will be called by `sexfree()` and passed the `vusr`
  pointer.

### Reader Function

To implement a reader, create a function like:

```c
void my_reader(SexNode* node) {
  MyType *t = malloc(sizeof(MyType));
  /* ... read characters and parse into t ... */
  node->vusr = t;
}
```

#### Implementation

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

When the reader is called, `cur` will be pointing at the sigil. This
allows the same function to handle multiple datatypes.

The reader should consume only as many characters as it uses. That
is, when the reader is finished, `cur` should be pointing at the last
character of the literal.

#### Helpers

The following functions may be useful when implementing a reader:

```c
/* Reads at most SEX_VALCHARS_MAX characters until is_term(sexpeek())
** returns 1, then returns a copy of those characters.
** When finished, cur is pointing at the last read character. */
char *sexscan(int (*is_term)(char));

/* Returns 1 if the character is null, whitespace, '(', or ')'.  */
int sexterm(char c);

/* Moves cur to the next non-whitespace character and returns it.  */
char sexskip(void);

/* Allocates and returns a new SexNode of type `type`.
** Memory is initialized to zero.
** Useful when reading nested literals. */
SexNode *sexnode(char type);

```


## Printing

`sexprint.c` can produce pretty-printed output to a tty or file:

```c
void sexprint(FILE *out, SexNode *node);
```

Example:

```c
sexprint(stdout, sexread("(a (b c ((d e) f g)) () h)"));

┌ a
├ ┌ b
│ ├ c
│ ├ ┌─┌ d
│ │ │ └ e
│ │ ├ f
│ └─└ g
├ ⊏ ∅
└ h

```


`sexprint.c` requires `stdio.h` and POSIX compliance (for
`fileno()`).

To disable pretty-printing:

```c
#define SEX_ENABLE_PRINT 0
```

If disabled:
- `sex.h` will not `#include <stdio.h>` and will not declare
  `sexprint()` or `sex_color_stem`.
- `sex.c` is a no-op, and thus POSIX is not required.

You can then exclude `sexprint.c` from your build (though it won't hurt
to leave it in, as the whole file is wrapped in `#if SEX_ENABLE_PRINT`
anyway).


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
to a tty. To change the color, set the `sex_color_stem` variable before
calling `sexprint()`:

```c
sex_color_stem = "\033[31m";      /* red */
/* or */
sex_color_stem = "\033[31;1m";    /* bright red */
/* or */
sex_color_stem = "\033[43m;31m";  /* yellow bg, red fg */
/* or */
sex_color_stem = "\033[0m";       /* default */
```


## Limitations

- The maximum number of characters in a number or symbol is 512 by
  default.
- The node struct uses anonymous unions for interface simplicity, so
  a C11 compiler is required.
  - *(You could easily change this by giving the union a name.)*
- Dotted pairs are not parsed as in many lisps. A dot in a list is
  interpreted as the symbol `.`
- Escape characters in strings are not parsed.


## Testing

Right now testing is manual: just a file that parses and prints
a somewhat pathological string. To run the test:

```
make -C test
```


### TODO:

- [ ] Better tests
- [ ] Parse escape characters in strings

