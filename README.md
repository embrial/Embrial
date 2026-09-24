# Embrial
Embrial is a statically type systems programming language designed around explicit memory management, memory safety, predictable behavior, and simple, composable privatives.

Embrial is still in early development

# Building
## Requirements
- Zig
- A C compiler/toolchain

## Build
```bash
zig build
```
# Specification
The Embrial specification is a set of files located in `specification/`. The specification formatter can be run as part of the Zig build system. Running:
```bash
zig build spec
```
reads the `specifiation/format.txt` file and processes directives in the form of `#directive argument`, to produce the `manual.txt`. The specification source files should be edited rather than editing the generated `manual.txt` directly.

## Specification Directives
The specification formatter uses a small set of directive to control how the manual is generated from the source files in the `specification/` directory. Directives begin with `#` and are interpreted by `specFormater.zig`.
### `#block`
Begins a block of specification content.
```
#block specifiation/filename.txt
```
The formmater reads the referenced files and includes its content in the generated specification. The formatter will,
- add the name of block at the top
- add line number on the left hand side in the form `  n |   `. Appropriately padding the number with spacing such that the `|` character is lined up from top to bottom.
- ensure that the included source file wraps by word should the length of the line in the manual.txt exceed 80 characters.

When processing a block should the formatter encounter the `#comment` directive, it will resume copying lines from the `format.txt` until the next `#block` directive is encountered. The `#block` directive encountered will resume writing from the previously opened source file so long as it does not have a filename following it.

### `#include`
The include keyword will paste exactly the file reference by its argument
```
#include filename.txt
```

### `#keyword`
Declares a keyword for the specification formatter.
```
#keyword var
#keyword const
#keyword fn
```

The specification formatter will search the `format.txt` to extract all keyword declaration in order to generate a keyword table for the `#keywords` directive to paste.

### `#keywords`
Paste the generated keyword table in the `manual.txt`
```
#keywords
```
