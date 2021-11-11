#pragma once


#include <AUI/View/AScrollArea.h>

class ViewPropertiesView: public AScrollArea {
private:
    AView* mTargetView;

    void requestTargetUpdate();
public:
    explicit ViewPropertiesView(AView* targetView);
    void displayApplicableRule(ADeque<ass::decl::IDeclarationBase*>& applicableDeclarations, const RuleWithoutSelector* rule);
    void setTargetView(AView* targetView);
};


