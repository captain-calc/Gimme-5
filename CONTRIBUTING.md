# Programming Style Guide (Focusing on C/C++)

**Version:** 1

Writing code is like writing a book. A well-written book following the
consistent rules of grammar and punctuation is pleasant to read. A poorly
written book confuses its readers and blunts its message's impact.
Well-written, consistently "punctuated" code is easier to read, easier to
debug, easier to validate, and easier to maintain.

## Functional Programming Style Guide (For the C Programming Language)

*Note: Some conventions stated here will also be used in C++ when functional
programming is blended with object-oriented programming.*

### `#include` Order and Placement

`#include` statements always appear first in a source file and immediately
after the include guards in a header file. System `#include` statements come
before local `#include` statements, and each group of statements is organized
alphabetically.

If the `#include` statement has a directory name, or names, in it, it goes
before the `#include` statements that have no directory specifiers. This
group of statements with directory specifiers is also organized alphabetically.

### Header File Example (gui.h)

```
#ifndef GUI_H
#define GUI_H


#include <stdint.h>

#define FONT_HEIGHT_IN_PIXELS (8)


struct point
{
  uint24_t xpos;
  uint8_t ypos;
};


void foo(void);


#endif
```

### Source File Example (gui.c)

```
#include <folder/system_header_aardvark.h>
#include <folder/system_header_boulder.h>
#include <system_header_apple.h>
#include <system_header_bear.h>

#include "folder/local_header_aardvark.h"
#include "folder/local_header_boulder.h"
#include "local_header_apple.h"
#include "local_header_bear.h"


// ============================================================================
// DEFINITIONS AND TYPE DECLARATIONS
// ============================================================================


#define GREEN  (0)
#define ORANGE (1)

typedef uint8_t some_custom_type_t;


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static unsigned int log10(IN unsigned int value);


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


// ... public function definitions ...

void foo(void)
{
  // ... definition ...
  log_of_variable = log10(variable);
  // ... definition ...
}

// ... public function definitions ...


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static unsigned int log10(IN unsigned int value)
{
  // ... definition ...
}
```

## Object-Oriented Programming Style Guide (For the C++ Programming Language)

### Class Declarations

The `public` section is declared before the `private` section because when we
reference the class declaration, we want to know what it can do, not how it is
implemented.

### Example Class Definition

```
class Rectangle
{
  public:
    Rectangle();
    ~Rectangle();

    void set_xpos(IN uint24_t xpos);
    void set_ypos(IN uint8_t ypos);
    void set_width(IN uint24_t width);
    void set_height(IN uint8_t height);
    void center_vertically();
    void center_horizontally();
    void center_both_axes();

    void set_color(IN uint8_t color);

  private:
    uint24_t xpos;
    uint8_t ypos;
    uint24_t width;
    uint8_t height;

    uint8_t color;
};
```
