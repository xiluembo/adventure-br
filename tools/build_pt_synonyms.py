#!/usr/bin/env python3
"""Generate Portuguese synonym table for Adventure parser."""

from __future__ import annotations

import pathlib
from dataclasses import dataclass
from typing import Dict, List

from pt_synonyms_data import MAPPING

ROOT = pathlib.Path(__file__).resolve().parents[1]
ADVWORD = ROOT / "src" / "advword.h"
OUTPUT = ROOT / "src" / "synonyms_pt.inc"

@dataclass(frozen=True)
class Entry:
    aword: str
    code: int


def load_vocabulary() -> Dict[str, Entry]:
    entries: Dict[str, Entry] = {}
    for line in ADVWORD.read_text().splitlines():
        line = line.strip()
        if not line.startswith('{'):
            continue
        parts = line.strip('{} ,').split(',')
        if len(parts) < 2:
            continue
        word = parts[0].strip().strip('"')
        code = int(parts[1])
        if word in entries:
            continue
        entries[word] = Entry(word, code)
    return entries


def validate(entries: Dict[str, Entry], mapping: Dict[str, List[str]]) -> None:
    missing = sorted(set(entries) - set(mapping))
    if missing:
        raise SystemExit(f"Missing Portuguese synonym entries for: {', '.join(missing)}")
    seen_terms: Dict[str, str] = {}
    for aword, terms in mapping.items():
        if not terms:
            raise SystemExit(f"No Portuguese terms defined for {aword}")
        for term in terms:
            upper = term.upper()
            if len(upper) >= 20:
                raise SystemExit(f"Portuguese term '{term}' for {aword} exceeds WORDSIZE")
            if upper in seen_terms:
                other = seen_terms[upper]
                raise SystemExit(
                    f"Portuguese term '{term}' for {aword} duplicates entry for {other}"
                )
            seen_terms[upper] = aword
    verbs = {w for w, e in entries.items() if e.code >= 2000 and e.code < 3000}
    for verb in verbs:
        terms = mapping[verb]
        if len(terms) < 2:
            raise SystemExit(f"Verb {verb} must have infinitive and imperative forms")


def render(mapping: Dict[str, List[str]]) -> str:
    rows = []
    for aword, terms in sorted(mapping.items()):
        for term in terms:
            rows.append(f"    {{ \"{term.upper()}\", \"{aword}\" }},")
    rows.sort(key=lambda line: line.split('"')[1])
    return "\n".join(rows) + "\n"


def main() -> None:
    entries = load_vocabulary()
    mapping: Dict[str, List[str]] = MAPPING

    validate(entries, mapping)
    content = "/* Auto-generated Portuguese synonym list. */\n" + render(mapping)
    OUTPUT.write_text(content)


if __name__ == "__main__":
    main()
