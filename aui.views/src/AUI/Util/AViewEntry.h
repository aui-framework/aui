#pragma once

#include <AUI/View/AView.h>
#include "AWordWrappingEngine.h"

class API_AUI_VIEWS AViewEntry: public AWordWrappingEngine::Entry {
private:
    _<AView> mView;

public:
    explicit AViewEntry(const _<AView>& view) : mView(view) {}
    AViewEntry() = default;

    glm::ivec2 getSize() override;
    void setPosition(const glm::ivec2& position) override;
    Float getFloat() const override;

    ~AViewEntry() override;
};


