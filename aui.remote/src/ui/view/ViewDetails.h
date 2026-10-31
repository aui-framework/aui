#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Json/AJson.h>
#include <AUI/View/AView.h>

/**
 * @brief What the panel of view details shows.
 */
struct ViewDetailsModel {
    /**
     * @brief Ref of the view; empty if none.
     */
    AString ref;

    /**
     * @brief Fields of the view in the tree (ViewTree::fields()): name, geometry, box model.
     */
    AJson fields;

    /**
     * @brief The result of `view.properties`; it comes later than the rest.
     */
    AOptional<AJson> properties;

    /**
     * @brief `view.properties` has failed.
     */
    AString propertiesError;
};

/**
 * @brief Details of the selected view, like the properties panel of the in-process devtools.
 */
_<AView> viewDetails(_<AProperty<ViewDetailsModel>> model);
