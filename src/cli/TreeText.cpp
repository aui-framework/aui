#include "TreeText.h"

namespace aui_remote {

namespace {

constexpr std::size_t MAX_TEXT = 80;

AString quote(const AString& text) {
    AString shortened = text.length() > MAX_TEXT ? text.substr(0, MAX_TEXT) + "…" : text;
    return AJson::toString(AJson(shortened));
}

void render(const AJson& node, int depth, AString& out) {
    out += AString(depth * 2, ' ');
    out += node.containsOpt("type") ? node["type"].asString() : AString("?");
    if (auto text = node.containsOpt("text"); text && text->isString() && !text->asString().empty()) {
        out += " " + quote(text->asString());
    }
    if (auto ref = node.containsOpt("ref"); ref && ref->isString()) {
        out += " [" + ref->asString() + "]";
    }
    if (auto g = node.containsOpt("geometry"); g && g->isObject()) {
        out += " {}x{} @{},{}"_format((*g)["w"].asInt(), (*g)["h"].asInt(), (*g)["absX"].asInt(),
                                       (*g)["absY"].asInt());
    }
    auto isFalse = [&](const char* name) {
        auto v = node.containsOpt(name);
        return v && v->isBool() && !v->asBool();
    };
    auto isTrue = [&](const char* name) {
        auto v = node.containsOpt(name);
        return v && v->isBool() && v->asBool();
    };
    if (isFalse("visible")) {
        out += " hidden";
    }
    if (isFalse("enabled")) {
        out += " disabled";
    }
    if (isTrue("focused")) {
        out += " focused";
    }
    if (auto actions = node.containsOpt("actions"); actions && actions->isArray()) {
        for (const auto& action : actions->asArray()) {
            out += " " + action.asString();
        }
    }
    out += "\n";
    if (auto children = node.containsOpt("children"); children && children->isArray()) {
        for (const auto& child : children->asArray()) {
            render(child, depth + 1, out);
        }
    }
}

}   // namespace

AString renderTreeText(const AJson& root) {
    AString out;
    render(root, 0, out);
    return out;
}

}   // namespace aui_remote
