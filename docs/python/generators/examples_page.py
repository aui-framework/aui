#  AUI Framework - Declarative UI toolkit for modern C++20
#  Copyright (C) 2020-2025 Alex2772 and Contributors
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#  SPDX-License-Identifier: MPL-2.0
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
import io
import os
import re
from pathlib import Path

from docs.python.generators import regexes, common


def examine():
    EXAMPLES_DIR = Path.cwd() / "examples"
    EXAMPLES_DIR = EXAMPLES_DIR.as_posix()

    examples_lists = {}

    for root, dirs, files in os.walk(EXAMPLES_DIR):
        for file in files:
            if file != "README.md":
                continue

            example_path = str(Path(root).relative_to(EXAMPLES_DIR))
            if "/" not in example_path and "\\" not in example_path:
                continue

            example_path = example_path.replace(os.sep, "_")

            def collect_srcs(top):
                for root, dirs, files in os.walk(top):
                    for f in files:
                        if any(f.endswith(ext) for ext in ['.h', 'cpp', 'CMakeLists.txt']):
                            yield Path(root) / f

            srcs = list(collect_srcs(root))
            # deduplicate source file list while preserving order
            seen_srcs = set()
            deduped_srcs = []
            for p in srcs:
                if p in seen_srcs:
                    continue
                seen_srcs.add(p)
                deduped_srcs.append(p)
            srcs = deduped_srcs

            # Filter out trivial source files (only includes/using/guards).
            def _is_trivial_source(p: Path) -> bool:
                try:
                    text = p.read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    return True
                non_blank_lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
                if not non_blank_lines:
                    return True
                for ln in non_blank_lines:
                    if ln.startswith('//') or ln.startswith('/*') or ln.startswith('*'):
                        continue
                    if ln.startswith('#include'):
                        continue
                    if ln.startswith('using '):
                        continue
                    if ln in ('{', '}', '#endif', '#if 0') or ln.startswith('#if') or ln.startswith('#define'):
                        continue
                    # found a non-trivial line
                    return False
                return True

            srcs = [p for p in srcs if not _is_trivial_source(p)]


            input_file = Path(root) / file
            with open(input_file, 'r', encoding='utf-8') as fis:
                title = fis.readline()
                assert title.startswith("# ")
                title = title.lstrip("# ").rstrip("\n")
                if "{" in title:
                    id = title[title.find("{#")+2:title.find("}")]
                    title = title[:title.find("{#")]
                else:
                    id = title.lower()
                    for i in [" ", "_"]:
                        id = id.replace(i, "-")
                    id = "".join(ch for ch in id if ch.isalnum() or ch == '-')
                    id = id.strip('-')

                category = None
                description = ""
                it = iter(fis.readlines())
                for line in it:
                    if m := regexes.AUI_EXAMPLE.match(line):
                        category = m.group(1)
                        for description_line in it:
                            description_line = description_line.strip("\n")
                            if not description_line:
                                break
                            description += " " + description_line

                description = description.strip()
                if not id:
                    raise RuntimeError(f"no id provided in {input_file}")
                if not category:
                    raise RuntimeError(f"no category provided in {input_file}")
                if not description:
                    raise RuntimeError(f"no description provided in {input_file}")

                examples_lists.setdefault(category, []).append({
                    'id': id,
                    'title': title,
                    'description': description,
                    'page_path': input_file,
                    'srcs': srcs,
                })

    return examples_lists


examples_lists = examine()
if not examples_lists:
    raise RuntimeError("no examples provided")

examples_index: dict[str, list[dict]] = {}
for category, items in examples_lists.items():
    for ex in items:
        seen_tokens = set()
        for src in ex.get('srcs', []):
            try:
                text = src.read_text(encoding='utf-8', errors='ignore')
            except Exception:
                continue

            # Quick heuristic: skip trivial files that only contain includes,
            # using-directives, braces, or preprocessor guards. This avoids
            # treating examples that are only a header wrapper as usage.
            non_blank_lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
            is_trivial = True
            for ln in non_blank_lines:
                # skip comment lines
                if ln.startswith('//') or ln.startswith('/*') or ln.startswith('*'):
                    continue
                # includes
                if ln.startswith('#include'):
                    continue
                # using namespace or using std::
                if ln.startswith('using '):
                    continue
                # simple braces or preprocessor guards
                if ln in ('{', '}', '#endif', '#if 0') or ln.startswith('#if') or ln.startswith('#define'):
                    continue
                # if we reach here, the file contains non-trivial content
                is_trivial = False
                break

            if is_trivial:
                continue

            # iterate by lines and collect tokens; avoid tokens coming only
            # from the same file multiple times by using seen_tokens
            for line in text.splitlines():
                for tok in re.findall(r"\b[A-Za-z_][A-Za-z0-9_:]*\b", line):
                    if tok in seen_tokens:
                        continue
                    seen_tokens.add(tok)
                    lst = examples_index.setdefault(tok, [])
                    # avoid duplicates: append example only once per token
                    if not any(existing.get('id') == ex.get('id') for existing in lst):
                        lst.append(ex)

def define_env(env):
    @env.macro
    def examples(category: str):
        output  = "| Name | Description |\n"
        output += "| ---- | ----------- |\n"

        for example in examples_lists[category]:
            title = example['title']
            id = example['id']
            description = example['description']
            srcs = example['srcs']
            output += f"| [{title}]({id}.md) | <!-- aui:example-file-count {len(srcs)} --> {description} |\n"

        return output

def example(category: str):
    return f"""
!!! example "Example's page"
    
    This page describes an example listed in [{category}](examples.md#{category}) category.

"""

def gen_pages():
    import mkdocs_gen_files
    for category_name, category in examples_lists.items():
        for example in category:
            id = example['id']
            page_path = example['page_path']
            mkdocs_gen_files.set_edit_path(f"{id}.md", '..' / page_path.relative_to(Path.cwd()))
            with mkdocs_gen_files.open(f"{id}.md", "w") as fos:
                with io.open(page_path, 'r', encoding='utf-8') as fis:
                    contents = fis.read()
                    print(contents, file=fos, end='')

                    if not example['srcs']:
                        if "## Source Code" not in contents:
                            raise RuntimeError(f'{page_path} contains neither "## Source Code" section nor source files.')
                        continue
                    print('\n## Source Code\n\n', file=fos)
                    print(f'[ <!-- aui:icon octicons-link-external-16 --> Repository ](https://github.com/aui-framework/aui/tree/master/{page_path.relative_to(Path.cwd())})\n', file=fos)
                    for f in example['srcs']:
                        filename = f.relative_to(page_path.parent)
                        print(f'\n### {filename}\n', file=fos)
                        extension = common.determine_extension(f)

                        print(f'```{extension} linenums="1"', file=fos)
                        def skip_license(iterator):
                            if "/*" in next(iterator):
                                for line in iterator:
                                    if "*/" in line:
                                        break

                            for line in iterator:
                                yield line

                        for line in skip_license(iter(f.read_text().splitlines())):
                            print(f'{line}', file=fos)
                        print(f'```', file=fos)

