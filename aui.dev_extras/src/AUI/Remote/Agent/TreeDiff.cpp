/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TreeDiff.h"

namespace aui::remote::tree_diff {

Diff diff(const Tree& current) {
    Diff result;
    result.root = current.root;
    result.upsert = current.nodes;
    return result;
}

Diff diff(const Tree& base, std::int64_t baseSeq, const Tree& current) {
    Diff result;
    result.base = baseSeq;
    result.root = current.root;
    for (const auto& [ref, node] : current.nodes) {
        if (auto it = base.nodes.find(ref); it != base.nodes.end() && it->second == node) {
            continue;
        }
        result.upsert[ref] = node;
    }
    for (const auto& [ref, _] : base.nodes) {
        if (!current.nodes.contains(ref)) {
            result.remove.push_back(ref);
        }
    }
    return result;
}

Tree apply(const Diff& diff, const Tree& base) {
    Tree result;
    if (diff.base) {
        result.nodes = base.nodes;
        for (const auto& ref : diff.remove) {
            result.nodes.erase(ref);
        }
    }
    for (const auto& [ref, node] : diff.upsert) {
        result.nodes[ref] = node;
    }
    result.root = diff.root;
    return result;
}

AJson toJson(const Node& node) {
    AJson json {
        { "type", node.type },
        { "x", node.x },
        { "y", node.y },
        { "w", node.w },
        { "h", node.h },
        { "absX", node.absX },
        { "absY", node.absY },
        { "visible", node.visible },
    };
    if (node.text) {
        json["text"] = *node.text;
    }
    if (node.name) {
        json["name"] = *node.name;
    }
    aui::impl::JsonArray children;
    for (const auto& child : node.children) {
        children.push_back(AJson(child));
    }
    json["children"] = AJson(std::move(children));
    return json;
}

Node nodeFromJson(const AJson& json) {
    Node node;
    node.type = json["type"].asString();
    if (auto text = json.containsOpt("text")) {
        node.text = text->asString();
    }
    if (auto name = json.containsOpt("name")) {
        node.name = name->asString();
    }
    node.x = json["x"].asInt();
    node.y = json["y"].asInt();
    node.w = json["w"].asInt();
    node.h = json["h"].asInt();
    node.absX = json["absX"].asInt();
    node.absY = json["absY"].asInt();
    node.visible = json["visible"].asBool();
    for (const auto& child : json["children"].asArray()) {
        node.children.push_back(child.asString());
    }
    return node;
}

AJson toJson(const Diff& diff) {
    aui::impl::JsonObject upsert;   // not operator[]: it would make the building quadratic
    for (const auto& [ref, node] : diff.upsert) {
        upsert.push_back({ ref, toJson(node) });
    }
    aui::impl::JsonArray remove;
    for (const auto& ref : diff.remove) {
        remove.push_back(AJson(ref));
    }
    AJson json {
        { "base", diff.base ? AJson(*diff.base) : AJson(nullptr) },
        { "root", diff.root },
    };
    json["upsert"] = AJson(std::move(upsert));
    json["remove"] = AJson(std::move(remove));
    return json;
}

Diff diffFromJson(const AJson& json) {
    Diff diff;
    if (auto base = json.containsOpt("base"); base && !base->isNull()) {
        diff.base = base->asLongInt();
    }
    diff.root = json["root"].asString();
    for (const auto& [ref, node] : json["upsert"].asObject()) {
        diff.upsert[ref] = nodeFromJson(node);
    }
    for (const auto& ref : json["remove"].asArray()) {
        diff.remove.push_back(ref.asString());
    }
    return diff;
}

}   // namespace aui::remote::tree_diff
