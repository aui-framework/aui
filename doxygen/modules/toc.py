#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

from json import loads
from pathlib import Path
from re import compile, escape
from concurrent.futures import ProcessPoolExecutor

from lxml import html, etree

from modules.config import CONFIG


def tag(name, content=None, **kwargs):
    global s
    tag = s.new_tag(name, **kwargs)
    if type(content) is list:
        for i in content:
            tag.append(i)
    elif content is None:
        pass
    else:
        tag.append(content)
    return tag


def fa(name):
    return tag("i", **{"class": f"fa {name}"})


def find_definition(contents: str, search_dirs=None):
    """
    Locate @defgroup or literal definition in source.
    """
    repo_root = Path.cwd()
    inter = repo_root / "doxygen" / "intermediate"
    dox_path = inter / "groups.dox"
    json_path = inter / "groups.json"

    if contents.startswith("@defgroup "):
        gid = contents.split(None, 1)[1].strip()
        if not json_path.exists():
            raise RuntimeError(
                f"{json_path} is missing! Please regenerate your groups JSON"
            )
        group_map = loads(json_path.read_text(encoding="utf-8"))
        if gid in group_map:
            return f"{dox_path.relative_to(repo_root).as_posix()}#L{group_map[gid]}"
        else:
            print(f"  ↪ '{gid}' not in JSON map, falling back to full scan")

    # 2) Fallback: brute‐force scan .h/.hpp/.dox
    pattern = compile(r"(?<!\w)" + escape(contents) + r"(?!\w)")
    search_dirs = search_dirs or [repo_root, repo_root / "doxygen" / "intermediate"]

    for base in search_dirs:
        if not base.exists():
            continue

        for ext in ("*.h", "*.hpp", "*.dox"):
            for path in base.rglob(ext):
                try:
                    lines = path.read_text(encoding="utf-8").splitlines()
                except UnicodeDecodeError:
                    continue

                for idx, line in enumerate(lines, start=1):
                    if pattern.search(line):
                        rel = path.relative_to(repo_root).as_posix()
                        return f"{rel}#L{idx}"

    raise RuntimeError(f"Can't find location of {contents}")


def process_file(full_path: Path):
    parser = html.HTMLParser(encoding="utf-8", remove_comments=True)
    tree = html.parse(str(full_path), parser)
    root = tree.getroot()

    # 1) Grab the original "contents" div
    contents = root.xpath(
        ".//div[contains(concat(' ', normalize-space(@class), ' '), ' contents ')]"
    )
    if not contents:
        return
    contents = contents[0]

    # skip if already patched
    if "contents-rails-left" in contents.get("class", ""):
        return

    # 2) Find the main content container
    doc_content = root.xpath(".//div[@id='doc-content']")
    if not doc_content:
        doc_content = root.xpath(".//div[@id='container']")
    if not doc_content:
        return
    doc_content = doc_content[0]

    # 3) Prepare the right rail container
    rails_right = etree.Element(
        "div",
        {
            "class": "contents contents-rails-right",
            "style": "position: fixed; right: 20px; top: 100px; width: 300px; z-index: 1000;",
        },
    )

    # 4) Create the TOC container
    toc = etree.SubElement(rails_right, "div", {"class": "aui-toc"})

    # 5) Add "Contents" header to TOC
    toc_header = etree.SubElement(toc, "p")
    toc_header.text = "Contents"

    toc_entries = 0

    # 6) Build TOC from headers
    for h in root.xpath("//h1|//h2|//h3"):
        text = "".join(h.xpath(".//text()")).strip()
        if not text or "Documentation" in text:
            continue
        if "memtitle" in h.get("class", ""):
            continue

        # find anchor inside the header
        a = h.find(".//a")
        if a is None or a.getparent() is not h:
            continue

        # normalize the ID
        if "autotoc" in a.get("id", ""):
            new_id = text.lower().replace(" ", "-")
            a.set("id", new_id)

        href = "#" + a.get("id")
        a.text = "#"
        a.set("class", "aui-toc-hash")
        a.set("style", "color: var(--fragment-comment) !important")
        h.append(a)

        entry = etree.SubElement(toc, h.tag)
        a2 = etree.SubElement(entry, "a", href=href)
        a2.text = text
        toc_entries += 1

    # 7) Add source buttons if page has aui-src attribute
    has_aui_src = root.xpath("//*[@aui-src]")
    if has_aui_src:
        # Create extras container
        extras_div = etree.SubElement(toc, "div", {"class": "aui-toc-extras"})

        # View Page Source button
        view_link = etree.SubElement(
            extras_div,
            "a",
            {
                "href": "javascript:jumpToSource('view')",
                "style": "display: block; margin-bottom: 8px;",
            },
        )
        view_icon = etree.SubElement(view_link, "i", {"class": "fa fa-file"})
        view_icon.text = " "  # Add space before text
        view_link_text = etree.Element("span")
        view_link_text.text = "View Page Source"
        view_link.append(view_link_text)

        # Edit Page Source button
        edit_link = etree.SubElement(
            extras_div,
            "a",
            {"href": "javascript:jumpToSource('edit')", "style": "display: block;"},
        )
        edit_icon = etree.SubElement(edit_link, "i", {"class": "fa fa-edit"})
        edit_icon.text = " "  # Add space before text
        edit_link_text = etree.Element("span")
        edit_link_text.text = "Edit Page Source"
        edit_link.append(edit_link_text)

        # Add to summary section if exists
        summary_divs = root.xpath('//div[@class="summary"]')
        if summary_divs:
            summary_div = summary_divs[0]

            # Add separator
            separator = etree.Element("span", text=" | ")
            summary_div.insert(0, separator)

            # Add Edit link
            edit_summary = etree.SubElement(
                summary_div, "a", {"href": "javascript:jumpToSource('edit')"}
            )
            edit_summary.text = "Edit Page Source"

            # Add separator
            separator = etree.Element("span", text=" | ")
            summary_div.insert(0, separator)

            # Add View link
            view_summary = etree.SubElement(
                summary_div, "a", {"href": "javascript:jumpToSource('view')"}
            )
            view_summary.text = "View Page Source"

        script = etree.Element("script")
        script.text = """
        function jumpToSource(action) {
            var auiSrc = document.querySelector('[aui-src]').getAttribute('aui-src');
            if (auiSrc) {
                var url;
                if (action === 'view') {
                    url = 'https://github.com/aui-framework/aui/blob/develop/' + auiSrc;
                } else if (action === 'edit') {
                    url = 'https://github.com/aui-framework/aui/edit/develop/' + auiSrc;
                }
                if (url) {
                    window.open(url, '_blank');
                }
            }
        }
        """
        body = root.find("body")
        if body is not None:
            body.append(script)

    # 8) Only add right rail if we have content
    if toc_entries > 0 or has_aui_src:
        # Add the right rail to the body
        body = root.find("body")
        if body is not None:
            body.append(rails_right)

    tree.write(str(full_path), encoding="utf-8", method="html", pretty_print=False)


def run():
    root = Path(CONFIG["output"])
    html_files = list(root.rglob("*.html"))
    with ProcessPoolExecutor() as pool:
        pool.map(process_file, html_files)
