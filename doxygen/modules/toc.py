#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import urllib.parse
from json import loads
from pathlib import Path
from re import compile, escape, sub

from bs4 import BeautifulSoup as soup
from modules import regexes
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


_group_map = None  # module‐level cache


def find_definition(contents: str, search_dirs=None):
    repo_root = Path.cwd()
    inter = repo_root / "doxygen" / "intermediate"
    dox_path = inter / "groups.dox"
    json_path = inter / "groups.json"

    # 1) Try JSON lookup for @defgroup
    if contents.startswith("@defgroup "):
        gid = contents.split(None, 1)[1].strip()

        global _group_map
        if _group_map is None:
            if not json_path.exists():
                raise RuntimeError(f"{json_path} missing—regenerate your groups JSON")
            _group_map = loads(json_path.read_text(encoding="utf-8"))

        if gid in _group_map:
            return f"{dox_path.relative_to(repo_root).as_posix()}#L{_group_map[gid]}"
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
                        print(f"found in {rel}#L{idx}")
                        return f"{rel}#L{idx}"

    raise RuntimeError(f"Can't find location of {contents}")


def run():
    for root, dirs, files in os.walk(CONFIG["output"]):
        for file in files:
            if not file.endswith(".html"):
                continue

            full_path = Path(root) / file
            raw_contents = full_path.read_bytes().decode("utf-8")
            if "contents-rails-left" in raw_contents:
                continue
            global s
            s = soup(raw_contents, "lxml")
            headers = s.find_all(["h1", "h2", "h3"])

            contents = s.find(class_="contents")
            if not contents:
                continue

            contents.attrs["class"] = "contents-rails-left"

            def aui_source_location():
                stem = full_path.stem
                if not (stem.startswith("UI") and stem.endswith("Test")):
                    return None

                if stem.startswith("md"):
                    possible_location = Path(
                        stem[3:]
                        .replace("_01", " ")
                        .replace("_2", "/")
                        .replace("_", "/")
                        + ".md"
                    )
                    if possible_location.exists():
                        return possible_location

                if stem.startswith("group__"):
                    core = stem.removeprefix("UI").removesuffix("Test")
                    # CamelCase → snake_case
                    gid = sub(r"([a-z0-9])([A-Z])", r"\1_\2", core).lower()
                    return find_definition(f"@defgroup {gid}")
                    # return find_definition("@defgroup " + stem[7:].replace("__", "_"))

                corresponding_xml = f"{stem}.xml"
                corresponding_xml = Path(CONFIG["xml"]) / corresponding_xml
                if not corresponding_xml.exists():
                    return None
                location = None
                with open(corresponding_xml, "r") as xml:
                    for i in reversed(xml.readlines()):
                        if m := regexes.LOCATION_FILE.match(i):
                            location = f"{m.group(1)}#L{m.group(2)}"
                            break

                return location

            if location := aui_source_location():
                contents.insert(
                    0,
                    s.new_tag(
                        "b",
                        **{
                            "aui-src": urllib.parse.quote(
                                str(location), safe="/#", encoding=None, errors=None
                            )
                        },
                    ),
                )

            contents = contents.wrap(s.new_tag("div"))
            contents.attrs["class"] = "contents aui-toc-contents"
            rails_right = s.new_tag("div", attrs={"class": "contents-rails-right"})
            contents.append(rails_right)

            toc = s.new_tag("div", attrs={"class": "aui-toc"})

            if not file == "index.html":
                for header in headers:
                    text = header.text.strip("\n")

                    a = header.find_next("a")
                    href = ""
                    if a:
                        if a.parent.name not in ["h1", "h2", "h3"]:
                            continue

                        a.unwrap()
                        a.append("#")
                        a.attrs["class"] = "aui-toc-hash"
                        a.attrs["style"] = "color: var(--fragment-comment) !important"

                        if "autotoc" in a.attrs.get("id", "autotoc"):
                            a.attrs["id"] = text.lower().replace(
                                " ", "-"
                            )  # generate betterid

                        href = a.attrs["href"] = f"#{a.attrs['id']}"

                        header.append(a)  # to the end

                    if "Documentation" in text:
                        continue
                    if "memtitle" in header.attrs.get(
                        "class", ""
                    ):  # drops function descriptions
                        continue

                    tag_name = header.name
                    is_doxygen_group_header = "groupheader" in header.attrs.get(
                        "class", ""
                    )
                    if is_doxygen_group_header:
                        tag_name = "h1"

                    toc.append(tag(tag_name, content=tag("a", content=text, href=href)))

            if s.find("b", attrs={"aui-src": True}):
                toc.append(
                    tag(
                        "div",
                        **{"class": "aui-toc-extras"},
                        content=[
                            tag(
                                "a",
                                content=[fa("fa-file"), "View Page Source"],
                                href="javascript:jumpToSource('view')",
                            ),
                            tag(
                                "a",
                                content=[fa("fa-edit"), "Edit Page Source"],
                                href="javascript:jumpToSource('edit')",
                            ),
                        ],
                    )
                )

                if summary := s.find("div", **{"class": "summary"}):
                    summary.insert(0, " | ")
                    summary.insert(
                        0,
                        tag(
                            "a",
                            content=["Edit Page Source"],
                            href="javascript:jumpToSource('edit')",
                        ),
                    )
                    summary.insert(0, " | ")
                    summary.insert(
                        0,
                        tag(
                            "a",
                            content=["View Page Source"],
                            href="javascript:jumpToSource('view')",
                        ),
                    )

            if toc.contents:
                toc.insert(0, tag("p", content="Contents"))
                rails_right.append(toc)

            full_path.write_text(str(s))
