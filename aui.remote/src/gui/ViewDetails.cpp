#include "ViewDetails.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Platform/AClipboard.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AText.h>

#include "remote/ViewTree.h"

using namespace declarative;
using namespace aui_remote;

namespace {

AString valueText(const AJson& value) {
    if (value.isString()) {
        return value.asString();
    }
    return AJson::toString(value);
}

_<AView> row(const AString& name, const AString& value) {
    return Horizontal {
        Label { name } AUI_OVERRIDE_STYLE { Opacity { 0.6f }, MinSize { 96_dp, {} } },
        Label { value },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } };
}

AString boxText(const std::array<int, 4>& fields) {
    // AUI's order is (top, right, bottom, left), as in CSS
    return "{} {} {} {}"_format(fields[1], fields[2], fields[3], fields[0]);
}

}   // namespace

ViewDetails::ViewDetails() {
    mContent = Vertical {} AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Padding { 4_dp } };
    setContents(Stacked { AScrollArea::Builder().withContents(mContent).build() AUI_LET { it->setExpanding(); } });
    clear();
}

void ViewDetails::clear() {
    mRef.clear();
    mProperties = nullptr;
    mContent->setViews({ AText::fromString("Hover a view in the tree to see its box model, click it to see the "
                                           "details. Hold CTRL over the frame to pick a view.")
                             AUI_OVERRIDE_STYLE { Opacity { 0.6f } } });
}

void ViewDetails::show(const AString& ref, const AJson& fields) {
    if (ref.empty()) {
        clear();
        return;
    }
    mRef = ref;
    auto field = [&](const char* key) -> AString {
        auto v = fields.containsOpt(key);
        return v ? valueText(*v) : AString {};
    };
    const auto box = ViewTree::boxOf(fields);

    AVector<_<AView>> views {
        Label { ViewTree::prettyName(fields) } AUI_OVERRIDE_STYLE { FontSize { 14_pt } },
        Horizontal {
          Label { ref },
          Button { .content = Label { "Copy" }, .onClick = [ref] { AClipboard::copyToClipboard(ref); } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
        row("Position", "{}, {} (in parent {}, {})"_format(box.position.x, box.position.y, field("x"), field("y"))),
        row("Size", "{}x{}"_format(box.size.x, box.size.y)),
        row("Padding", boxText(box.padding)),
        row("Margin", boxText(box.margin)),
        row("Visible", field("visible")),
    };
    if (auto text = field("text"); !text.empty()) {
        views << row("Text", text);
    }
    if (auto names = field("name"); !names.empty()) {
        views << row("ASS names", names.replacedAll(" ", ", "));
    }
    mProperties = Vertical { Label { "Loading properties..." } AUI_OVERRIDE_STYLE { Opacity { 0.6f } } };
    mProperties AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Margin { 8_dp, 0, 0, 0 } };
    views << mProperties;
    mContent->setViews(std::move(views));
}

void ViewDetails::setProperties(const AString& ref, const AJson& properties) {
    if (ref != mRef || !mProperties) {
        return;
    }
    AVector<_<AView>> views;
    for (const char* key : { "enabled", "focused", "role" }) {
        if (auto v = properties.containsOpt(key)) {
            views << row(key, valueText(*v));
        }
    }
    if (auto inspector = properties.containsOpt("inspector"); inspector && inspector->isObject()) {
        views << (Label { "Inspector" } AUI_OVERRIDE_STYLE { FontSize { 11_pt }, Margin { 4_dp, 0, 0, 0 } });
        for (const auto& [name, value] : inspector->asObject()) {
            views << row(name, valueText(value));
        }
    }
    mProperties->setViews(std::move(views));
}

void ViewDetails::setPropertiesError(const AString& ref, const AString& message) {
    if (ref != mRef || !mProperties) {
        return;
    }
    mProperties->setViews({ Label { message } AUI_OVERRIDE_STYLE { TextColor { AColor::RED } } });
}
