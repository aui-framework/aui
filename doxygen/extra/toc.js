/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function findNearestVisibleElement(query) {
    var currentAnchor = null;
    query.each((key, anchor) => {
        if (currentAnchor != null) {
            if (anchor.getBoundingClientRect().y > anchor.getBoundingClientRect().height * 5) {
                return false;
            }
        }
        currentAnchor = anchor;
    });
    return currentAnchor
}

addEventListener("DOMContentLoaded", (event) => {
    var anchors = $('.aui-toc-hash');
    sourceLocationMarkers = $('b[aui-src]');
    var prevAnchor  = null;
    var onScroll = (event) => {
        var currentAnchor = findNearestVisibleElement(anchors);
        if (currentAnchor == null) {
            return;
        }
        if (prevAnchor === currentAnchor) {
            return;
        }
        $(".aui-toc a").removeAttr("active");
        const href = currentAnchor.getAttribute("href");
        $(".aui-toc a[href=\"" + href + "\"]").attr("active", true);
    }
    document.getElementById("doc-content").addEventListener('scroll', onScroll);
    addEventListener('scroll', onScroll);
});


function jumpToSource(action) {
    var marker = findNearestVisibleElement(sourceLocationMarkers);
    if (marker == null) {
        return;
    }
    var src = marker.getAttribute("aui-src");

    let PREFIX = action === 'view' ? "https://github.com/aui-framework/aui/blob/develop/" : "https://github.com/aui-framework/aui/edit/develop/"
    window.open(PREFIX + src, '_blank').focus();
}
