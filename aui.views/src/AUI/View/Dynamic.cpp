#include "Dynamic.h"

namespace {
class DynamicView: public AViewContainer {
public:
    DynamicView() {
        connect(content.changed, [this](_<AView> v) {
            ALayoutInflater::inflate(this, std::move(v));
        });
    }

    AProperty<_<AView>> content;
};
}

namespace declarative::experimental {

_<AView> Dynamic::operator()() {
    auto v = _new<DynamicView>();
    content.bindTo(v->content);
    return v;
}

}