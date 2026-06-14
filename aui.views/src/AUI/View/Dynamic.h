#pragma once

#include "AAbstractTypeableView.h"
#include <AUI/Util/Declarative/Contracts.h>

namespace declarative::experimental {
/**
 * @brief A view that exposes a contract::In<_<AView>> to provide dynamic content.
 *
 * The view simply forwards the provided child view as its content. It can be used in declarative UI to inject
 * arbitrary views at runtime.
 */
class API_AUI_VIEWS Dynamic {
public:
    // Declare contract input for a view
    contract::In<_<AView>> content;

    _<AView> operator()();
};
}