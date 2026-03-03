# PDF Coding Style Guide

## File Structure

Every `.c`/`.h` file uses sections separated by banners. Keep all banners even if a section is empty.

### Header Guards

Header files use guards `PATH_FILENAME_H_`, matching the repo-relative path. These  `#ifndef`/`#define` should be placed above the Includes banner.

\`\`\`c
#ifndef SOURCE_API_UART_API_H_
#define SOURCE_API_UART_API_H_
// ...
#endif /* SOURCE_API_UART_API_H_ */
\`\`\`

## Macros and definitions

Macros use `ALL_UPPERCASE` format, with underscore in between.

### #if defined

To avoid plausible preprocessor bugs, only `#if defined` is allowed. All `#endif` should be followed by a `/* SOME_MACRO */` comment.

## Includes

`.c` source files include the module's own header **first**, before the feature guard. Inside the guard, project headers precede system headers (`<stdint.h>` etc.). Project headers are sorted by layer (app → api → driver).

`.h` header files first include `framework_config.h` (if necessary), followed by a `#if defined(SOME_MACRO)` feature guard (if necessary).

## Naming

All identifiers use `snake_case` as the base. Prefixes and suffixes are **mandatory**. No abstract variable, definition names, use as explicit variable name as possible.

| Category | Prefix | Suffix | Case | Example |
|---|---|---|---|---|
| Local variable | — | — | `snake_case` | `received_byte` |
| Global variable (`extern` or `static`) | `g_` | — | `snake_case` | `g_is_initialized` |
| Enum typedef | `e` on tag | `_t` | `snake_case` tag | `eUart_t` |
| Struct typedef | `s` on tag | `_t` | `snake_case` tag | `sUartDesc_t` |
| Plain type / callback typedef | — | `_t` | `snake_case` | `led_driver_callback_t` |
| Opaque handle typedef | — | `_Handle` | `PascalCase` | `RingBuffer_Handle` |
| Function | `Module_Submodule_` | — | `PascalCase` verb | `UART_Driver_Init` |
| Constant `#define` / macro | — | — | `SCREAMING_SNAKE_CASE` | `UART_BUFFER_CAPACITY` |

Initialise variables at declaration: `= {0}`, `= NULL`, `= false`. Try to avoid magic numbers.

### Enums

Use prefix `e` when naming enum, followed by `_t` in enum typedef. Always include `_First = 0`, a default value aliased to `_First`, and `_Last` as the last enum value. Use `_First`/`_Last` for iteration and bounds checks.

\`\`\`c
typedef enum eUart {
    eUart_First = 0,
    eUart_Debug = eUart_First,
    eUart_uRos,
    eUart_Last
} eUart_t;
\`\`\`

## Formatting

| Rule | Correct | Wrong |
|---|---|---|
| Space after keyword | `if (x)` `while (x)` | `if(x)` `while(x)` |
| Space before `(` in functions | `bool Func (void)` | `bool Func(void)` |
| Space before `{` | `if (x) {` | `if (x){` |
| `*` in declarations | `char *ptr` | `char* ptr` |
| `*` in casts | `(uint8_t*) ptr` | `(uint8_t *) ptr` |

### Comparisons 

To avoid possible bugs (`=` instead of `==`), use Yoda style in comparisons. Constants, literals, and `NULL` go on the **left**; the variable goes on the **right**:

\`\`\`c
if (NULL != ptr)           // correct
if (eState_Init == state)  // correct
if (ptr == NULL)           // wrong
\`\`\`

### Braces & Switch

`case` bodies are wrapped in `{}`. `break` goes after the closing brace. Always include `default`. Mark intentional fall-through with `/* fall through */`.

\`\`\`c
switch (state) {
    case eState_A: {
        /* fall through */
    } 
    case eState_B: {
        /* ... */
    } break; 
    default: {
    } break;
}
\`\`\`

### Functions

- If a function doesn't take arguments, use `void`: `bool Func (void)`
- `void` functions end with `return;`
- `const` every parameter that is not modified
- Validate all parameters at the top; return `false`/`NULL` immediately on failure

## Debug Trace

Every `.c` file that may have trace output declares a module name string in **Private constants**:

```c
#if defined(DEBUG_UART_API)
CREATE_MODULE_NAME (UART_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_UART_API */
```
