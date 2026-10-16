; Keywords
"var" @keyword
"const" @keyword
"implements" @keyword
"fn" @keyword
"try" @keyword
"by" @keyword
"orelse" @keyword
"catch" @keyword
"if" @keyword
"else" @keyword
"module" @keyword
"struct" @keyword
"enum" @keyword

; Type names
(bindingType
  (identifier) @type)

; Variable declarations
(bindingDeclaration
  (identifier) @variable)

; Identifiers
(identifier) @variable

; Integer literals
(decimalLiteral) @number
(binaryLiteral) @number
(octalLiteral) @number
(hexLiteral) @number

; Floating-point literals
(decimalFloatLiteral) @number
(hexFloatLiteral) @number
(floatLiteral) @number

; Comments
(comment) @comment

; Operators
[
  "+"
  "-"
  "*"
  "/"
  "%"
  "="
  "=="
  "!="
  "<"
  ">"
  "<="
  ">="
  "&"
  "|"
  "^"
  "!"
] @operator

; Delimiters
[
  "("
  ")"
  "["
  "]"
  "{"
  "}"
] @punctuation.bracket

; Separators
[
  ";"
  ","
  "."
  ":"
] @punctuation.delimiter
