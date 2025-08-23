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

const req = new XMLHttpRequest();
req.open("GET", new URL("search/search_index.json", new URL("..", location.href)));
req.responseType = "blob";

// Handle response
req.addEventListener("load", async () => {
    if (req.status >= 200 && req.status < 300) {
        const search_index = JSON.parse(await req.response.text());
        console.log(search_index);
        document.querySelectorAll(".md-content p a, .md-content table a, .md-content code a").forEach(element => {
            const href = element.getAttribute("href");
            if (!href) {
                return;
            }
            let target = null;
            for (const i of search_index["docs"]) {
                if (i.location.length === 0) {
                    continue;
                }
                if (href.endsWith("/" + i.location)) {
                    target = i;
                    element.setAttribute("aui-instant-preview", "");
                    break;
                }
            }
            element.addEventListener("mouseenter", () => {
                element.insertAdjacentHTML("beforebegin", `<span id="aui-instant-preview-container"><div class="aui-instant-preview-content"><h2>${target.title}</h2>${target.text}</div></span>`);
            });

            element.addEventListener("mouseleave", () => {
                // Handle mouseleave
                // document.querySelector("#aui-instant-preview-container").remove();
            });
        });


        // Every response that is not in the 2xx range is considered an error
    } else {
        console.log(req.statusText);
    }
});

// Handle network errors
req.addEventListener("error", () => {
    console.log("Network error");
})

// Handle aborted requests
req.addEventListener("abort", () => {
    console.log("Network error");
})

// Send request and automatically abort request upon unsubscription
req.send();
