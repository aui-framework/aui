#pragma once

#include <AUI/Common/AObject.h>
#include "ALayout.h"
#include <AUI/Traits/concepts.h>

class AView;

namespace aui::detail {
    template<aui::convertible_to<_<AView>> Storage = _<AView>>
    class LinearLayoutImpl: public ALayout {
    public:
        void removeView(aui::no_escape<AView> view, size_t index) override {
            if constexpr (std::is_same_v<Storage, _<AView>>) {
                assert(mViews[index].get() == view.ptr());
            }
            mViews.removeAt(index);
        }

        AVector<_<AView>> getAllViews() override {
            return { mViews.begin(), mViews.end() };
        }

    protected:
        void addViewBasicImpl(Storage view, AOptional<size_t> index) {
            auto at = mViews.end();
            if (index) {
                at = mViews.begin() + *index;
            }
            mViews.insert(at, std::move(view));
        }

    protected:
        AVector<Storage> mViews;
    };

}

/**
 * @brief Implements addView/removeView/getAllViews and protected mViews field for Vertical, Horizontal and Stacked layouts.
 * @tparam Storage optional storage type. See details for further info.
 * @details
 * @note The Storage type is `_<AView>` by default. If you specify your own type, you would have to implement addView
 *       to fill your custom type with data, you may use addViewBasicImpl then.
 *
 */
template<aui::convertible_to<_<AView>> Storage = _<AView>>
class ALinearLayout: public aui::detail::LinearLayoutImpl<Storage> {};

template<>
class ALinearLayout<_<AView>>: public aui::detail::LinearLayoutImpl<_<AView>> {
public:
    void addView(const _<AView>& view, AOptional<size_t> index) override {
        addViewBasicImpl(view, index);
    }
};
