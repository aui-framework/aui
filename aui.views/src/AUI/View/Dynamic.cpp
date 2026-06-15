#include "Dynamic.h"

namespace {
class DynamicView: public AViewContainer {
public:
    DynamicView() {
    }

    void setContent(AArc<AView> v) {
        ALayoutInflater::inflate(this, std::move(v));
    }
};
}

namespace declarative::experimental {

AArc<AView> Dynamic::operator()() {
    auto v = _new<DynamicView>();
    content.bindTo(ASlotDef{AUI_SLOT(v.get())::setContent});
    return v;
}

}