; Keywords
[
    "var"
    "const"
    "implements"
    "fn"
    "try"
    "by"
    "orelse"
    "catch"
] @keyword

; Type names
(type) @type
(bindingType) @type

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
