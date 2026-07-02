# Conventions

This file documents the coding conventions used within this project.

## C++ (.cc, .h)

- Indentation is four spaces and opening braces are placed on the next line for functions and class methods.
- Braces on a separate line for classes, structs, enums, and other structural elements.
- Braces on the same line for control flow and namespaces.
- Anonymous namespaces can be used to contain functions and variables to the current translation unit.
- Closing braces for namespaces should include a comment with the name of the namespace.
- Use `#pragma once` for header guards, with standard library includes listed before local headers and separated by a blank line.
- Class names use PascalCase (e.g., `Sonic2RingLayout`, `KosinskiWriter`, `Rom`), and functions use camelCase (e.g., `readWord`, `getColorCount`).
- Local variables, parameters, and struct fields use camelCase (e.g., `blocksAddr`, `encodedY`).
- Member variables are prefixed with `m_` (e.g., `m_bar`, `m_byteCount`).
- Enum values use PascalCase (e.g., `First`, `Second`).
- Other constants and constexpr expressions can use ALL_CAPS.
- Use `std::shared_ptr` for shared ownership, with constructor injection for dependencies.
- Prefer `const` correctness for read-only methods and pass-by-value for small types while using references for larger objects or smart pointers.
- Comments may delineate sections and provide inline notes.
- Multi-line doc comments are permitted for classes and inline explanation comments for logic blocks.
- Use `auto` for local variables when the type is clear from context, especially with calculated addresses or values.
- Header include ordering. Place stdlib before local headers, blank-line separated.

```cpp
#pragma once

#include <memory>

#include "something.h"

class Bar;

namespace {

int helper()
{
    return 1;
}

}  // namespace

namespace example {

/**
* Example class using repository conventions
*/
class Foo
{
public:
    enum Status
    {
        First  = 1 << 0,
        Second = 1 << 1
    };

    // single line comment
    explicit Foo(const std::shared_ptr<Bar> &bar);

    int32_t countThings() const
    {
        auto total = Start;
        if (enabled) {
            for (int32_t i = 0; i < 4; i++) {
                // loop body
                total += i;
            }
        } else {
          // not enabled
        }

        return total + helper();
    }

private:
    std::shared_ptr<Bar> m_bar;

    static constexpr int32_t SOME_VALUE = 3;
};

}  // namespace example
```

### Spacing

A note on spacing.

This code originally used 2-space indentation, but new files should use 4-spaces.

Existing code will be migrated when appropriate.
