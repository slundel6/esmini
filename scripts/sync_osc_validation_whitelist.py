#!/usr/bin/env python3
"""
Sync the osc-validation whitelist.txt with the tests currently discovered by
pytest's --collect-only.

For every test currently discovered by pytest:
  - if it already has an entry in whitelist.txt, that line and its keyword
    (INCLUDE/EXCLUDE) are left untouched
  - if it is new, it is appended to whitelist.txt with the EXCLUDE keyword

Tests listed in whitelist.txt that are no longer discovered by pytest are not
removed automatically, they are only reported as a warning.

Requirements:
  - --cwd must point at an existing local checkout of PMSFIT/osc-validation
    with its dependencies installed (e.g. `pip install -e .` in a venv, with
    that venv active or its pytest otherwise on PATH).
  - --collect-root is a path *inside* that checkout, e.g. "osc_validation/validation"
    (note the underscore - it is a Python package name, not a directory to guess).

Example, run from the esmini repo root, assuming a sibling checkout of
osc-validation with its venv already active:
    python3 test/sync_osc_validation_whitelist.py \
        --whitelist .github/actions/run_osc_validation/whitelist.txt \
        --collect-root osc_validation/validation \
        --cwd ../osc-validation
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

VALID_KEYWORDS = {"INCLUDE", "EXCLUDE", "DEVIATION"}
# Matches pytest -q collect-only summary lines, e.g. "28 tests collected in 1.76s"
# or "no tests collected", which should not be treated as test ids.
SUMMARY_LINE_RE = re.compile(r"^(\d+ tests? collected|no tests collected)", re.IGNORECASE)


def collect_tests(cwd: Path, collect_root: str) -> list[str]:
    """Run pytest --collect-only and return the collected test ids, prefixed
    with collect_root so they match the format used in whitelist.txt."""
    result = subprocess.run(
        ["pytest", collect_root, "--collect-only", "-q"],
        cwd=cwd,
        capture_output=True,
        text=True,
    )
    # pytest exits with 5 when no tests are collected, which is not an error here.
    if result.returncode not in (0, 5):
        print(result.stdout)
        print(result.stderr, file=sys.stderr)
        raise SystemExit(f"pytest --collect-only failed with exit code {result.returncode}")

    prefix = collect_root.rstrip("/") + "/"
    tests = []
    for line in result.stdout.splitlines():
        line = line.strip()
        if not line or line.startswith("=") or SUMMARY_LINE_RE.match(line):
            continue
        tests.append(prefix + line)
    return tests


def parse_whitelist(path: Path) -> tuple[list[str], dict[str, str]]:
    """Return the raw lines of the whitelist plus a mapping of test id -> keyword."""
    lines = path.read_text().splitlines() if path.exists() else []
    known: dict[str, str] = {}
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        parts = stripped.split(maxsplit=1)
        if len(parts) == 2 and parts[0] in VALID_KEYWORDS:
            known[parts[1]] = parts[0]
    return lines, known


def sync(whitelist_path: Path, collected: list[str]) -> None:
    lines, known = parse_whitelist(whitelist_path)

    new_tests = [t for t in collected if t not in known]
    stale_tests = sorted(set(known) - set(collected))

    if new_tests:
        if lines and lines[-1].strip() != "":
            lines.append("")
        lines.append("# Newly discovered tests (auto-added, review and update the keyword)")
        for test in new_tests:
            lines.append(f"EXCLUDE {test}")
        whitelist_path.write_text("\n".join(lines) + "\n")

    included = sum(1 for keyword in known.values() if keyword == "INCLUDE")
    excluded = sum(1 for keyword in known.values() if keyword == "EXCLUDE")

    print(f"Collected tests:     {len(collected)}")
    print(f"New tests added:     {len(new_tests)} (as EXCLUDE)")
    print(f"Existing included:   {included}")
    print(f"Existing excluded:   {excluded}")
    print(f"Existing total:      {len(known)}")

    if stale_tests:
        print(f"\nWarning: {len(stale_tests)} whitelist entries were not collected by pytest (stale?):")
        for test in stale_tests:
            print(f"  - {test}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--whitelist", required=True, type=Path, help="Path to whitelist.txt")
    parser.add_argument(
        "--cwd",
        default=Path("."),
        type=Path,
        help="Directory to run pytest from, i.e. the osc-validation checkout (default: %(default)s)",
    )
    parser.add_argument(
        "--collect-root",
        default="osc_validation/validation",
        help="pytest collection root, relative to --cwd (default: %(default)s)",
    )
    args = parser.parse_args()

    if not args.cwd.is_dir():
        raise SystemExit(f"--cwd '{args.cwd}' is not a directory (expected the osc-validation checkout)")

    collect_path = args.cwd / args.collect_root
    if not collect_path.exists():
        raise SystemExit(
            f"--collect-root '{args.collect_root}' does not exist under --cwd '{args.cwd}' "
            f"(looked for '{collect_path}'). Did you mean 'osc_validation/validation' (with an underscore)?"
        )

    collected = collect_tests(args.cwd, args.collect_root)
    sync(args.whitelist, collected)


if __name__ == "__main__":
    main()
