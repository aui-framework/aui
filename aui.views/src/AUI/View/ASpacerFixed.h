#pragma once

#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Fixed-size view which is useful in UI building.
 * @ingroup useful_views
 * @details
 *
 * ABlankSpace is fixed size blank view which acquires specified space.
 */
class API_AUI_VIEWS ASpacerFixed: public AView {
public:
    ASpacerFixed(AMetric space): mSpace(space) {}

    int getContentMinimumWidth(ALayoutDirection layout) override;

    int getContentMinimumHeight(ALayoutDirection layout) override;

private:
    AMetric mSpace;
};


namespace declarative {
    using SpacerFixed = aui::ui_building::view<ASpacerFixed>;
}