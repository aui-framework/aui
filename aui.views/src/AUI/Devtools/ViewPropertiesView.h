#pragma once


#include <AUI/View/AScrollArea.h>

class ViewPropertiesView: public AScrollArea {
private:
    _weak<AView> mTargetView;

    void requestTargetUpdate();
public:
    explicit ViewPropertiesView(const _<AView>& targetView);
    void displayApplicableRule(const _<AViewContainer>& dst,
                               ADeque<ass::decl::IDeclarationBase*>& applicableDeclarations,
                               const RuleWithoutSelector* rule);
    void setTargetView(const _<AView>& targetView);
};


