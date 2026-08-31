#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H
/*
Header comment:
    File:       constraints.h
    Project:    lang
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/


typedef enum {
    CONTRAINT_FUNCTION,
    CONTRAINT_MODULE,
    CONTRAINT_STRUCT,
    CONTRAINT_INTERFACE,
    CONTRAINT_ENUM,
    CONTRAINT_FIELD,
    CONTRAINT_PARAMETER,
    CONTRAINT_GENERIC,
    CONTRAINT_DECLARATION,
    CONTRAINT_TYPE,
} ConstraintKind;

typedef struct {
    ConstraintKind kind;

    size_t ownedCount;
    Constraint *next;

    Symbol data;
    Constraint *generics;
} Constraint;

#endif
