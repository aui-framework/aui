#pragma once

#include <array>
#include <optional>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <AUI/Common/AException.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Json/AJson.h>

namespace aui_remote {

/**
 * @brief View tree of the remote application, kept flat (by ref) and updated with the diffs pushed by the agent.
 * @details
 * The diff has the form of `tree` of `frames.frame`:
 *
 * ```
 * { base: int|null, root: ref, upsert: { ref: Node }, remove: [ref] }
 * Node = { type, text?, name?, layout?, x, y, w, h, absX, absY, visible, padding?, margin?, children: [ref...], ... }
 * ```
 *
 * `base` is the seq of the frame the diff is built against. `null` means a full tree: whatever the tree held is dropped.
 * Fields of a node the tree does not know are kept as is.
 */
class ViewTree {
public:
    /**
     * @brief Applies a diff. The tree is left untouched if it throws.
     * @throws AException if the diff is malformed or refers to unknown views
     */
    void apply(const AJson& treeDiff);

    /**
     * @brief Applies a diff of the frame `seq`, checking that it is built against the tree this object holds.
     * @param seq seq of the frame the diff arrived with; becomes baseSeq()
     * @throws AException on desync (`base` of the diff differs from baseSeq()), or as apply(const AJson&)
     */
    void apply(const AJson& treeDiff, int seq);

    [[nodiscard]] bool empty() const noexcept { return mNodes.empty(); }

    /**
     * @return seq of the last frame applied with apply(const AJson&, int); nullopt if none
     */
    [[nodiscard]] std::optional<int> baseSeq() const { return mSeq; }

    /**
     * @return ref of the root view; empty if the tree is empty
     */
    [[nodiscard]] const AString& rootRef() const noexcept { return mRoot; }

    [[nodiscard]] std::size_t size() const noexcept { return mNodes.size(); }

    /**
     * @return fields of the view (everything but `children`); null if there is no such view
     */
    [[nodiscard]] const AJson* fields(const AString& ref) const;

    /**
     * @return refs of the children of the view; empty if there is no such view
     */
    [[nodiscard]] const AVector<AString>& children(const AString& ref) const;

    /**
     * @return refs from the root to the view inclusive; empty if there is no such view
     */
    [[nodiscard]] AVector<AString> pathTo(const AString& ref) const;

    /**
     * @brief Finds the view under a point of the window, like AView::getViewAtRecursive: the topmost (the last) visible
     * child that contains the point, recursively.
     * @return refs from the root to the deepest view under the point; empty if the point is outside the root
     */
    [[nodiscard]] AVector<AString> hitTest(glm::vec2 position) const;

    /**
     * @brief Box model of a view, in window coordinates. Fields are `{left, top, right, bottom}`.
     */
    struct Box {
        glm::ivec2 position { 0, 0 };
        glm::ivec2 size { 0, 0 };
        std::array<int, 4> padding {};
        std::array<int, 4> margin {};
    };

    /**
     * @return box model of the view; nullopt if there is no such view
     */
    [[nodiscard]] std::optional<Box> box(const AString& ref) const;

    /**
     * @param fields fields of a view, see fields()
     */
    [[nodiscard]] static Box boxOf(const AJson& fields);

    /**
     * @brief Name of a view as the in-process devtools show it: `AHorizontalLayout (AViewContainer) .name`.
     * @param fields fields of the view, see fields()
     */
    [[nodiscard]] static AString prettyName(const AJson& fields);

    /**
     * @return nested form identical in shape to the `view.tree` result: `{ref, type, text, geometry: {x, y, w, h,
     * absX, absY}, x, y, w, h, absX, absY, visible, children: [...]}`; null if empty. The geometry is duplicated flat
     * and as `geometry` so renderTreeText() works with it.
     */
    [[nodiscard]] AJson toJson() const;

    /**
     * @brief Diff that turns `a` into `b`. `base` is `a.baseSeq()` (null if `a` is empty).
     */
    [[nodiscard]] static AJson diff(const ViewTree& a, const ViewTree& b);

    /**
     * @return whether both trees hold the same views (the seq is not compared)
     */
    [[nodiscard]] bool sameContent(const ViewTree& other) const;

private:
    struct Node {
        AJson fields;                // everything but `children`
        AVector<AString> children;   // refs
    };
    std::unordered_map<std::string, Node> mNodes;
    AString mRoot;
    std::optional<int> mSeq;

    void applyImpl(const AJson& treeDiff);
    [[nodiscard]] AJson nodeJson(const AString& ref, int depth) const;
};

}   // namespace aui_remote
