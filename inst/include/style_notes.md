# Formatting style notes for C++ code

## Overview

We mostly follow https://google.github.io/styleguide/cppguide.html#Naming

- snake_case(.h) for file names and namespaces
- PascalCase for classes and types, including template types, enums and values within enums
- camelCase for functions and methods
- snake_case for local variables
- s_snake_case for compile-time variables (constexpr and static, but not ordinary const) - but not within enums
  - Note: differs to google as they want kPascalCase for even const local variables
- m_snake_case for member variables within classes
- get_variable and set_variable for simple getter/setter of variable
- EVIL_CASE for macros only
- struct for public POD only, and simple snake_case for all members
- Opening curlies on new line for class/struct/namespace but same line for if
- Use auto with trailing return type on new line for all functions/members, except if return type is void
- Almost aways auto and absolutely always const
