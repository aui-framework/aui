#include "ViewDetails.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Platform/AClipboard.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AText.h>
#include <AUI/View/Dynamic.h>

#include "network/ViewTree.h"

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

_<AView> title(AString text) { return Label { std::move(text) } AUI_OVERRIDE_STYLE { FontSize { 14_pt } }; }

AString boxText(const std::array<int, 4>& fields) {
    // AUI's order is (top, right, bottom, left), as in CSS
    return "{} {} {} {}"_format(fields[1], fields[2], fields[3], fields[0]);
}

_<AView> propertiesView(const ViewDetailsModel& model) {
    AVector<_<AView>> views;
    if (!model.propertiesError.empty()) {
        views << (Label { model.propertiesError } AUI_OVERRIDE_STYLE { TextColor { AColor::RED } });
    } else if (!model.properties) {
        views << (Label { "Loading properties..." } AUI_OVERRIDE_STYLE { Opacity { 0.6f } });
    } else {
        for (const char* key : { "enabled", "focused", "role" }) {
            if (auto v = model.properties->containsOpt(key)) {
                views << row(key, valueText(*v));
            }
        }
        if (auto inspector = model.properties->containsOpt("inspector"); inspector && inspector->isObject()) {
            views << (Label { "Inspector" } AUI_OVERRIDE_STYLE { FontSize { 11_pt }, Margin { 4_dp, 0, 0, 0 } });
            for (const auto& [name, value] : inspector->asObject()) {
                views << row(name, valueText(value));
            }
        }
    }
    auto result = Vertical {} AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Margin { 8_dp, 0, 0, 0 } };
    result->setViews(std::move(views));
    return result;
}

_<AView> content(const ViewDetailsModel& model) {
    auto result = Vertical::Expanding {} AUI_OVERRIDE_STYLE { LayoutSpacing { 2_dp }, Padding { 4_dp } };
    if (model.ref.empty()) {
        result->setViews({ AText::fromString("Hover a view in the tree to see its box model, click it to see the "
                                             "details. Hold CTRL over the frame to pick a view.")
                               AUI_OVERRIDE_STYLE { Opacity { 0.6f } } });
        return result;
    }
    const auto& fields = model.fields;
    auto field = [&](const char* key) -> AString {
        auto v = fields.containsOpt(key);
        return v ? valueText(*v) : AString {};
    };
    const auto box = ViewTree::boxOf(fields);

    AVector<_<AView>> views {
        title(ViewTree::prettyName(fields)),
        Horizontal {
          Label { model.ref },
          Button { Label { "Copy" }, [ref = model.ref] { AClipboard::copyToClipboard(ref); } },
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
    views << propertiesView(model);
    result->setViews(std::move(views));
    return result;
}

}   // namespace

_<AView> viewDetails(_<AProperty<ViewDetailsModel>> model) {
    return Stacked {
        AScrollArea::Builder()
            .withContents(experimental::Dynamic { AUI_REACT(content(**model)) } AUI_OVERRIDE_STYLE { Expanding() })
            .build() AUI_LET { it->setExpanding(); },
    };
}
