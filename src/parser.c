#ifndef LANG_PARSER_H
    #error "parser.c missing dependency LANG_PARSER_H"
#elif !defined(LANG_HEADERS)
    #error "parser.c missing dependency LANG_HEADERS"
#elif !defined(LANG_AST)
    #error "parser.c missing dependency LANG_AST"
#elif !defined(LANG_SYMBOL)
    #error "parser.c missing dependency LANG_SYMBOL"
#elif !defined(LANG_PARSER)
#define LANG_PARSER

void Parser_init(Parser *parser, Arena *arenas) {
    if (!parser || parser->status != PARSER_STATUS_UNINITIALIZED) return;

    parser->symbols             = arenas;
    parser->symbolPointers      = arenas + 1;
    parser->literals            = arenas + 2;
    parser->ast                 = arenas + 3;
    parser->parseStack          = arenas + 4;
    parser->symbolTableStack    = areans + 5;

    Arena_init(Symbol,      parser->symbols,            0);
    if (parser->symbol.status != ARENA_STATUS_OK) return;

    Arena_init(String,      parser->literals,           0);
    if (parser->literals.status != ARENA_STATUS_OK)
        goto Parser_init_freeSymbols;

    Arena_init(AstNode,     parser->ast,                0);
    if (parser->ast.status != ARENA_STATUS_OK)
        goto Parser_init_freeSymbolPointers;

    Arena_init(NonTerminal, parser->parseStack,         0);
    if (parser->parseStack.status != ARENA_STATUS_OK)
        goto Parser_init_freeLiterals;

    Arena_init(SymbolTable, parser->symbolTableStack,   0);
    if (parser->symbolTableStack.status != ARENA_STATUS_OK)
        goto Parser_init_freeAst;
    
    return;
Parser_init_freeAst:
    Arena_free(parser->ast);
Parser_init_freeLiterals:
    Arena_free(parser->literals);
Parser_init_freeSymbolPointers://
    Arena_free(parser->symbolPointers);
Parser_init_freeSymbols://
    Arena_free(parser->symbols);
}

void Parser_deinit(Parser *parser) {
    Arena **areans = &parser->symbols;
    for (int i = 0; i < 6; i++) {
        Arena_free(arenas[i]);
    }
}


#endif
