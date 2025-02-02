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

addEventListener("DOMContentLoaded", (event) => {
    var anchors = $('.aui-toc-hash');
    var prevAnchor  = null;
    var onScroll = (event) => {
        var currentAnchor = null;
        anchors.each((key, anchor) => {
            if (anchor.getBoundingClientRect().y > anchor.getBoundingClientRect().height * 5) {
                return false;
            }
            currentAnchor = anchor;
        });
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
