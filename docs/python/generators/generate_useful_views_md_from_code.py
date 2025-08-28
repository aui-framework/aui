
# Simple copy: make useful_views.md match useful_views_new.md style exactly
from pathlib import Path
src = Path(__file__).parent.parent / 'useful_views_new.md'
dst = Path(__file__).parent.parent / 'useful_views.md'
if src.exists():
    dst.write_text(src.read_text(encoding='utf-8'), encoding='utf-8')