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
import logging
import re

import mkdocs_gen_files

from docs.python.generators import cpp_parser, common, group_page, examples_page
from docs.python.generators.cpp_parser import DoxygenEntry, CppClass
from pathlib import Path

log = logging.getLogger('mkdocs')


def _examples_for_symbol_with_snippets(names: list[str], anchors: list[str] | None = None):
    """Search examples index and lists for occurrences of provided names.

    Returns list of dicts with keys: title, id, description, src (Path), snippet, category
    """
    results = []
    try:
        index = examples_page.examples_index
    except Exception:
        index = None

    seen = set()

    def _snippet_matches_anchors(snippet: str, anchors_list: list[str]) -> bool:
        # check for various anchor contexts: word match, Class::member, ->member or .member
        for a in anchors_list:
            if not a:
                continue
            # If anchor is qualified (Class::member), require that exact qualified usage appears.
            if '::' in a:
                if re.search(re.escape(a), snippet):
                    return True
                # also accept usage of the member via member-access or function call patterns when qualified not present
                mem = a.split('::')[-1]
                mem_esc = re.escape(mem)
                if re.search(r"\b" + mem_esc + r"\s*->", snippet) or re.search(r"\b" + mem_esc + r"\s*\.", snippet) or re.search(r"\b" + mem_esc + r"\s*\(", snippet):
                    return True
                continue

            # For unqualified anchors (short names), be stricter: accept only member-access, method call, or slot patterns.
            a_esc = re.escape(a)
            if re.search(r"\b" + a_esc + r"\s*->", snippet) or re.search(r"\b" + a_esc + r"\s*\.", snippet) or re.search(r"\b" + a_esc + r"\s*\(", snippet) or re.search(r"AUI_SLOT\(\s*" + a_esc + r"\s*\)", snippet):
                return True
        return False

    # Special-case: when 'me' is requested as a name/anchor, only return examples
    # that contain the macro-style pseudo-namespace usage `me::` anywhere in the
    # snippet or file. This handles the `#define me this, &...` pattern that the
    # project uses.
    try:
        names_or_anchors = set([n for n in (names or []) if n])
        if anchors:
            for a in anchors:
                if a:
                    names_or_anchors.add(a)
        if 'me' in names_or_anchors:
            seen_local = set()
            # search index first if available
            if index is not None:
                for key, exlist in index.items():
                    for ex in exlist:
                        for src in ex.get('srcs', []):
                            try:
                                text = src.read_text(encoding='utf-8', errors='ignore')
                            except Exception:
                                continue
                            if 'me::' in text:
                                k = (ex.get('id'), str(src))
                                if k in seen_local:
                                    continue
                                seen_local.add(k)
                                # create a small snippet around first occurrence
                                lines = text.splitlines()
                                pos = text.find('me::')
                                cum = 0
                                line_idx = 0
                                for i, l in enumerate(lines):
                                    if pos <= cum + len(l):
                                        line_idx = i
                                        break
                                    cum += len(l) + 1
                                snippet = '\n'.join(lines[max(0, line_idx - 2):min(len(lines), line_idx + 3)])
                                results.append({
                                    'title': ex.get('title'),
                                    'id': ex.get('id'),
                                    'description': ex.get('description'),
                                    'src': Path(src),
                                    'snippet': snippet,
                                })
            # also scan lists
            try:
                lists = examples_page.examples_lists
            except Exception:
                lists = {}
            for category, items in lists.items():
                for ex in items:
                    for src in ex.get('srcs', []):
                        try:
                            text = src.read_text(encoding='utf-8', errors='ignore')
                        except Exception:
                            continue
                        if 'me::' in text:
                            k = (ex.get('id'), str(src))
                            if k in seen_local:
                                continue
                            seen_local.add(k)
                            lines = text.splitlines()
                            pos = text.find('me::')
                            cum = 0
                            line_idx = 0
                            for i, l in enumerate(lines):
                                if pos <= cum + len(l):
                                    line_idx = i
                                    break
                                cum += len(l) + 1
                            snippet = '\n'.join(lines[max(0, line_idx - 2):min(len(lines), line_idx + 3)])
                            results.append({
                                'title': ex.get('title'),
                                'id': ex.get('id'),
                                'description': ex.get('description'),
                                'src': Path(src),
                                'snippet': snippet,
                                'category': category,
                            })
            return results
    except Exception:
        pass

    if index is not None:
        for name in names:
            if not name:
                continue
            for ex in index.get(name, []):
                key = (ex.get('id'))
                if key in seen:
                    continue
                # find the specific src that contains the token
                for src in ex.get('srcs', []):
                    try:
                        text = src.read_text(encoding='utf-8', errors='ignore')
                    except Exception:
                        continue
                    # prefer matches that occur in non-trivial lines (not includes/usings/comments)
                    lines = text.splitlines()
                    found = None
                    for m in re.finditer(r"\b" + re.escape(name) + r"\b", text):
                        pos = m.start()
                        cum = 0
                        line_idx = 0
                        for i, l in enumerate(lines):
                            if pos <= cum + len(l):
                                line_idx = i
                                break
                            cum += len(l) + 1
                        ln = lines[line_idx].strip()
                        # trivial line checks
                        if ln.startswith('#include') or ln.startswith('using ') or ln.startswith('//') or ln.startswith('/*') or ln.startswith('*'):
                            continue
                        if ln in ('{', '}', '#endif', '#if 0') or ln.startswith('#if') or ln.startswith('#define'):
                            continue
                        found = (m, line_idx)
                        break
                    if not found:
                        # no non-trivial match in this file
                        continue
                    m, line_idx = found
                    # expand local window for more reliable local-context checks
                    start = max(0, line_idx - 10)
                    end = min(len(lines), line_idx + 11)
                    local_text = '\n'.join(lines[start:end])
                    snippet = '\n'.join(lines[max(0, line_idx - 2):min(len(lines), line_idx + 3)])
                    # if anchors are provided, ensure snippet relates to anchors or the file contains an instantiation
                    if anchors:
                        # Special-case: if caller asked for the 'me' pseudo-namespace, keep
                        # only examples that actually use the macro-style 'me::' form.
                        try:
                            if any((a or '').strip() == 'me' for a in anchors):
                                if 'me::' not in snippet and 'me::' not in local_text:
                                    continue
                        except Exception:
                            pass
                        # collect potential variable names for instantiations of the anchor classes
                        # but restrict detection to the local window around the match to avoid
                        # accepting unrelated instantiations elsewhere in the file.
                        # derive class names: if anchor is qualified (Class::member) use Class,
                        # but also accept plain class-name anchors like 'AButton'.
                        class_names = []
                        for a in anchors:
                            if not a:
                                continue
                            if '::' in a:
                                class_names.append(a.split('::')[0])
                            else:
                                # treat an unqualified anchor as a class name only if it looks
                                # like a type/class (starts with an uppercase letter). This
                                # avoids treating unqualified method names like 'setValue'
                                # as class names.
                                try:
                                    if re.match(r'[A-Z]', a):
                                        class_names.append(a)
                                except Exception:
                                    pass
                        var_names = set()
                        for cname in class_names:
                            if not cname:
                                continue
                            for m in re.finditer(r"(\w+)\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text):
                                var_names.add(m.group(1))
                            for m in re.finditer(r"_<\s*" + re.escape(cname) + r"\s*>\s*(\w+)", local_text):
                                var_names.add(m.group(1))
                            for m in re.finditer(r"\b" + re.escape(cname) + r"\s+(\w+)\s*(=|;)", local_text):
                                var_names.add(m.group(1))

                        # If snippet directly matches anchors, ensure it's not a generic member-access match
                        if _snippet_matches_anchors(snippet, anchors):
                            # If the qualified anchor is present (Class::member), accept.
                            accepted = False
                            for a in anchors:
                                if not a:
                                    continue
                                if '::' in a and re.search(re.escape(a), snippet):
                                    accepted = True
                                    break
                            if not accepted:
                                # For member-access style matches, require that LHS is a known instantiated var
                                member_tokens = [a.split('::')[-1] for a in anchors if a]
                                lhs_ok = False
                                for mem in member_tokens:
                                    mem_esc = re.escape(mem)
                                    # arrow access
                                    for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    # dot access
                                    for m in re.finditer(r"(\w+)\s*\.\s*" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    # AUI_SLOT(var)::mem pattern
                                    for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                # if LHS wasn't provably a local var, allow match only if the
                                # local window contains an explicit instantiation or the
                                # class name itself (e.g. `_new<Class>` or `Class var;`).
                                if not lhs_ok:
                                    # require local instantiation or explicit mention of the class in the local window
                                    class_inst_ok = False
                                    for cname in class_names:
                                        if not cname:
                                            continue
                                        if re.search(r"_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"\b" + re.escape(cname) + r"\b", local_text):
                                            class_inst_ok = True
                                            break
                                    if not class_inst_ok:
                                        continue
                        else:
                            # Not matching anchors strictly; require explicit evidence in local window
                            class_in_snippet = False
                            for cname in class_names:
                                if not cname:
                                    continue
                                if re.search(r"\b" + re.escape(cname) + r"\b", local_text):
                                    class_in_snippet = True
                                    break

                            var_ref_ok = False
                            if var_names:
                                for v in var_names:
                                    if re.search(r"\b" + re.escape(v) + r"\s*->", local_text) or re.search(r"\b" + re.escape(v) + r"\s*\.", local_text) or re.search(r"AUI_SLOT\(\s*" + re.escape(v) + r"\s*\)", local_text) or re.search(r"AUI_SLOT\(\s*" + re.escape(v) + r"\s*\)::", local_text):
                                        var_ref_ok = True
                                        break

                            if not (class_in_snippet or var_ref_ok):
                                continue
                    results.append({
                        'title': ex['title'],
                        'id': ex['id'],
                        'description': ex['description'],
                        'src': Path(src),
                        'snippet': snippet,
                    })
                    seen.add(key)
                    break
    # fallback: scan lists manually
    try:
        lists = examples_page.examples_lists
    except Exception:
        lists = {}

    for category, items in lists.items():
        for ex in items:
            for src in ex.get('srcs', []):
                try:
                    text = src.read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    continue
                for name in names:
                    if not name:
                        continue
                    # find all matches and prefer non-trivial-line matches
                    lines = text.splitlines()
                    found = None
                    for m in re.finditer(r"\b" + re.escape(name) + r"\b", text):
                        pos = m.start()
                        cum = 0
                        line_idx = 0
                        for i, l in enumerate(lines):
                            if pos <= cum + len(l):
                                line_idx = i
                                break
                            cum += len(l) + 1
                        ln = lines[line_idx].strip()
                        if ln.startswith('#include') or ln.startswith('using ') or ln.startswith('//') or ln.startswith('/*') or ln.startswith('*'):
                            continue
                        if ln in ('{', '}', '#endif', '#if 0') or ln.startswith('#if') or ln.startswith('#define'):
                            continue
                        found = (m, line_idx)
                        break
                    if not found:
                        continue
                    m, line_idx = found
                    key = (ex.get('id'), str(src))
                    if key in seen:
                        continue
                    # expand local window for more reliable local-context checks
                    start = max(0, line_idx - 10)
                    end = min(len(lines), line_idx + 11)
                    local_text = '\n'.join(lines[start:end])
                    snippet = '\n'.join(lines[max(0, line_idx - 2):min(len(lines), line_idx + 3)])
                    # if anchors are provided, ensure snippet relates to anchors or the local window contains an instantiation
                    if anchors:
                        # Special-case: if caller asked for the 'me' pseudo-namespace, keep
                        # only examples that actually use the macro-style 'me::' form.
                        try:
                            if any((a or '').strip() == 'me' for a in anchors):
                                if 'me::' not in snippet and 'me::' not in local_text:
                                    continue
                        except Exception:
                            pass
                        # if snippet matches anchor patterns, still require stronger evidence
                        # for member-access style matches: qualified Class::member is OK, but
                        # simple member-access (it->setValue) must have local LHS/class evidence.
                        if _snippet_matches_anchors(snippet, anchors):
                            # check for explicit qualified occurrence
                            accepted = False
                            for a in anchors:
                                if not a:
                                    continue
                                if '::' in a and re.search(re.escape(a), snippet):
                                    accepted = True
                                    break
                            if not accepted:
                                # collect potential variable names from local_text
                                class_names = [a.split('::')[0] for a in anchors if '::' in (a or '')]
                                var_names = set()
                                for cname in class_names:
                                    if not cname:
                                        continue
                                    for m in re.finditer(r"(\w+)\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text):
                                        var_names.add(m.group(1))
                                    for m in re.finditer(r"_<\s*" + re.escape(cname) + r"\s*>\s*(\w+)", local_text):
                                        var_names.add(m.group(1))
                                    for m in re.finditer(r"\b" + re.escape(cname) + r"\s+(\w+)\s*(=|;)", local_text):
                                        var_names.add(m.group(1))

                                lhs_ok = False
                                for mem in [a.split('::')[-1] for a in anchors if a]:
                                    mem_esc = re.escape(mem)
                                    for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                                        if m.group(1) in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    for m in re.finditer(r"(\w+)\s*\.\s*" + mem_esc, snippet):
                                        if m.group(1) in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                                        if m.group(1) in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break

                                if not lhs_ok:
                                    # require class instantiation or class name in local window
                                    class_inst_ok = False
                                    for cname in class_names:
                                        if not cname:
                                            continue
                                        if re.search(r"_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"\b" + re.escape(cname) + r"\b", local_text):
                                            class_inst_ok = True
                                            break
                                    if not class_inst_ok:
                                        continue
                        else:
                            class_names = [a.split('::')[0] for a in anchors if '::' in (a or '')]
                            # include unqualified anchors only if they look like class/type names
                            for a in anchors:
                                if a and '::' not in a:
                                    try:
                                        if re.match(r'[A-Z]', a):
                                            class_names.append(a)
                                    except Exception:
                                        pass
                            var_names = set()
                            inst_presence = False
                            for cname in class_names:
                                if not cname:
                                    continue
                                for m in re.finditer(r"(\w+)\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text):
                                    var_names.add(m.group(1))
                                for m in re.finditer(r"_<\s*" + re.escape(cname) + r"\s*>\s*(\w+)", local_text):
                                    var_names.add(m.group(1))
                                for m in re.finditer(r"\b" + re.escape(cname) + r"\s+(\w+)\s*(=|;)", local_text):
                                    var_names.add(m.group(1))
                                if re.search(r"_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\b", local_text):
                                    inst_presence = True

                            class_in_snippet = False
                            for cname in class_names:
                                if not cname:
                                    continue
                                if re.search(r"\b" + re.escape(cname) + r"\b", local_text):
                                    class_in_snippet = True
                                    break

                            var_ref_ok = False
                            if var_names:
                                for v in var_names:
                                    if re.search(r"\b" + re.escape(v) + r"\s*->", local_text) or re.search(r"\b" + re.escape(v) + r"\s*\.", local_text) or re.search(r"AUI_SLOT\(\s*" + re.escape(v) + r"\s*\)", local_text) or re.search(r"AUI_SLOT\(\s*" + re.escape(v) + r"\s*\)::", local_text):
                                        var_ref_ok = True
                                        break

                            # require explicit evidence in the local window: either the class name appears
                            # or the snippet references an instantiated variable. Mere presence of an
                            # instantiation elsewhere in the file is not sufficient.
                            if not (class_in_snippet or var_ref_ok):
                                continue
                    results.append({
                        'title': ex['title'],
                        'id': ex['id'],
                        'description': ex['description'],
                        'src': Path(src),
                        'snippet': snippet,
                        'category': category,
                    })
                    seen.add(key)
    return results


def _format_token_sequence(tokens: list[str]):
    output = " ".join(tokens)
    for i in [",", "&&", ">", "&", "*"]:
        output = output.replace(f" {i} ", f"{i} ")
    for i in "()<[]:":
        output = output.replace(f"{i} ", i)
        output = output.replace(f" {i}", i)
    return output


def gen_pages():
    for parse_entry in cpp_parser.index:
        if hasattr(parse_entry, 'doc'):
            if "<!-- aui:no_dedicated_page -->" in parse_entry.doc:
                continue

        if isinstance(parse_entry, DoxygenEntry):
            group_page.handle_doxygen_entry(parse_entry)
            continue

        slugged_name = parse_entry.namespaced_name().lower().replace('::', '_')
        parse_entry.page_url = f'{slugged_name}.md'
        with mkdocs_gen_files.open(parse_entry.page_url, 'w') as fos:
            print(f'# {parse_entry.namespaced_name()}', file=fos)
            print(f'', file=fos)
            doxygen = common.parse_doxygen(parse_entry.doc)
            page_examples: list[dict] = []
            # per-page set to avoid printing same example (id, src) multiple times
            printed_example_pairs: set = set()

            def _dedupe_examples_list(exs: list[dict]) -> list[dict]:
                """Remove duplicate examples that refer to the same example id and same source path.

                Keeps the first occurrence.
                """
                out = []
                seen_pairs = set()
                for ex in exs:
                    if not ex:
                        continue
                    key = (ex.get('id'), str(ex.get('src')))
                    if key in seen_pairs:
                        continue
                    seen_pairs.add(key)
                    out.append(ex)
                return out

            def _filter_examples_by_relevance(exs: list[dict], names: list[str], strict: bool = True) -> list[dict]:
                """Keep only examples whose snippet/title/src appears relevant for any of the provided names.

                Heuristics:
                - If names include a qualified member (contains '::'), prefer snippets that mention the class or show member access (.name or ->name or Class::name).
                - Otherwise keep examples that contain the token as a word, construction patterns (_new<T>, _<T>), or appear in file/title.
                """
                out = []
                # normalize tokens
                tokens = []
                for n in names:
                    if not n:
                        continue
                    if '::' in n:
                        parts = n.split('::')
                        tokens.append(parts[-1])
                        tokens.append(parts[0])
                    else:
                        tokens.append(n)

                is_member_context = any('::' in (n or '') for n in names)

                for ex in exs:
                    snippet = ex.get('snippet', '') or ''
                    title = (ex.get('title') or '').lower()
                    src = str(ex.get('src') or '').lower()
                    matched = False
                    # read full file text for more precise checks when needed
                    full_text = ''
                    try:
                        full_text = Path(ex.get('src')).read_text(encoding='utf-8', errors='ignore')
                    except Exception:
                        full_text = ''

                    # derive a local context window around the snippet to avoid using
                    # file-global evidence when deciding relevance. This reduces false
                    # positives where instantiations exist elsewhere in the file.
                    local_text = full_text
                    try:
                        # find a best-effort match for the snippet inside the full text
                        snippet_plain = snippet.strip()
                        if snippet_plain:
                            idx = full_text.find(snippet_plain)
                            if idx >= 0:
                                # compute line-based window
                                before = full_text[:idx]
                                start_line = before.count('\n')
                                lines = full_text.splitlines()
                                start = max(0, start_line - 10)
                                end = min(len(lines), start_line + snippet_plain.count('\n') + 11)
                                local_text = '\n'.join(lines[start:end])
                    except Exception:
                        local_text = full_text
                    for t in tokens:
                        if not t:
                            continue
                        t_esc = re.escape(t)
                        # helper: detect likely variable declarations like 'auto drawable' or 'Type drawable'
                        var_decl_re = re.compile(r"\b(auto|[A-Za-z_][\w:<>\s\*&]+)\s+" + t_esc + r"\b")

                        # For member-context (method/member names) require call/member-access patterns.
                        if is_member_context:
                            if re.search(r"\b" + t_esc + r"\s*\(", snippet) or re.search(t_esc + r"::", snippet) or re.search(r"\b" + t_esc + r"\s*->", snippet) or re.search(r"\b" + t_esc + r"\s*\.", snippet) or re.search(r"AUI_SLOT\(\s*" + t_esc + r"\s*\)", snippet):
                                matched = True
                        else:
                            # Non-member tokens: accept function-call, construction patterns, qualified usage, or member access.
                            if re.search(r"_new<" + t_esc + r">", snippet) or re.search(r"_<" + t_esc + r">", snippet) or re.search(t_esc + r"::", snippet) or re.search(r"\b" + t_esc + r"\s*->", snippet) or re.search(r"\b" + t_esc + r"\s*\.", snippet) or re.search(r"\b" + t_esc + r"\s*\(", snippet) or re.search(r"AUI_SLOT\(\s*" + t_esc + r"\s*\)", snippet):
                                matched = True
                            else:
                                # plain word match is acceptable only if it's not a simple variable declaration
                                if re.search(r"\b" + t_esc + r"\b", snippet) and not var_decl_re.search(snippet):
                                    matched = True

                        # title/src-based matches still acceptable
                        if t.lower() in title or t.lower() in src:
                            matched = True

                        if matched:
                            break

                    # stricter for member context: require that either class name appears in snippet/src/title or member access patterns exist
                    # when strict is False we accept looser matches (used as a fallback to avoid empty pages)
                    if is_member_context and matched and strict:
                        # check presence of a class token
                        class_present = False
                        for n in names:
                            if not n:
                                continue
                            if '::' in n:
                                class_name = n.split('::')[0]
                                if re.search(r"\b" + re.escape(class_name) + r"\b", snippet) or class_name.lower() in title or class_name.lower() in src:
                                    class_present = True
                                    break
                            else:
                                # only treat unqualified token as a class name when it looks like one
                                try:
                                    if re.match(r'[A-Z]', n) and (re.search(r"\b" + re.escape(n) + r"\b", snippet) or n.lower() in title or n.lower() in src):
                                        class_present = True
                                        break
                                except Exception:
                                    pass
                        if not class_present:
                            # accept only if member access patterns are present (->member or .member or Class::member)
                            member_ok = False
                            for n in names:
                                mem = n.split('::')[-1]
                                mem_esc = re.escape(mem)
                                # find member access occurrences and verify LHS is an instantiated variable of the class
                                # patterns: <lhs>->mem  or <lhs>.mem  or AUI_SLOT(<lhs>)::mem  or Class::mem
                                if re.search(re.escape(n), snippet):
                                    member_ok = True
                                    break
                                m_found = False
                                for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    m_found = True
                                    # verify lhs is instantiated as the class in the local context
                                    if re.search(r"\b" + re.escape(lhs) + r"\b", local_text):
                                        # check instantiation patterns for class tokens inside local_text
                                        for cname in [n.split('::')[0] for n in names if '::' in (n or '')]:
                                            if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                                member_ok = True
                                                break
                                    if member_ok:
                                        break
                                if member_ok:
                                    break
                                for m in re.finditer(r"(\w+)\s*\.\s*" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    m_found = True
                                    if re.search(r"\b" + re.escape(lhs) + r"\b", local_text):
                                        for cname in [n.split('::')[0] for n in names if '::' in (n or '')]:
                                            if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                                member_ok = True
                                                break
                                    if member_ok:
                                        break
                                if member_ok:
                                    break
                                # check AUI_SLOT(var)::mem pattern
                                for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    if re.search(r"\b" + re.escape(lhs) + r"\b", local_text):
                                        for cname in [n.split('::')[0] for n in names if '::' in (n or '')]:
                                            if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                                member_ok = True
                                                break
                                    if member_ok:
                                        break
                                # if snippet had member access but LHS isn't provably our class instance, member_ok stays False
                            if not member_ok:
                                matched = False
                        # if not strict and matched is True, we will keep the example even if member_ok

                    if matched:
                        # Additional safeguard for member-context when using relaxed matching:
                        # require that member-access occurrences (->/. or AUI_SLOT::) either
                        # use a qualified Class::member or have LHS variable names that are
                        # provably instances of the class within the local_text window.
                        if is_member_context and not strict:
                            # derive class names from provided names
                            class_names = []
                            for n in names:
                                if not n:
                                    continue
                                if '::' in n:
                                    class_names.append(n.split('::')[0])
                                else:
                                    try:
                                        if re.match(r'[A-Z]', n):
                                            class_names.append(n)
                                    except Exception:
                                        pass

                            member_tokens = [n.split('::')[-1] for n in names if n]
                            member_ok = False
                            for mem in member_tokens:
                                mem_esc = re.escape(mem)
                                # If Class::mem appears, accept
                                for cname in class_names:
                                    if re.search(re.escape(f"{cname}::{mem}"), snippet):
                                        member_ok = True
                                        break
                                if member_ok:
                                    break

                                # check member-access occurrences and verify LHS instantiation
                                for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    # look for instantiation patterns in local_text linking lhs to class
                                    for cname in class_names:
                                        if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                            member_ok = True
                                            break
                                    if member_ok:
                                        break
                                if member_ok:
                                    break

                                for m in re.finditer(r"(\w+)\s*\.\s*" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    for cname in class_names:
                                        if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                            member_ok = True
                                            break
                                    if member_ok:
                                        break
                                if member_ok:
                                    break

                                for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                                    lhs = m.group(1)
                                    for cname in class_names:
                                        if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                            member_ok = True
                                            break
                                    if member_ok:
                                        break

                            if not member_ok:
                                # reject this example in relaxed mode if we cannot tie member access
                                # to the correct class instance or a qualified usage.
                                matched = False

                        if matched:
                            out.append(ex)
                return out

            def _examples_for_symbol(names: list[str]):
                try:
                    index = examples_page.examples_index
                except Exception:
                    index = None

                merged = []
                seen = set()

                if index is not None:
                    for name in names:
                        if not name:
                            continue
                        for ex in index.get(name, []):
                            key = ex.get('id')
                            if key in seen:
                                continue
                            seen.add(key)
                            merged.append(ex)
                    if merged:
                        return merged

                try:
                    lists = examples_page.examples_lists
                except Exception:
                    return []

                for category, items in lists.items():
                    for ex in items:
                        for src in ex.get('srcs', []):
                            try:
                                text = src.read_text(encoding='utf-8', errors='ignore')
                            except Exception:
                                continue
                            for name in names:
                                if not name:
                                    continue
                                if re.search(r"\b" + re.escape(name) + r"\b", text):
                                    key = (ex['id'], ex['title'])
                                    if key in seen:
                                        break
                                    seen.add(key)
                                    merged.append(ex)
                                    break
                            else:
                                continue
                            break
                return merged


            include_dir = parse_entry.location
            while include_dir.name != 'src':
                if include_dir == include_dir.parent:
                    break
                include_dir = include_dir.parent

            module_name = str(include_dir.parent.name).replace('.', '::')

            for type_entry in [i for i in doxygen if i[0] == '@brief']:
                print(type_entry[1], file=fos)

            # For non-class symbols (macros, free functions, enums), collect examples to print after Detailed Description
            if not isinstance(parse_entry, CppClass):
                try:
                    names_to_search = []
                    if hasattr(parse_entry, 'namespaced_name'):
                        names_to_search.append(parse_entry.namespaced_name())
                    if hasattr(parse_entry, 'name'):
                        names_to_search.append(parse_entry.name)
                    exs = _examples_for_symbol_with_snippets(names_to_search, anchors=None)
                    if exs:
                        page_examples.extend(exs)
                except Exception:
                    pass

            has_detailed_description = bool([i for i in doxygen if i[0] == '@details'])

            print('<table>', file=fos)
            for type_entry in [('Header:', f'<code>#include &lt;{parse_entry.location.relative_to(include_dir)}&gt;</code>'), ('CMake:', f'<code>aui_link(my_target PUBLIC {module_name})</code>')]:
                print(f'<tr><td>{type_entry[0]}</td><td>{type_entry[1]}</td></tr>', file=fos)
            print('</table>', file=fos)

            if hasattr(parse_entry, 'definition'):
                print('## Definition', file=fos)
                print('```cpp', file=fos)
                print(parse_entry.definition, file=fos)
                print('```\n', file=fos)

            if has_detailed_description:
                print('## Detailed Description', file=fos)
                for type_entry in [i for i in doxygen if i[0] == '@details']:
                    print(type_entry[1], file=fos)

            # Print page-level examples immediately after Detailed Description
            if page_examples:
                printed_example_ids = set()
                print('\n## Examples', file=fos)
                for ex in page_examples:
                    exid = ex.get('id')
                    if exid in printed_example_ids:
                        continue
                    printed_example_ids.add(exid)
                    try:
                        src_rel = ex['src'].relative_to(Path.cwd())
                    except Exception:
                        src_rel = ex['src']
                    pair = (ex.get('id'), str(src_rel))
                    if pair in printed_example_pairs:
                        continue
                    printed_example_pairs.add(pair)
                    extension = common.determine_extension(ex['src'])
                    print(f"??? note \"{src_rel}\"", file=fos)
                    print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                    print(f"    ```{extension}", file=fos)
                    for line in ex['snippet'].splitlines():
                        print(f"    {line}", file=fos)
                    print(f"    ```", file=fos)

            # collect class-level examples (don't print here; used as fallback per-method)
            class_examples = []
            try:
                top_names = []
                if hasattr(parse_entry, 'namespaced_name'):
                    top_names.append(parse_entry.namespaced_name())
                if hasattr(parse_entry, 'name'):
                    top_names.append(parse_entry.name)
                class_examples = _examples_for_symbol(top_names) or []
            except Exception:
                class_examples = []
                pass

            # For classes: print class-level examples once (used as fallback reference).
            if isinstance(parse_entry, CppClass) and class_examples:
                try:
                    exs = _examples_for_symbol_with_snippets([parse_entry.namespaced_name(), parse_entry.name], anchors=[parse_entry.namespaced_name(), parse_entry.name, parse_entry.name.split('::')[-1] if '::' in parse_entry.namespaced_name() else None])
                    exs = _dedupe_examples_list(exs)
                    exs = _filter_examples_by_relevance(exs, [parse_entry.namespaced_name(), parse_entry.name], strict=True)
                    # fallback to a relaxed filter if strict yields nothing (avoid empty class pages)
                    if not exs:
                        fallback_list = _dedupe_examples_list(_examples_for_symbol([parse_entry.namespaced_name(), parse_entry.name]))
                        exs = _filter_examples_by_relevance(fallback_list, [parse_entry.namespaced_name(), parse_entry.name], strict=False)
                    if exs:
                        print('\n## Examples', file=fos)
                        for ex in exs:
                            if not ex or 'src' not in ex or not ex.get('snippet'):
                                # skip malformed example entries
                                continue
                            try:
                                src_rel = ex['src'].relative_to(Path.cwd())
                            except Exception:
                                src_rel = ex['src']
                            pair = (ex.get('id'), str(src_rel))
                            if pair in printed_example_pairs:
                                continue
                            printed_example_pairs.add(pair)
                            extension = common.determine_extension(ex['src'])
                            # emit admonition header with no blank line after it
                            print(f"\n??? note \"{src_rel}\"", file=fos)
                            print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                            print(f"    ```{extension}", file=fos)
                            for line in ex['snippet'].splitlines():
                                print(f"    {line}", file=fos)
                            print(f"    ```", file=fos)
                except Exception:
                    pass

            def _render_invisible_header(toc, id, on_other_pages=None):
                # hack: present the header as invisible block. The header will still appear in TOC and
                # can be anchor referenced. Also, the markdown parser in index.py can pick up the heading.
                if not on_other_pages:
                    on_other_pages = toc
                print(f'<div style="position:absolute;opacity:0" markdown>', file=fos)
                print(f'### {toc} {"{"} #{id} {on_other_pages} {"}"} ', file=fos)
                print(f'</div>', file=fos)

            if hasattr(parse_entry, 'types'):
                types = [i for i in parse_entry.types]
                if types:
                    print('', file=fos)
                    print('## Public Types', file=fos)
                    for type_entry in types:
                        full_name = f"{parse_entry.namespaced_name()}::{type_entry.name}"
                        _render_invisible_header(toc=type_entry.name, id=full_name, on_other_pages=full_name)
                        print('', file=fos)
                        print('---', file=fos)
                        print('', file=fos)
                        print(f'`{type_entry.generic_kind} {parse_entry.namespaced_name()}::{type_entry.name}`', file=fos)
                        print(f'', file=fos)

                        doxygen = common.parse_doxygen(type_entry.doc)

                        # todo add a check for @brief to exist
                        for i in [i for i in doxygen if i[0] == '@brief']:
                            print(i[1], file=fos)

                        for i in doxygen:
                            if i[0] in ['', '@details']:
                                print(f'{i[1]}', file=fos)

                        # Examples for nested types
                        names_to_search = [full_name, type_entry.name]
                        exs = _examples_for_symbol_with_snippets(names_to_search, anchors=[full_name, type_entry.name])
                        exs = _dedupe_examples_list(exs)
                        exs = _filter_examples_by_relevance(exs, names_to_search, strict=True)
                        if not exs:
                            fallback_list = _dedupe_examples_list(_examples_for_symbol(names_to_search))
                            exs = _filter_examples_by_relevance(fallback_list, names_to_search, strict=False)
                        if exs:
                            print('\n## Examples', file=fos)
                            for ex in exs:
                                if not ex or 'src' not in ex or not ex.get('snippet'):
                                    continue
                                try:
                                    src_rel = ex['src'].relative_to(Path.cwd())
                                except Exception:
                                    src_rel = ex['src']
                                pair = (ex.get('id'), str(src_rel))
                                if pair in printed_example_pairs:
                                    continue
                                printed_example_pairs.add(pair)
                                extension = common.determine_extension(ex['src'])
                                print(f"\n??? note \"{src_rel}\"", file=fos)
                                print(file=fos)
                                print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                print(file=fos)
                                print(f"    ```{extension}", file=fos)
                                for line in ex['snippet'].splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)

                        # enums - see APath::DefaultPath
                        if hasattr(type_entry, 'enum_values'):
                            print('<table markdown>', file=fos)
                            print('<tr markdown>', file=fos)
                            print('<th>Constant</th>', file=fos)
                            print('<th>Description</th>', file=fos)
                            print('</tr>', file=fos)
                            for v in type_entry.enum_values:
                                print('<tr markdown>', file=fos)
                                print('<td markdown>', file=fos)
                                print(f'`#!cpp {type_entry.name}::{v[0]}`', file=fos)
                                print('</td>', file=fos)
                                print('<td markdown>', file=fos)
                                for i in common.parse_doxygen(v[1]):
                                    print(i[1], file=fos)
                                print('</td>', file=fos)
                                print('</tr>', file=fos)

                            print('</table>', file=fos)

                        # nested struct - see AUpdater::InstallCmdline
                        if hasattr(type_entry, 'fields'):
                            if type_entry.fields:
                                print('<table markdown>', file=fos)
                                print('<tr markdown>', file=fos)
                                print('<th>Field</th>', file=fos)
                                print('<th>Description</th>', file=fos)
                                print('</tr>', file=fos)
                                for v in type_entry.fields:
                                    print('<tr markdown>', file=fos)
                                    print('<td markdown>', file=fos)
                                    print(f'`#!cpp {v.type_str} {v.name}`', file=fos)
                                    print('</td>', file=fos)
                                    print('<td markdown>', file=fos)
                                    for i in common.parse_doxygen(v.doc):
                                        print(i[1], file=fos)
                                    print('</td>', file=fos)
                                    print('</tr>', file=fos)

                                print('</table>', file=fos)
                            else:
                                print('\n\n_Empty structure._', file=fos)


            if hasattr(parse_entry, 'fields'):
                fields = [f for f in parse_entry.fields if f.visibility != 'private' and f.doc is not None]
                if fields:
                    print('## Public fields and Signals', file=fos)
                    for field in sorted(fields, key=lambda x: x.name):
                        print('---', file=fos)
                        full_name = f"{parse_entry.namespaced_name()}::{field.name}"
                        _render_invisible_header(toc=field.name, id=full_name, on_other_pages=full_name)
                        print(f'`#!cpp {field.type_str} {field.name}`\n\n', file=fos)
                        doxygen = common.parse_doxygen(field.doc)
                        for d in doxygen:
                            if d[0] != '@brief':
                                continue
                            print(d[1], file=fos)
                        print('', file=fos)
                        for d in doxygen:
                            if d[0] == '@brief':
                                continue
                            print(d[1], file=fos)
                        print(f'\n', file=fos)

                        # Examples for fields
                        names_to_search = [full_name, field.name]
                        exs = _examples_for_symbol_with_snippets(names_to_search, anchors=[full_name, field.name])
                        exs = _dedupe_examples_list(exs)
                        exs = _filter_examples_by_relevance(exs, names_to_search, strict=True)
                        if not exs:
                            fallback_list = _dedupe_examples_list(_examples_for_symbol(names_to_search))
                            exs = _filter_examples_by_relevance(fallback_list, names_to_search, strict=False)
                        if exs:
                            print('## Examples', file=fos)
                            for ex in exs:
                                if not ex or 'src' not in ex or not ex.get('snippet'):
                                    continue
                                try:
                                    src_rel = ex['src'].relative_to(Path.cwd())
                                except Exception:
                                    src_rel = ex['src']
                                pair = (ex.get('id'), str(src_rel))
                                if pair in printed_example_pairs:
                                    continue
                                printed_example_pairs.add(pair)
                                extension = common.determine_extension(ex['src'])
                                print(f"\n??? note \"{src_rel}\"", file=fos)
                                print(file=fos)
                                print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                print(file=fos)
                                print(f"    ```{extension}", file=fos)
                                for line in ex['snippet'].splitlines():
                                    print(f"    {line}", file=fos)
                                print(f"    ```", file=fos)

            if hasattr(parse_entry, 'methods'):
                methods = [i for i in parse_entry.methods if i.visibility != 'private' and i.doc is not None]
                if methods:
                    print('', file=fos)
                    print('## Public Methods', file=fos)
                    methods_grouped = {}
                    for i in methods:
                        methods_grouped.setdefault(i.name, []).append(i)
                    for name, overloads in sorted(methods_grouped.items(), key=lambda x: x[0] if x[0] != parse_entry.name else '!!!ctor'):
                        full_name = f"{parse_entry.namespaced_name()}::{name}"
                        _render_invisible_header(toc=f"{name}", id=full_name, on_other_pages=f'{full_name}()')
                        for overload in overloads:
                            print('', file=fos)
                            print('---', file=fos)
                            print('', file=fos)
                            print(f'```cpp', file=fos)
                            if overload.template_clause:
                                print(_format_token_sequence([i[1] for i in overload.template_clause]), file=fos)

                            if overload.modifiers_before:
                                print(_format_token_sequence(overload.modifiers_before), end=' ', file=fos)

                            if overload.return_type: # not a constructor
                                print(f'{overload.return_type} ', end='', file=fos)
                            print(f'{parse_entry.name}::{overload.name}{_format_token_sequence([ i[1] for i in overload.args])}', end='', file=fos)
                            print('', file=fos)
                            print(f'```', file=fos)
                            print(f'', file=fos)

                            doxygen = common.parse_doxygen(overload.doc)

                            # todo add a check for @brief to exist
                            for i in [i for i in doxygen if i[0] == '@brief']:
                                print(i[1], file=fos)
                            print('<dl class="doxygen-dl">', file=fos)
                            params = [i for i in doxygen if i[0] == '@param']
                            if params:
                                print('<dt>Arguments</dt>', file=fos)
                                print('<dd><div style="display: table">', file=fos)
                                for i in params:
                                    print(f'<div style="display: table-row">', file=fos)
                                    argument_name = i[1].split(' ')[0]
                                    print(f'<b style="display: table-cell; padding-right: 6px"><code>{argument_name}</code></b>', file=fos)
                                    print(f'<div style="display: table-cell">{i[1][len(argument_name):]}</div>', file=fos)
                                    print(f'</div>', file=fos)
                                print('</div></dd>', file=fos)
                            for i in doxygen:
                                if i[0] != '@return':
                                    continue
                                print('<dt>Returns</dt>', file=fos)
                                print(f'<dd>{i[1]}</dd>', file=fos)
                            print('</dl>', file=fos)

                            for i in doxygen:
                                if i[0] in ['', '@details']:
                                    print(f'{i[1]}', file=fos)

                            # Examples for this overload: show method-specific examples only.
                            try:
                                method_full = f"{parse_entry.namespaced_name()}::{overload.name}"
                                method_names = [method_full, overload.name]
                                method_exs = _examples_for_symbol_with_snippets(method_names, anchors=[method_full, overload.name]) or []
                                method_exs = _dedupe_examples_list(method_exs)
                                method_exs = _filter_examples_by_relevance(method_exs, method_names, strict=True)
                                if not method_exs:
                                    fallback_list = _dedupe_examples_list(_examples_for_symbol(method_names))
                                    # Pre-filter fallback candidates to those that actually contain
                                    # member-access or call patterns for the member token. This
                                    # avoids pulling in unrelated examples that mention the
                                    # token in other contexts.
                                    mem_token = re.escape(overload.name)
                                    member_pattern = re.compile(r"\b" + mem_token + r"\s*\(|\b" + mem_token + r"\s*->|\b" + mem_token + r"\s*\.|AUI_SLOT\(\s*" + mem_token + r"\s*\)")
                                    filtered_fallback = [ex for ex in fallback_list if ex.get('snippet') and member_pattern.search(ex.get('snippet'))]
                                    method_exs = _filter_examples_by_relevance(filtered_fallback, method_names, strict=False)
                                if method_exs:
                                    print('\n## Examples', file=fos)
                                    for ex in method_exs:
                                        if not ex or 'src' not in ex or not ex.get('snippet'):
                                            continue
                                        try:
                                            src_rel = ex['src'].relative_to(Path.cwd())
                                        except Exception:
                                            src_rel = ex['src']
                                        extension = common.determine_extension(ex['src'])
                                        print(f"\n??? note \"{src_rel}\"", file=fos)
                                        print(file=fos)
                                        print(f"    [{ex['title']}]({ex['id']}.md) - {ex.get('description','')}", file=fos)
                                        print(file=fos)
                                        print(f"    ```{extension}", file=fos)
                                        for line in ex['snippet'].splitlines():
                                            print(f"    {line}", file=fos)
                                        print(f"    ```", file=fos)
                            except Exception:
                                pass




    with mkdocs_gen_files.open('classes.md', 'w') as f:
        classes_alphabet = { }
        classes = [i for i in cpp_parser.index if isinstance(i, CppClass)]

        for clazz in classes:
            letter = clazz.name[0]
            if letter == 'A' and clazz.name[1].isupper():
                # most classes in AUI start with 'A', so it makes less sense to chunk by 'A'. Instead, we'll use the
                # second letter.
                letter = clazz.name[1]
            letter = letter.upper()
            classes_alphabet.setdefault(letter, []).append(clazz)
        classes_alphabet = sorted(classes_alphabet.items())
        print('<div class="class-index-title">', file=f)
        for letter, _ in classes_alphabet:
            print(f'<a href="#{letter.lower()}">{letter}</a>', file=f)
        print('</div>', file=f)

        print('<div class="class-index" markdown>', file=f)
        for letter, classes2 in classes_alphabet:
            print('<div class="item odd-highlight" markdown>', file=f)
            print(f'<div class="letter" id="{letter.lower()}">{letter}</div>', file=f)
            print('<div class="list" markdown>', file=f)
            for c in sorted(classes2, key=lambda x: x.name):
                if not c.page_url:
                    continue
                print('<div class="entry" markdown>', file=f)
                print(f'[{c.name}]({c.page_url})', file=f)
                # print(f'{c.name}', file=f)
                print('</div>', file=f)
            print('</div>', file=f)
            print('</div>', file=f)
        print('</div>', file=f)

