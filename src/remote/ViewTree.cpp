#include "ViewTree.h"

#include <unordered_set>

namespace aui_remote {

namespace {

[[noreturn]] void invalid(const AString& what) { throw AException("invalid view tree diff: " + what); }

const AJson& require(const AJson& object, const char* key, const char* where) {
    if (!object.isObject()) {
        invalid(AString(where) + " is not an object");
    }
    auto v = object.containsOpt(key);
    if (!v) {
        invalid(AString("no `") + key + "` in " + where);
    }
    return *v;
}

AString signature(const AJson& fields, const AVector<AString>& children) {
    AJson::Array c;
    for (const auto& ref : children) {
        c.push_back(AJson(ref));
    }
    return AJson::toString(fields) + "|" + AJson::toString(AJson(std::move(c)));
}

int intField(const AJson& fields, const char* key) {
    auto v = fields.containsOpt(key);
    return v && v->isNumber() ? v->asInt() : 0;
}

std::array<int, 4> boxFields(const AJson& fields, const char* key) {
    std::array<int, 4> result {};
    if (auto v = fields.containsOpt(key); v && v->isArray()) {
        const auto& array = v->asArray();
        for (std::size_t i = 0; i < result.size() && i < array.size(); ++i) {
            result[i] = array[i].isNumber() ? array[i].asInt() : 0;
        }
    }
    return result;
}

AString stripTypePrefixes(AString name) {
    if (name.startsWith("class ")) {
        name = name.substr(6);
    }
    if (name.startsWith("LayoutVisitor::")) {
        name = "A" + name.substr(15);
    }
    return name;
}

}   // namespace

void ViewTree::apply(const AJson& treeDiff) {
    ViewTree next = *this;
    next.applyImpl(treeDiff);
    next.mSeq = mSeq;
    *this = std::move(next);
}

void ViewTree::apply(const AJson& treeDiff, int seq) {
    ViewTree next = *this;
    const auto& base = require(treeDiff, "base", "tree");
    if (base.isNull()) {
        // a full tree: it stands on its own
    } else if (!base.isInt()) {
        invalid("`base` is not an int or null");
    } else if (!mSeq || *mSeq != base.asInt()) {
        throw AException(
            "view tree desync: the diff is built against frame {}, the tree is at {}"_format(
                base.asInt(), mSeq ? AString::number(*mSeq) : AString("nothing")));
    }
    next.applyImpl(treeDiff);
    next.mSeq = seq;
    *this = std::move(next);
}

void ViewTree::applyImpl(const AJson& diff) {
    const auto& base = require(diff, "base", "tree");
    if (base.isNull()) {
        mNodes.clear();
        mRoot.clear();
    }
    if (auto remove = diff.containsOpt("remove"); remove && !remove->isNull()) {
        if (!remove->isArray()) {
            invalid("`remove` is not an array");
        }
        for (const auto& ref : remove->asArray()) {
            if (!ref.isString()) {
                invalid("a ref in `remove` is not a string");
            }
            mNodes.erase(ref.asString().toStdString());
        }
    }
    if (auto upsert = diff.containsOpt("upsert"); upsert && !upsert->isNull()) {
        if (!upsert->isObject()) {
            invalid("`upsert` is not an object");
        }
        for (const auto& [ref, json] : upsert->asObject()) {
            if (!json.isObject()) {
                invalid("node " + ref + " is not an object");
            }
            Node node;
            node.fields = AJson::Object {};
            for (const auto& [key, value] : json.asObject()) {
                if (key == "children") {
                    if (!value.isArray()) {
                        invalid("`children` of " + ref + " is not an array");
                    }
                    for (const auto& child : value.asArray()) {
                        if (!child.isString()) {
                            invalid("a child of " + ref + " is not a string");
                        }
                        node.children << child.asString();
                    }
                } else if (key != "ref") {
                    node.fields[key] = value;
                }
            }
            mNodes[ref.toStdString()] = std::move(node);
        }
    }
    const auto& root = require(diff, "root", "tree");
    if (!root.isString()) {
        invalid("`root` is not a string");
    }
    mRoot = root.asString();

    // the tree must be whole: every child exists, no view has two parents (this also rules out cycles)
    if (!mNodes.contains(mRoot.toStdString())) {
        invalid("root " + mRoot + " is not in the tree");
    }
    std::unordered_set<std::string> visited;
    AVector<std::string> stack { mRoot.toStdString() };
    while (!stack.empty()) {
        auto ref = std::move(stack.back());
        stack.pop_back();
        if (!visited.insert(ref).second) {
            invalid("view " + AString::fromUtf8(ref) + " is reachable twice");
        }
        for (const auto& child : mNodes.at(ref).children) {
            auto it = mNodes.find(child.toStdString());
            if (it == mNodes.end()) {
                invalid("view " + child + " is referenced but absent");
            }
            stack << child.toStdString();
        }
    }
    // forget views that left the tree; the agent is not obliged to `remove` them
    for (auto it = mNodes.begin(); it != mNodes.end();) {
        it = visited.contains(it->first) ? std::next(it) : mNodes.erase(it);
    }
}

const AJson* ViewTree::fields(const AString& ref) const {
    auto it = mNodes.find(ref.toStdString());
    return it == mNodes.end() ? nullptr : &it->second.fields;
}

const AVector<AString>& ViewTree::children(const AString& ref) const {
    static const AVector<AString> none;
    auto it = mNodes.find(ref.toStdString());
    return it == mNodes.end() ? none : it->second.children;
}

AVector<AString> ViewTree::pathTo(const AString& ref) const {
    if (empty() || !mNodes.contains(ref.toStdString())) {
        return {};
    }
    AVector<AString> path;
    auto find = [&](const auto& self, const AString& current) -> bool {
        path << current;
        if (current == ref) {
            return true;
        }
        for (const auto& child : children(current)) {
            if (self(self, child)) {
                return true;
            }
        }
        path.pop_back();
        return false;
    };
    find(find, mRoot);
    return path;
}

AVector<AString> ViewTree::hitTest(glm::vec2 position) const {
    auto contains = [&](const AString& ref) {
        const auto& f = *fields(ref);
        if (auto visible = f.containsOpt("visible"); visible && visible->isBool() && !visible->asBool()) {
            return false;
        }
        const glm::vec2 offset { intField(f, "absX"), intField(f, "absY") };
        const glm::vec2 size { intField(f, "w"), intField(f, "h") };
        const auto local = position - offset;
        return local.x >= 0 && local.y >= 0 && local.x < size.x && local.y < size.y;
    };
    AVector<AString> path;
    if (empty() || !contains(mRoot)) {
        return path;
    }
    path << mRoot;
    for (bool found = true; found;) {
        found = false;
        const auto& c = children(path.back());
        for (auto it = c.rbegin(); it != c.rend(); ++it) {
            if (contains(*it)) {
                path << *it;
                found = true;
                break;
            }
        }
    }
    return path;
}

std::optional<ViewTree::Box> ViewTree::box(const AString& ref) const {
    const auto* f = fields(ref);
    if (!f) {
        return std::nullopt;
    }
    return boxOf(*f);
}

ViewTree::Box ViewTree::boxOf(const AJson& fields) {
    return Box {
        .position = { intField(fields, "absX"), intField(fields, "absY") },
        .size = { intField(fields, "w"), intField(fields, "h") },
        .padding = boxFields(fields, "padding"),
        .margin = boxFields(fields, "margin"),
    };
}

AString ViewTree::prettyName(const AJson& fields) {
    auto string = [&](const char* key) -> AString {
        auto v = fields.containsOpt(key);
        return v && v->isString() ? v->asString() : AString {};
    };
    AString name = stripTypePrefixes(string("type"));
    if (auto layout = string("layout"); !layout.empty()) {
        name = "{} ({})"_format(stripTypePrefixes(layout), name);
    }
    // the last ASS name that is not an internal one
    auto names = string("name").split(' ');
    for (auto it = names.rbegin(); it != names.rend(); ++it) {
        if (!it->empty() && !it->contains("::detail::")) {
            name += " " + *it;
            break;
        }
    }
    return name;
}

AJson ViewTree::nodeJson(const AString& ref, int depth) const {
    const auto& node = mNodes.at(ref.toStdString());
    AJson result = AJson::Object {};
    result["ref"] = ref;
    for (const auto& [key, value] : node.fields.asObject()) {
        result[key] = value;
    }
    if (!node.fields.containsOpt("geometry") && node.fields.containsOpt("w")) {
        AJson geometry = AJson::Object {};
        for (const char* key : { "x", "y", "w", "h", "absX", "absY" }) {
            if (auto v = node.fields.containsOpt(key)) {
                geometry[key] = *v;
            }
        }
        result["geometry"] = std::move(geometry);
    }
    AJson::Array children;
    for (const auto& child : node.children) {
        children.push_back(nodeJson(child, depth + 1));
    }
    result["children"] = AJson(std::move(children));
    return result;
}

AJson ViewTree::toJson() const {
    if (empty()) {
        return AJson(nullptr);
    }
    return nodeJson(mRoot, 0);
}

AJson ViewTree::diff(const ViewTree& a, const ViewTree& b) {
    AJson result = AJson::Object {};
    const bool full = !a.mSeq || a.empty();
    result["base"] = full ? AJson(nullptr) : AJson(*a.mSeq);
    result["root"] = b.mRoot;
    AJson upsert = AJson::Object {};
    for (const auto& [ref, node] : b.mNodes) {
        auto it = full ? a.mNodes.end() : a.mNodes.find(ref);
        if (it != a.mNodes.end() && signature(it->second.fields, it->second.children) ==
                                        signature(node.fields, node.children)) {
            continue;
        }
        AJson json = node.fields;
        AJson::Array children;
        for (const auto& child : node.children) {
            children.push_back(AJson(child));
        }
        json["children"] = AJson(std::move(children));
        upsert[AString::fromUtf8(ref)] = std::move(json);
    }
    AJson::Array remove;
    for (const auto& [ref, node] : a.mNodes) {
        if (!full && !b.mNodes.contains(ref)) {
            remove.push_back(AJson(AString::fromUtf8(ref)));
        }
    }
    result["upsert"] = std::move(upsert);
    result["remove"] = AJson(std::move(remove));
    return result;
}

bool ViewTree::sameContent(const ViewTree& other) const {
    if (mRoot != other.mRoot || mNodes.size() != other.mNodes.size()) {
        return false;
    }
    for (const auto& [ref, node] : mNodes) {
        auto it = other.mNodes.find(ref);
        if (it == other.mNodes.end() ||
            signature(node.fields, node.children) != signature(it->second.fields, it->second.children)) {
            return false;
        }
    }
    return true;
}

}   // namespace aui_remote
