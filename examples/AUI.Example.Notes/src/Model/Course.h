#pragma once

#include <AUI/Data.h>
#include <AUI/Data/AModelMeta.h>
#include <AUI/Data/ASqlModel.h>

struct Course: ASqlModel<Course> {
    AString name;
    AString description;
};

A_META(Course) {
    A_SQL_TABLE("courses")
    A_FIELDS {
        return {
                A_FIELD(name)
                A_FIELD(description)
        };
    }
};