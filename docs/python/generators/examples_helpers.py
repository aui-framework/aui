"""Helper functions extracted from the generators.

Contains:
- build_examples_index
- examples_for_symbol
- examples_for_symbol_with_snippets
- dedupe_examples_list
- compute_hl_lines
- filter_examples_by_relevance
- collect_macro_examples_blocks

These functions accept explicit examples_lists/examples_index parameters to avoid import-time coupling.
"""
from pathlib import Path
import re
from typing import List, Dict, Any


def build_examples_index(examples_lists: Dict[str, List[Dict[str, Any]]]) -> Dict[str, List[Dict[str, Any]]]:
    """Build a token -> examples index from examples_lists.

    Skips trivial source files and avoids duplicate entries per token.
    """
    examples_index: Dict[str, List[Dict[str, Any]]] = {}
    for category, items in (examples_lists or {}).items():
        for ex in items:
            seen_tokens = set()
            for src in ex.get('srcs', []):
                try:
                    text = Path(src).read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    continue

                # Skip trivial files that only contain includes/using/guards/comments
                non_blank_lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
                is_trivial = True
                for ln in non_blank_lines:
                    if ln.startswith('//') or ln.startswith('/*') or ln.startswith('*'):
                        continue
                    if ln.startswith('#include'):
                        continue
                    if ln.startswith('using '):
                        continue
                    if ln in ('{', '}', '#endif', '#if 0') or ln.startswith('#if') or ln.startswith('#define'):
                        continue
                    is_trivial = False
                    break

                if is_trivial:
                    continue

                for line in text.splitlines():
                    for tok in re.findall(r"\b[A-Za-z_][A-Za-z0-9_:]*\b", line):
                        if tok in seen_tokens:
                            continue
                        seen_tokens.add(tok)
                        lst = examples_index.setdefault(tok, [])
                        if not any(existing.get('id') == ex.get('id') for existing in lst):
                            lst.append(ex)
    return examples_index


def dedupe_examples_list(exs: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Remove duplicate examples that refer to the same (id, src) pair."""
    out: List[Dict[str, Any]] = []
    seen_pairs = set()
    for ex in (exs or []):
        if not ex:
            continue
        key = (ex.get('id'), str(ex.get('src')))
        if key in seen_pairs:
            continue
        seen_pairs.add(key)
        out.append(ex)
    return out


def compute_hl_lines(snippet: str, tokens: List[str]) -> str | None:
    """Return mkdocs hl_lines string from snippet and tokens, or None."""
    if not snippet or not tokens:
        return None
    lines = snippet.splitlines()
    hits = []
    for idx, line in enumerate(lines, start=1):
        for t in tokens:
            if not t:
                continue
            mem = t.split('::')[-1]
            mem_esc = re.escape(mem)
            if re.search(r"\b" + mem_esc + r"\b", line) or re.search(mem_esc + r"::", line) or re.search(r"\b" + mem_esc + r"\s*\(", line) or re.search(r"\b" + mem_esc + r"\s*->", line) or re.search(r"\b" + mem_esc + r"\s*\.", line) or re.search(r"AUI_SLOT\(\s*" + mem_esc + r"\s*\)", line):
                if re.search(r"\b(auto|[A-Za-z_][\w:<>\t\s\*&]+)\s+" + mem_esc + r"\b", line):
                    if not (re.search(r"\b" + mem_esc + r"\s*\(", line) or re.search(mem_esc + r"::", line) or re.search(r"\b" + mem_esc + r"\s*->", line) or re.search(r"\b" + mem_esc + r"\s*\.", line)):
                        continue
                hits.append(str(idx))
                break
    if not hits:
        return None
    uniq = []
    for h in hits:
        if h not in uniq:
            uniq.append(h)
    return " ".join(uniq)


def filter_examples_by_relevance(exs: List[Dict[str, Any]], names: List[str], strict: bool = True) -> List[Dict[str, Any]]:
    """Filter examples by heuristics for relevance to given names.

    See generator heuristics for details.
    """
    out: List[Dict[str, Any]] = []
    tokens: List[str] = []
    for n in (names or []):
        if not n:
            continue
        if '::' in n:
            parts = n.split('::')
            tokens.append(parts[-1])
            tokens.append(parts[0])
        else:
            tokens.append(n)

    is_member_context = any('::' in (n or '') for n in (names or []))

    for ex in (exs or []):
        snippet = ex.get('snippet', '') or ''
        title = (ex.get('title') or '').lower()
        src = str(ex.get('src') or '').lower()
        matched = False
        full_text = ''
        try:
            full_text = Path(ex.get('src')).read_text(encoding='utf-8', errors='ignore')
        except Exception:
            full_text = ''
        local_text = full_text
        try:
            snippet_plain = snippet.strip()
            if snippet_plain:
                idx = full_text.find(snippet_plain)
                if idx >= 0:
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
            var_decl_re = re.compile(r"\b(auto|[A-Za-z_][\w:<>\s\*&]+)\s+" + t_esc + r"\b")

            if is_member_context:
                if re.search(r"\b" + t_esc + r"\s*\(", snippet) or re.search(t_esc + r"::", snippet) or re.search(r"\b" + t_esc + r"\s*->", snippet) or re.search(r"\b" + t_esc + r"\s*\.", snippet) or re.search(r"AUI_SLOT\(\s*" + t_esc + r"\s*\)", snippet):
                    matched = True
            else:
                if re.search(r"_new<" + t_esc + r">", snippet) or re.search(r"_<" + t_esc + r">", snippet) or re.search(t_esc + r"::", snippet) or re.search(r"\b" + t_esc + r"\s*->", snippet) or re.search(r"\b" + t_esc + r"\s*\.", snippet) or re.search(r"\b" + t_esc + r"\s*\(", snippet) or re.search(r"AUI_SLOT\(\s*" + t_esc + r"\s*\)", snippet):
                    matched = True
                else:
                    if re.search(r"\b" + t_esc + r"\b", snippet) and not var_decl_re.search(snippet):
                        matched = True

            if t.lower() in title or t.lower() in src:
                matched = True

            if matched:
                break

        if is_member_context and matched and strict:
            class_present = False
            for n in (names or []):
                if not n:
                    continue
                if '::' in n:
                    class_name = n.split('::')[0]
                    if re.search(r"\b" + re.escape(class_name) + r"\b", snippet) or class_name.lower() in title or class_name.lower() in src:
                        class_present = True
                        break
                else:
                    try:
                        if re.match(r'[A-Z]', n) and (re.search(r"\b" + re.escape(n) + r"\b", snippet) or n.lower() in title or n.lower() in src):
                            class_present = True
                            break
                    except Exception:
                        pass
            if not class_present:
                member_ok = False
                for n in (names or []):
                    mem = n.split('::')[-1]
                    mem_esc = re.escape(mem)
                    if re.search(re.escape(n), snippet):
                        member_ok = True
                        break
                    for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                        lhs = m.group(1)
                        m_found = True
                        if re.search(r"\b" + re.escape(lhs) + r"\b", local_text):
                            for cname in [nm.split('::')[0] for nm in (names or []) if '::' in (nm or '')]:
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
                            for cname in [nm.split('::')[0] for nm in (names or []) if '::' in (nm or '')]:
                                if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                    member_ok = True
                                    break
                        if member_ok:
                            break
                    if member_ok:
                        break
                    for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                        lhs = m.group(1)
                        if re.search(r"\b" + re.escape(lhs) + r"\b", local_text):
                            for cname in [nm.split('::')[0] for nm in (names or []) if '::' in (nm or '')]:
                                if re.search(r"\b" + re.escape(lhs) + r"\s*=\s*_new<\s*" + re.escape(cname) + r"\b", local_text) or re.search(r"_<\s*" + re.escape(cname) + r"\s*>\s*" + re.escape(lhs), local_text) or re.search(r"\b" + re.escape(cname) + r"\s+" + re.escape(lhs) + r"\s*(=|;)", local_text):
                                    member_ok = True
                                    break
                        if member_ok:
                            break
                if not member_ok:
                    matched = False

        if matched:
            out.append(ex)
    return out


def examples_for_symbol(names: List[str], examples_lists: Dict[str, List[Dict[str, Any]]] | None = None, examples_index: Dict[str, List[Dict[str, Any]]] | None = None) -> List[Dict[str, Any]]:
    """Find examples that mention any of the given names using index or scanning lists."""
    index = examples_index
    merged = []
    seen = set()
    if index is not None:
        for name in (names or []):
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

    lists = examples_lists or {}
    for category, items in lists.items():
        for ex in items:
            for src in ex.get('srcs', []):
                try:
                    text = Path(src).read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    continue
                for name in (names or []):
                    if not name:
                        continue
                    if re.search(r"\b" + re.escape(name) + r"\b", text):
                        key = (ex.get('id'), ex.get('title'))
                        if key in seen:
                            break
                        seen.add(key)
                        merged.append(ex)
                        break
                else:
                    continue
                break
    return merged


def examples_for_symbol_with_snippets(names: List[str], anchors: List[str] | None = None, examples_lists: Dict[str, List[Dict[str, Any]]] | None = None, examples_index: Dict[str, List[Dict[str, Any]]] | None = None) -> List[Dict[str, Any]]:
    """Find examples and include best snippet for each. Uses index when available.

    Returns list of dicts: title,id,description,src(snippet Path),snippet,category
    """
    results: List[Dict[str, Any]] = []
    index = examples_index
    seen = set()
    # make lists available early so index-handling can resolve categories
    try:
        lists = examples_lists or {}
    except Exception:
        lists = {}

    def _snippet_matches_anchors(snippet: str, anchors_list: List[str]) -> bool:
        for a in (anchors_list or []):
            if not a:
                continue
            if '::' in a:
                if re.search(re.escape(a), snippet):
                    return True
                mem = a.split('::')[-1]
                mem_esc = re.escape(mem)
                if re.search(r"\b" + mem_esc + r"\s*->", snippet) or re.search(r"\b" + mem_esc + r"\s*\.", snippet) or re.search(r"\b" + mem_esc + r"\s*\(", snippet):
                    return True
                continue
            a_esc = re.escape(a)
            if re.search(r"\b" + a_esc + r"\s*->", snippet) or re.search(r"\b" + a_esc + r"\s*\.", snippet) or re.search(r"\b" + a_esc + r"\s*\(", snippet) or re.search(r"AUI_SLOT\(\s*" + a_esc + r"\s*\)", snippet):
                return True
        return False

    # special-case 'me'
    try:
        names_or_anchors = set([n for n in (names or []) if n])
        if anchors:
            for a in anchors:
                if a:
                    names_or_anchors.add(a)
        if 'me' in names_or_anchors:
            seen_local = set()
            if index is not None:
                for key, exlist in index.items():
                    for ex in exlist:
                        for src in ex.get('srcs', []):
                            try:
                                text = Path(src).read_text(encoding='utf-8', errors='ignore')
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
                                snippet = '\n'.join(lines[max(0, line_idx - 4):min(len(lines), line_idx + 6)])
                                results.append({
                                    'title': ex.get('title'),
                                    'id': ex.get('id'),
                                    'description': ex.get('description'),
                                    'src': Path(src),
                                    'snippet': snippet,
                                })
            try:
                lists = examples_lists or {}
            except Exception:
                lists = {}
            for category, items in lists.items():
                for ex in items:
                    for src in ex.get('srcs', []):
                        try:
                            text = Path(src).read_text(encoding='utf-8', errors='ignore')
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
                                snippet = '\n'.join(lines[max(0, line_idx - 4):min(len(lines), line_idx + 6)])
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
        for name in (names or []):
            if not name:
                continue
            for ex in index.get(name, []):
                key = (ex.get('id'))
                if key in seen:
                    continue
                for src in ex.get('srcs', []):
                    try:
                        text = Path(src).read_text(encoding='utf-8', errors='ignore')
                    except Exception:
                        continue
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
                    start = max(0, line_idx - 10)
                    end = min(len(lines), line_idx + 11)
                    local_text = '\n'.join(lines[start:end])
                    snippet = '\n'.join(lines[max(0, line_idx - 4):min(len(lines), line_idx + 6)])
                    if anchors:
                        try:
                            if any((a or '').strip() == 'me' for a in anchors):
                                if 'me::' not in snippet and 'me::' not in local_text:
                                    continue
                        except Exception:
                            pass
                        class_names = []
                        for a in (anchors or []):
                            if not a:
                                continue
                            if '::' in a:
                                class_names.append(a.split('::')[0])
                            else:
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

                        if _snippet_matches_anchors(snippet, anchors):
                            accepted = False
                            for a in (anchors or []):
                                if not a:
                                    continue
                                if '::' in a and re.search(re.escape(a), snippet):
                                    accepted = True
                                    break
                            if not accepted:
                                member_tokens = [a.split('::')[-1] for a in (anchors or []) if a]
                                lhs_ok = False
                                for mem in member_tokens:
                                    mem_esc = re.escape(mem)
                                    for m in re.finditer(r"(\w+)\s*->\s*" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    for m in re.finditer(r"(\w+)\s*\.\s*" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                    for m in re.finditer(r"AUI_SLOT\(\s*(\w+)\s*\)::" + mem_esc, snippet):
                                        lhs = m.group(1)
                                        if lhs in var_names:
                                            lhs_ok = True
                                            break
                                    if lhs_ok:
                                        break
                                if not lhs_ok:
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
                    # try to determine category for this example
                    category = ex.get('category', None)
                    if not category:
                        for cat, items in (lists or {}).items():
                            for item in items:
                                if item.get('id') == ex.get('id'):
                                    category = cat
                                    break
                            if category:
                                break
                    results.append({
                        'title': ex['title'],
                        'id': ex['id'],
                        'description': ex['description'],
                        'src': Path(src),
                        'snippet': snippet,
                        'category': category,
                    })
                    seen.add(key)
                    break
    try:
        lists = examples_lists or {}
    except Exception:
        lists = {}
    for category, items in lists.items():
        for ex in items:
            for src in ex.get('srcs', []):
                try:
                    text = Path(src).read_text(encoding='utf-8', errors='ignore')
                except Exception:
                    continue
                for name in (names or []):
                    if not name:
                        continue
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
                    start = max(0, line_idx - 10)
                    end = min(len(lines), line_idx + 11)
                    local_text = '\n'.join(lines[start:end])
                    snippet = '\n'.join(lines[max(0, line_idx - 4):min(len(lines), line_idx + 6)])
                    if anchors:
                        try:
                            if any((a or '').strip() == 'me' for a in anchors):
                                if 'me::' not in snippet and 'me::' not in local_text:
                                    continue
                        except Exception:
                            pass
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


def collect_macro_examples_blocks(examples_lists: Dict[str, List[Dict[str, Any]]] | None = None) -> List[str]:
    """Collect examples for all macros and return list of rendered note blocks."""
    examples_blocks = []
    lists = examples_lists or {}
    macros = []
    # attempt to find macros index if available
    try:
        from docs.python.generators import cpp_parser
        from docs.python.generators.cpp_parser import CppMacro
        macros = sorted([i for i in cpp_parser.index if isinstance(i, CppMacro)], key=lambda x: x.name)
    except Exception:
        macros = []

    for i in macros:
        name = i.name
        pat = re.compile(r"\b" + re.escape(name) + r"\b")
        for cat, examples in lists.items():
            for ex in examples:
                for src in ex.get('srcs', []):
                    try:
                        text = Path(src).read_text(encoding='utf-8', errors='ignore')
                    except Exception:
                        continue
                    if not pat.search(text):
                        continue
                    m = pat.search(text)
                    lines = text.splitlines()
                    pos = m.start()
                    cum = 0
                    line_idx = 0
                    for idx, l in enumerate(lines):
                        if pos <= cum + len(l):
                            line_idx = idx
                            break
                        cum += len(l) + 1
                    start = max(0, line_idx - 4)
                    end = min(len(lines), line_idx + 6)
                    snippet = '\n'.join(lines[start:end])
                    try:
                        src_rel = Path(src).relative_to(Path.cwd())
                    except Exception:
                        src_rel = Path(src)
                    extension = 'cpp'
                    block = ''
                    block += f"??? note \"{src_rel}\"\n"
                    block += f"    [{ex['title']}]({ex['id']}.md) — {ex.get('description','')}\n"
                    block += f"    ```{extension}\n"
                    for line in snippet.splitlines():
                        block += f"    {line}\n"
                    block += f"    ```\n"
                    examples_blocks.append(block)
    return examples_blocks
