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
            let href = element.getAttribute("href");
            if (!href) {
                return;
            }
            if (!href.startsWith("../")) {
                return;
            }
            href = href.substring(3);
            let target = [];
            for (const i of search_index["docs"]) {
                if (i.location.length === 0) {
                    continue;
                }
                if (i.location.startsWith(href)) {
                    target.push(i);
                }
            }
            if (target.length === 0) {
                return;
            }
            element.setAttribute("aui-instant-preview", "");
            element.addEventListener("mouseenter", () => {
                let timeout = setTimeout(() => {
                    let prev = document.querySelector("#aui-instant-preview-popup");
                    if (prev != null) {
                        prev.remove();
                    }
                    let popup = document.createElement("div")
                    popup.id = "aui-instant-preview-popup";
                    let contents = `<a href="${element.getAttribute('href')}" style="opacity:0;font-size: .8rem;">${element.innerHTML}</a><div class="aui-instant-preview-inner">`;
                    for (const i of target) {
                        if (target.length === 0) {
                            contents += `<h1>${i.title}</h1>${i.text}`;
                        } else {
                            contents += `<h2>${i.title}</h2>${i.text}`;
                        }
                    }
                    contents += `</div>`;
                    popup.innerHTML = contents;
                    let position = element.getBoundingClientRect();
                    const scrollX = window.pageXOffset || document.documentElement.scrollLeft;
                    const scrollY = window.pageYOffset || document.documentElement.scrollTop;
                    popup.style.left = `${scrollX + position.left - 8}px`;
                    popup.style.top = `${scrollY + position.top}px`;
                    document.body.appendChild(popup);

                    popup.addEventListener("mouseleave", () => {
                        popup.classList.add("hide");
                        popup.addEventListener('animationend', () => popup.remove(), {once: true});
                    });
                }, 400);
                element.addEventListener("mouseleave", () => {
                    clearTimeout(timeout);
                }, {once: true});
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
