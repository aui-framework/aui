#pragma once

#include <AUI/View/AViewContainer.h>

class API_AUI_VIEWS ALayoutInflater {
public:
    /**
     * @brief Wraps <code>view</code> with <code>root</code> using <code>Stacked</code> layout and expanding.
     * @details Used when usage of AViewContainer::setContents is not possible (see AViewContainer::setContents)
     *
     * @param root container to wrap with
     * @param view view to be wrapped
     * @note Clears contents of <code>root</code>.
     * @note Stolen from Android.
     * @return <code>root</code>
     */
    static const _<AViewContainer>& inflate(const _<AViewContainer>& root, const _<AView>& view);
};


