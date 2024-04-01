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

    /**
     * @brief any_cast the external user data stored in this vertex.
     */
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

    /**
     * @brief row of the vertex relative to it's parent. If vertex is root, row should be 0.
     */
    [[nodiscard]]
    std::size_t row() const {
        return mRow;
    }

    /**
     * @brief column of the vertex relative to it's parent. If vertex is root, column should be 0.
     */
    [[nodiscard]]
    std::size_t column() const {
        return mColumn;
    }

    /**
     * @brief check if this vertex is the root.
     */
    [[nodiscard]]
    bool isRoot() const noexcept {
        return mRow == 0 && mColumn == 0;
    }

private:
    std::size_t mRow;
    std::size_t mColumn;
    std::any mUserdata;
};

