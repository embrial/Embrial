/**
 * @file Embrial grammar for tree-sitter
 * @author Adrian Long
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

export default grammar({
  name: "embrial",

  extras: $ => [
    /\s/, // whitespace
    $.comment,
  ],

  supertype: $ => [
    $.expression,
  ],

  word: $ => $.identifier,

  rules: {
    source_file: $ => optional($.statements),
    
    comment: $ => token(seq('#', /.*/)),

    statements: $ => repeat1(choice(
      $.itemDeclaration,
      $.statement
    )),
    statement: $ => seq(
      choice(
        $.bindingDeclaration,
        $.expression,
      ),
      ';'
    ),
    
    itemDeclaration: $ => choice(
      $.functionDeclaration,
    ),


    implementsExpression: $ => seq(
      $.identifier,
      optional($.genericsDeclaration),
      optional(seq(
        "implements",
        $.identifier,
        repeat(seq(
           '+',
           $.identifier,
        ))
      ))
    ),

    genericsDeclaration: $ => seq(
      '<',
      $.implementsExpression,
      repeat(seq(',', $.implementsExpression)),
      '>'
    ),

    functionSignature: $ => prec.right(seq(
      '(',
      optional(seq(
        $.identifier,
        $.type,
        repeat(seq(
          ',',
          $.identifier,
          $.type
        ))
      )),
      ')',
      optional($.type),
    )),
    functionDeclaration: $ => seq(
      "fn",
      $.identifier,
      optional($.genericsDeclaration),
      $.functionSignature,
      choice(
        seq('{', $.statements, '}'),
        seq(':', $.expression, ';')
      )  
    ),

    bindingDeclaration: $ => seq(
      choice("var", "const"),
      $.identifier,
      choice(
        seq($.type, optional(seq('=', $.expression))),
        seq('=', $.expression)
      )
    ),

    prefixTypeModifiers: $ => repeat1(choice (
      '?',
      '@',
      '*',
      '~',
      seq('[', optional($.intLiteral), ']'),
      "[@]",
      "[*]",
      "[~]",
    )),

    genericsArguments: $ => seq(
      '<',
      $.identifier,
      optional($.genericsArguments),
      repeat(seq(
        ',',
        $.identifier,
        optional($.genericsArguments),
      )),
      '>'
    ),

    genericsTypeModifier: $ => $.genericsArguments,

    errorTypeModifier: $ => optional(seq(
      '!',
      choice(
        $.identifier,
        seq(
          '{',
          $.identifier,
          repeat(seq(
            ',',
            $.identifier
          )),
          '}'
        ),
      )
    )),

    lambdaType: $ => seq(
      "fn",
      optional($.genericsTypeModifier),
      $.functionSignature,
    ),

    lambda: $ => seq(
      "fn",
      optional($.genericsDeclaration),
      $.functionSignature,
      choice(
        seq('{', $.statements, '}'),
        seq(':', $.expression)
      )
    ),

    bindingType: $ => prec.right(seq(
      optional($.prefixTypeModifiers),
      $.identifier,
      optional($.genericsTypeModifier)
    )),

    type: $ => choice(
      $.bindingType,
      $.lambdaType
    ),

    expression: $ => choice(
      $.assignment_expression,
      $.logical_or_expression,
      $.logical_and_expression,
      $.bitwise_or_expression,
      $.bitwise_xor_expression,
      $.bitwise_and_expression,
      $.equality_expression,
      $.relational_expression,
      $.binary_expression,
      $.unary_expression,
      $.postfix_expression,
      $.primary_expression,
    ),

    basicAssignmentOperator: $ => choice(
      '=',
      "&=",
      "|=",
      "^=",
      "+=",
      "-=",
      "*=",
      "/=",
      "<<=",
      ">>="
    ),
    wrapingAssignmentOperator: $ => choice(
      "+%=",
      "-%=",
      "*%=",
    ),
    saturatingAssignmentOperator: $ => choice(
      "+|=",
      "-|=",
      "*|=",
      "<<|=",
      ">>|="
    ),

    assignmentOperator: $ => choice(
      $.basicAssignmentOperator,
      $.wrapingAssignmentOperator,
      $.saturatingAssignmentOperator
    ),

    assignment_expression: $ => prec(1, seq(
      $.identifier,
      $.assignmentOperator,
      $.expression
    )),

    logical_or_expression: $ => prec.left(2, seq(
      $.expression, "||", $.expression,
    )),

    logical_and_expression: $ => prec.left(2, seq(
      $.expression, "&&", $.expression
    )),

    bitwise_or_expression: $ => prec.left(3, seq(
      $.expression, '|', $.expression
    )),

    bitwise_xor_expression: $ => prec.left(4, seq(
      $.expression, '^', $.expression
    )),
    
    bitwise_and_expression: $ => prec.left(5, seq(
      $.expression, '&', $.expression
    )),

    eqOperator: $ => choice(
      "==",
      "!="
    ),

    equality_expression: $ => prec.left(6, seq(
      $.expression, $.eqOperator, $.expression
    )),

    comparisonOperator: $ => choice(
      '<',
      "<=",
      '>',
      ">="
    ),

    relational_expression: $ => prec.left(7, seq(
      $.expression, $.comparisonOperator, $.expression
    )),

    shiftOperator: $ => choice(
      "<<",
      "<<|",
      ">>",
      ">>|",
    ),
    addOrSubOperator: $ => choice(
      "+",
      "+%",
      "+|",
      "-",
      "-%",
      "-|"
    ),
    
    mulOrDivOperator: $ => choice(
      "*",
      "*%",
      "*|",
      "/",
      "//",
      "%",
    ),

    binary_expression: $ => choice(
      prec.left(8, seq($.expression, $.shiftOperator, $.expression)),
      prec.left(9, seq($.expression, $.addOrSubOperator, $.expression)),
      prec.left(10, seq($.expression, $.mulOrDivOperator, $.expression)),
    ),

    unary_expression: $ => prec.right(11, seq(
      choice("try", '&', '$', '~', '!'),
      $.expression
    )),

    expressionBlock: $ => seq(
      '(',
      $.expression,
      repeat(seq(';', $.expression)),
      ')'
    ),

    intLiteralOrExpressionBlock: $ => choice(
      $.intLiteral,
      $.expressionBlock
    ),

    rangeExpression: $ => seq(
      $.intLiteralOrExpressionBlock,
      optional(seq(
        "..",
        optional('='),
        $.intLiteralOrExpressionBlock,
        optional(seq(
          "by",
          $.intLiteral
        ))
      ))
    ),

    chainExpression: $ => seq(
      '.',
      choice(
        $.identifier,
        '?',
        '!',
        '*'
      ),
    ),

    postfix_expression: $ => prec.left(12, seq(
      $.expression,
      choice(
        seq("orelse", $.expression),
        seq("catch", $.expression),
        seq('[', $.rangeExpression, ']'),
        seq('(', $.expression, repeat(seq(',', $.expression)), ')'),
        $.chainExpression
      )
    )),

    arrayLiteral: $ => seq(
      '[',
      optional(seq(
        $.expression,
        repeat(seq(',',$.expression))
      )),
      ']'
    ),

    primary_expression: $ => choice(
      $.identifier,
      $.floatLiteral,
      $.rangeExpression,
      $.arrayLiteral,
      $.lambda,
      $.ifExpression,
    ),

    ifExpression: $ => prec.right(seq(
        'if',
        $.expression,
        $.block,
        optional(seq(
            'else',
            $.block
        ))
    )),

    block: $ => choice(
        seq(':', $.statement),
        seq('{', $.statements, '}')
    ),


    intLiteral: $ => choice(
      $.decimalLiteral,
      $.binaryLiteral,
      $.octalLiteral,
      $.hexLiteral
    ),

    decimalLiteral: $ => choice(
      '0',
      seq(/[1-9]/, repeat(seq(optional('_'), /[0-9]/)))
    ),
    binaryLiteral: $ => seq(
      '0',
      choice('b', 'B'),
      $.binaryDigits
    ),
    octalLiteral: $ => seq(
      '0',
      choice('b', 'B'),
      $.octalDigits
    ),
    hexLiteral: $ => seq(
      '0',
      choice('x', 'X'),
      $.hexDigits
    ),

    decimalDigits: $ => seq(
      /[0-9]/,
      repeat(seq(
        optional('_'),
        /[0-9]/
      ))
    ),
    binaryDigits: $ => seq(
      /[01]/,
      repeat(seq(
        optional('_'),
        /[01]/
      ))
    ),
    octalDigits: $ => seq(
      /[0-7]/,
      repeat(seq(
        optional('_'),
        /[0-7]/
      ))
    ),
    hexDigits: $ => seq(
      /[0-9a-fA-F]/,
      repeat(seq(
        optional('_'),
        /[0-9a-fA-F]/
      ))
    ),

    floatLiteral: $ => choice(
      $.decimalFloatLiteral,
      $.hexFloatLiteral
    ),
    decimalFloatLiteral: $ => prec.right(seq(
      $.decimalDigits,
      optional(seq(
        '.',
        $.decimalDigits
      )),
      optional($.decimalExponent)
    )),
    decimalExponent: $ => seq(
      choice('e', 'E'),
      optional(choice('+', '-')),
      $.decimalDigits
    ),
    hexFloatLiteral: $ => seq(
      $.hexLiteral,
      choice('p', 'P'),
      optional(choice('+','-')),
      $.decimalDigits
    ),

    identifier: $ => /[a-zA-Z_][a-zA-Z0-9_]*/,
  }
});
