#pragma once

#include <AUI/Data.h>
#include <AUI/Data/AModelMeta.h>
#include <AUI/Data/ASqlModel.h>

struct Note: ASqlModel<Note> {
    AString name;
    AString description;
};

A_META(Note) {
    A_SQL_TABLE("courses")
    A_FIELDS {
        return {
                A_FIELD(name)
                A_FIELD(description)
        };
    }
};