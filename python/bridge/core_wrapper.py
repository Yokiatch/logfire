from __future__ import annotations
import json
import sys
from pathlib import Path

# Resolve build dir relative to this file's location
_root     = Path(__file__).resolve().parent.parent.parent
_build    = _root / "build"

if str(_build) not in sys.path:
    sys.path.insert(0, str(_build))

try:
    import _logfire as _core
except ImportError as e:
    raise ImportError(
        f"Native extension not found in {_build}\n"
        f"Run: cmake --build build --parallel"
    ) from e


def query(
    path: str | Path,
    *,
    pattern: str = "",
    field_filter: str = "",
    limit: int = 0,
    offset: int = 0,
) -> list[str]:
    raw = _core.query_file(
        str(path),
        pattern,
        field_filter,
        limit,
        offset,
    )
    return json.loads(raw)