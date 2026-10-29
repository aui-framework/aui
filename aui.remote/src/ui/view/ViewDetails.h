#pragma once

#include <AUI/Json/AJson.h>
#include <AUI/View/AViewContainer.h>

/**
 * @brief Details of the selected view, like the properties panel of the in-process devtools.
 * @details
 * What the view tree knows (name, geometry, box model) is shown right away; debugInspectorInfo and the state come
 * with `view.properties`, see setProperties().
 */
class ViewDetails : public AViewContainerBase {
public:
    ViewDetails();

    /**
     * @brief Shows the view.
     * @param ref ref of the view; empty clears the panel
     * @param fields fields of the view in the tree (ViewTree::fields())
     */
    void show(const AString& ref, const AJson& fields);

    void clear();

    /**
     * @brief Adds the result of `view.properties`. Ignored unless the view is still shown.
     */
    void setProperties(const AString& ref, const AJson& properties);

    /**
     * @brief `view.properties` has failed. Ignored unless the view is still shown.
     */
    void setPropertiesError(const AString& ref, const AString& message);

    [[nodiscard]] const AString& ref() const noexcept { return mRef; }

private:
    AString mRef;
    _<AViewContainer> mContent;
    _<AViewContainer> mProperties;
};
