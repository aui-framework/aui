#pragma once

#include "AAbstractLabel.h"

/**
 * @brief Represents a simple single-line text display view.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ALabel.png">
 * @ingroup useful_views
 * @details
 * ALabel is used for displayed nonformatted single-line text.
 */
class API_AUI_VIEWS ALabel: public AAbstractLabel {
public:
    using AAbstractLabel::AAbstractLabel;
};


namespace declarative {
    using Label = aui::ui_building::view<ALabel>;
}