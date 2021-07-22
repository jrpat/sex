# A Simple S-Expression Parser

### Supported datatypes

- List: `(a b c)`
- Symbol: `abcd` `ab-cd`
- String: `"abc"`
- Integer: `123` `-123`
- Decimal: `12.3` `-12.3`
- Custom: *see below*


### Node structure

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

-----

## API

API functions are all lowercase, all one word, prefixed by `sex`.

### Parsing

```c
/*
** Recursively parse an s-expression into a tree of SexNodes
*/
SexNode *sexread(const char *str);

/*
** Recursively free a SexNode's memory
*/
void sexfree(SexNode *node);
```

### Custom Readers

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

---

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
/* Get the current character */
char sexcur(void);

/* Move cur by n and return the new character */
char sexnext(int n);

/* Get the char at cur+n without moving cur */
char sexpeek(int n);
```

When the reader is called, `cur` will be pointing at the
sigil. (ie. `sexcur()` will return the sigil). This allows the same
function to handle multiple datatypes.

The reader should consume only as many characters as it uses. That
is, when the reader is finished, `sexcur()` should return the last
character used by the reader. 

---

The following utility functions may be useful when implementing
a reader:

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

## Limitations

- The maximum number of characters in a number or symbol is 512.
- The node struct uses anonymous unions, so a C11 compiler is required.
  - You could easily change this by giving the union a name.


### TODO:

- [ ] Escaped characters in strings




