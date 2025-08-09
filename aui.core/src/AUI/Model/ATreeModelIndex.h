#pragma once

#include <AUI/Common/AException.h>
#include <any>
#include <variant>

/**
 * @brief Valid index of ITreeModel.
 */
class ATreeModelIndex {
public:
    class Exception: public AException {
    public:
        using AException::AException;
    };

    /**
     * @param row row of the vertex relative to it's parent.
     * @param column column of the vertex relative to it's parent.
     * @param userdata userdata stored in this vertex.
     */
    explicit ATreeModelIndex(std::size_t row, std::size_t column, std::any userdata) noexcept:
        mRow(row),
        mColumn(column),
        mUserdata(std::move(userdata)) {}

    /**
     * @brief any_cast the external user data stored in this vertex.
     * @sa mUserdata
     */
    template<typename T>
    [[nodiscard]]
    T as() const {
        try {
            return std::any_cast<T>(mUserdata);
        } catch (...) {
            throw Exception("ATreeModelIndex::as any_cast failed (asked for {}, actually stored {})"_format(
                typeid(T).name(), mUserdata.type().name()));
        }
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return mUserdata.has_value();
    }

    /**
     * @brief row of the vertex relative to it's parent.
     */
    [[nodiscard]]
    std::size_t row() const {
        return mRow;
    }

    /**
     * @brief column of the vertex relative to it's parent.
     */
    [[nodiscard]]
    std::size_t column() const {
        return mColumn;
    }

    /**
    * @brief Tag type to define root vertex.
    * @details
    * Use ATreeModelIndex::ROOT to reference.
    */
    static constexpr struct {} ROOT {};

private:
    /**
     * @brief Row of the vertex relative to it's parent.
     */
    std::size_t mRow;

    /**
     * @brief Column of the vertex relative to it's parent.
     */
    std::size_t mColumn;
    
    /**
     * @brief External user data that helps the ITreeModel implementation to identify the vertex.
     */
    std::any mUserdata;
};

/**
 * @brief Like ATreeModelIndex, but allows to refer to ATreeModelIndex::ROOT instead of index.
 * @details
 * In addition to default variant functions (std::get, std::get_if, std::visit), you can also use the following
 * expressions:
 * ```cpp
 * bool isRoot = vertex == ATreeModelIndex::ROOT;
 * if (!isRoot) {
 *   ATreeModelIndex = *vertex;
 * }
 * ```
 */
using ATreeModelIndexOrRoot = std::variant<ATreeModelIndex, decltype(ATreeModelIndex::ROOT)>;

// these allow the following expression:
// vertex == ATreeModelIndex::ROOT

inline bool operator==(const ATreeModelIndexOrRoot& lhs, const decltype(ATreeModelIndex::ROOT)&) {
    return !std::holds_alternative<ATreeModelIndex>(lhs);
}

inline bool operator!=(const ATreeModelIndexOrRoot& lhs, const decltype(ATreeModelIndex::ROOT)&) {
    return std::holds_alternative<ATreeModelIndex>(lhs);
}

// this allows the following expression:
// ATreeModelIndexOrRoot vertex = ...
// ATreeModelIndex index = *vertex

inline const ATreeModelIndex& operator*(const ATreeModelIndexOrRoot& v) {
    if (auto i = std::get_if<ATreeModelIndex>(&v)) {
        return *i;
    }
    throw ATreeModelIndex::Exception("ATreeModelIndexOrRoot holds root");
}
