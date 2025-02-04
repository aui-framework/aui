#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2024 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import urllib
from pathlib import Path

from modules import regexes
from modules.config import CONFIG
from bs4 import BeautifulSoup as soup

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

def find_definition(contents):
    for root, dirs, files in os.walk(Path.cwd()):
        for file in files:
            if not file.endswith(".h"):
                continue
            full_path = Path(root) / file
            with open(full_path, 'r') as fis:
                for line_number, line_contents in enumerate(fis.readlines()):
                    if contents in line_contents:
                        return f"{full_path.relative_to(Path.cwd())}#L{line_number+1}"

    raise RuntimeError(f"can't find location of {contents}")



def run():
    for root, dirs, files in os.walk(CONFIG['output']):
        for file in files:
            if not file.endswith(".html"):
                continue

            full_path = Path(root) / file
            raw_contents = full_path.read_bytes().decode('utf-8')
            if "contents-rails-left" in raw_contents:
                continue
            global s
            s = soup(raw_contents, 'lxml')
            headers = s.find_all(["h1", "h2", "h3"])

            contents = s.find(class_="contents")
            if not contents:
                continue

            contents.attrs['class'] = 'contents-rails-left'

            def aui_source_location():
                stem = full_path.stem
                if stem.startswith("md"):
                    possible_location = Path(stem[3:].replace('_01', ' ').replace('_2', '/').replace('_', '/') + ".md")
                    if possible_location.exists():
                        return possible_location

                if stem.startswith("group__"):
                    return find_definition("@defgroup " + stem[7:].replace("__", "_"))



                corresponding_xml = f'{stem}.xml'
                corresponding_xml = Path(CONFIG['xml']) / corresponding_xml
                if not corresponding_xml.exists():
                    return None
                location = None
                with open(corresponding_xml, 'r') as xml:
                    for i in reversed(xml.readlines()):
                        if m := regexes.LOCATION_FILE.match(i):
                            location = f"{m.group(1)}#L{m.group(2)}"
                            break

                return location

            if location := aui_source_location():
                contents.insert(0, s.new_tag('b', **{'aui-src': urllib.parse.quote(str(location), safe='/#', encoding=None, errors=None)}))

            contents = contents.wrap(s.new_tag('div'))
            contents.attrs['class'] = 'contents aui-toc-contents'
            rails_right = s.new_tag('div', attrs={'class': 'contents-rails-right'})
            contents.append(rails_right)

            toc = s.new_tag('div', attrs={'class': 'aui-toc'})

            if not file == "index.html":
                for header in headers:
                    text = header.text.strip('\n')

                    a = header.find_next("a")
                    href = ""
                    if a:
                        a.unwrap()
                        a.append("#")
                        a.attrs['class'] = "aui-toc-hash"
                        a.attrs['style'] = "color: var(--fragment-comment) !important"

                        if "autotoc" in a.attrs.get('id', 'autotoc'):
                            a.attrs['id'] = text.lower().replace(' ', '-') # generate betterid

                        href = a.attrs['href'] = f"#{a.attrs['id']}"

                        header.append(a) # to the end

                    if "Documentation" in text:
                        continue
                    if "memtitle" in header.attrs.get('class', ''): # drops function descriptions
                        continue

                    tag_name = header.name
                    is_doxygen_group_header = "groupheader" in header.attrs.get('class', '')
                    if is_doxygen_group_header:
                        tag_name = "h1"

                    toc.append(tag(tag_name, content=tag('a', content=text, href=href)))

            if s.find("b", attrs={"aui-src": True}):
                toc.append(tag("div", **{"class": "aui-toc-extras"}, content=[
                    tag("a", content=[fa("fa-file"), "View Page Source"], href="javascript:jumpToSource('view')"),
                    tag("a", content=[fa("fa-edit"), "Edit Page Source"], href="javascript:jumpToSource('edit')"),
                ]))

                if summary := s.find("div", **{"class": "summary"}):
                    summary.insert(0, ' | ')
                    summary.insert(0, tag("a", content=["View Page Source"], href="javascript:jumpToSource('view')"))
                    summary.insert(0, tag("a", content=["Edit Page Source"], href="javascript:jumpToSource('edit')"))

            if toc.contents:
                toc.insert(0, tag('p', content='Contents'))
                rails_right.append(toc)

            full_path.write_text(str(s))