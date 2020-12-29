#pragma once

namespace aui {

    /**
     * \brief Враппер для объектов, предоставляющие функции итераторов rbegin и rend (ReverseIterable). Используется
     *        для foreach-циклов
     * \example
     * <pre>
     * for (auto& view : aui::reverse_iterator_wrap(mViews)) { ... }
     * </pre>
     * \tparam T ReverseIterable
     */
    template <typename T>
    struct reverse_iterator_wrap {
    private:
        T& mIterable;

    public:
        using iterator = typename T::reverse_iterator;
        reverse_iterator_wrap(T& mIterable) : mIterable(mIterable) {}

        iterator begin() {
            return mIterable.rbegin();
        }
        iterator end() {
            return mIterable.rend();
        }
    };
}