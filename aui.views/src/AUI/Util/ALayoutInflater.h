#pragma once

#include <AUI/View/AViewContainer.h>

class API_AUI_VIEWS ALayoutInflater {
public:
    /**
     * <p>
     * Wraps <code>view</code> with <code>root</code> using <code>Stacked</code> layout and expanding.
     * </p>
     * <p>
     * Used when <a href="AViewContainer::setContents">AViewContainer::setContents</a> is unavailable due to access
     * visibility or your root is an instance of class derived from <a href="AViewContainer">AViewContainer</a>.
     * </p>
     *
     * @param root container to wrap with
     * @param view view to be wrapped
     * @note Clears contents of <code>root</code>.
     * @note Stolen from Android.
     * @return <code>root</code>
     */
    static const _<AViewContainer>& inflate(const _<AViewContainer>& root, const _<AView>& view);
};


