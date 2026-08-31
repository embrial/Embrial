/*
Header comment:
    File:       ast.h
    Project:    TBD
    Created:    2026-08-19

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/


typedef struct {
    SourceLocation location;

	GrammarSymbol kind;
    union {
        String literal;             // STRING_LITERAL, NUMERIC_LITERAL
        Symbol *symbol;             // DECLARATION, TYPE
        TokenType op;               // EXPRESSION
        TypeRestraintList *types;   // BLOCK
    };

	size_t childIndex;
	size_t childCount;
} AstNode;
