#pragma once

#include <AUI/Common/AException.h>
#include <any>
#include "AListModelIndex.h"

class ATreeModelIndex {
public:
    class Exception: public AException {
    public:
        using AException::AException;
    };

    ATreeModelIndex() = default;

    /**
     * @param row row of the vertex relative to it's parent. If vertex is root, row should be 0.
     * @param column column of the vertex relative to it's parent. If vertex is root, column should be 0.
     * @param userdata the data stored in this vertex.
     */
    explicit ATreeModelIndex(std::size_t row, std::size_t column, std::any userdata) noexcept:
        mRow(row),
        mColumn(column),
        mUserdata(std::move(userdata)) {}

    template<typename T>
    [[nodiscard]]
    T as() const {
        try {
            return std::any_cast<T>(mUserdata);
        } catch (...) {
            throw Exception(
                    "bad userdata type (expected {}, holds {})"_format(typeid(T).name(), mUserdata.type().name()));
        }
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return mUserdata.has_value();
    }

    [[nodiscard]]
    std::size_t row() const {
        return mRow;
    }

    [[nodiscard]]
    std::size_t column() const {
        return mColumn;
    }

private:
    std::size_t mRow, mColumn;
    std::any mUserdata;
};

