#!/usr/bin/env bash
#
# Run the whitelisted osc-validation explicit permutations against an esmini build.
#
# Used by the "Run explicit permutations" step in
# .github/actions/run_osc_validation/action.yml, but is also standalone so it
# can be copy-pasted into a local osc-validation checkout to run the same
# tests locally. When run locally, this script must be run from the root of
# an osc-validation checkout (with its venv already set up/activated), same
# as in CI.
#
# whitelist.txt is the single source of truth: test_profile.toml (required by
# osc-validations pytest plugin for xfail handling) is generated on the fly
# from the DEVIATION/reason entries in whitelist.txt, so it never needs to be
# maintained separately.
#
# Usage:
#   run_explicit_permutations.sh <whitelist_file> [sim_binary] [generated_profile_file]
#
# Arguments (all can also be provided via env vars WHITELIST / SIM_BIN / GENERATED_PROFILE):
#   whitelist_file          Path to whitelist.txt (e.g. esmini/.github/actions/run_osc_validation/whitelist.txt)
#   sim_binary               Path to the esmini binary to validate (default: ../bin/esmini, relative to cwd)
#   generated_profile_file   Where to write the generated test_profile.toml (default: a temp file, removed on exit)
#
# Example (from a local osc-validation checkout, next to an esmini checkout):
#   ./run_explicit_permutations.sh ../esmini/.github/actions/run_osc_validation/whitelist.txt

set -euo pipefail

WHITELIST="${1:-${WHITELIST:-}}"
SIM_BIN="${2:-${SIM_BIN:-}}"
GENERATED_PROFILE="${3:-${GENERATED_PROFILE:-}}"

# Check for zero length
if [ -z "$WHITELIST" ]; then
  echo "Usage: $0 <whitelist_file> [sim_binary] [generated_profile_file]" >&2
  exit 1
fi
if [ ! -f "$WHITELIST" ]; then
  echo "Whitelist file not found: $WHITELIST" >&2
  exit 1
fi

if [ -z "$SIM_BIN" ]; then
  SIM_BIN="$(pwd)/../bin/esmini"
fi

# If we generate a temporary profile, clean it up in the end
owns_generated_profile=false
if [ -z "$GENERATED_PROFILE" ]; then
  GENERATED_PROFILE="$(mktemp)"
  owns_generated_profile=true
fi

echo "Using whitelist:     $WHITELIST"
echo "Using esmini binary: $SIM_BIN"

# Unroll the whitelist into an array of tests to run, honoring
# INCLUDE/EXCLUDE/DEVIATION keywords (blank lines and '#' comments are
# ignored). DEVIATION tests run alongside INCLUDE tests, but are expected to
# fail. Each DEVIATION line must be immediately followed by a
# `reason = "..."` line, which is collected below to generate test_profile.toml.
tests=()
included_count=0
excluded_count=0
deviation_count=0
deviation_tests=()
deviation_reasons=()
pending_deviation=""

fail_if_pending_deviation() {
  if [ -n "$pending_deviation" ]; then
    echo "DEVIATION test '$pending_deviation' in $WHITELIST is missing its following 'reason = \"...\"' line" >&2
    exit 1
  fi
}

while read -r keyword test; do
  [ -z "$keyword" ] && continue
  case "$keyword" in
    \#*) continue ;;
    INCLUDE) fail_if_pending_deviation; tests+=("$test"); included_count=$((included_count + 1)) ;;
    DEVIATION) fail_if_pending_deviation; tests+=("$test"); deviation_count=$((deviation_count + 1)); pending_deviation="$test" ;;
    EXCLUDE) fail_if_pending_deviation; echo "Excluding: $test"; excluded_count=$((excluded_count + 1)) ;;
    reason)
      if [ -z "$pending_deviation" ]; then
        echo "Found a 'reason' line in $WHITELIST with no preceding DEVIATION test" >&2
        exit 1
      fi
      # $test holds the remainder of the line, e.g.: = "Known deviation: ..."
      reason="${test#*\"}"
      reason="${reason%\"}"
      deviation_tests+=("$pending_deviation")
      deviation_reasons+=("$reason")
      pending_deviation=""
      ;;
    *) echo "Unknown keyword '$keyword' in $WHITELIST" >&2; exit 1 ;;
  esac
done < "$WHITELIST"
fail_if_pending_deviation
total_count=$((included_count + excluded_count + deviation_count))

echo "Included tests:  $included_count"
echo "Excluded tests:  $excluded_count"
echo "Deviation tests: $deviation_count"
echo "Total tests:     $total_count"

if [ "${#tests[@]}" -eq 0 ]; then
  echo "No INCLUDE/DEVIATION tests found in $WHITELIST"
  exit 1
fi

echo "Running tests:"
printf '  %s\n' "${tests[@]}"

# Generate test_profile.toml from the DEVIATION/reason pairs collected above.
# osc-validations test id format drops the "osc_validation/validation/"
# prefix used in whitelist.txt.
generate_test_profile() {
  local out="$1"
  : > "$out"
  local i
  for i in "${!deviation_tests[@]}"; do
    local test_id="${deviation_tests[$i]#osc_validation/validation/}"
    {
      echo "[[xfail]]"
      printf 'test = "%s"\n' "$test_id"
      printf 'reason = "%s"\n' "${deviation_reasons[$i]}"
      echo
    } >> "$out"
  done
}
generate_test_profile "$GENERATED_PROFILE"
echo "Generated test profile: $GENERATED_PROFILE"

LOG_FILE="$(mktemp)"

set +e
pytest \
  "${tests[@]}" \
  --toolpath "$SIM_BIN" \
  --tool ESMini \
  --test-profile "$GENERATED_PROFILE" \
  2>&1 | tee "$LOG_FILE"
exitcode=${PIPESTATUS[0]}
set -e

# pytests final summary line looks like:
#   "1 failed, 2 passed, 1 xfailed in 3.50s"
# Extract the count for each outcome category it reports.
get_count() {
  grep -oE "[0-9]+ $1" "$LOG_FILE" | tail -n1 | grep -oE '^[0-9]+' || echo 0
}
passed_count="$(get_count passed)"
failed_count="$(get_count failed)"
deviated_count="$(get_count xfailed)"
unexpectedly_passed_count="$(get_count xpassed)"

echo "Passed:              $passed_count"
echo "Failed:              $failed_count"
echo "Deviated (xfailed):  $deviated_count"
echo "Unexpectedly passed: $unexpectedly_passed_count"

SUMMARY=""
if [ -n "${VERSION_NOTICE:-}" ]; then
  SUMMARY="### osc-validation version check

$VERSION_NOTICE

"
fi

SUMMARY="${SUMMARY}$(cat <<EOF
### osc-validation explicit permutations

**Whitelist:**
- Included: $included_count
- Deviations: $deviation_count
- Excluded: $excluded_count
- Total: $total_count

**Run results:**
- Passed: $passed_count
- Failed: $failed_count
- Deviated: $deviated_count
EOF
)"
if [ "$unexpectedly_passed_count" != "0" ]; then
  SUMMARY="$SUMMARY
- Unexpectedly passed (no longer deviates): $unexpectedly_passed_count"
fi

if [ "${#deviation_tests[@]}" -gt 0 ]; then
  SUMMARY="$SUMMARY

### osc-validation deviations
"
  for i in "${!deviation_tests[@]}"; do
    SUMMARY="$SUMMARY
- ${deviation_tests[$i]#osc_validation/validation/}
  ${deviation_reasons[$i]}"
  done
fi

echo "$SUMMARY"
if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
  echo "$SUMMARY" >> "$GITHUB_STEP_SUMMARY"
fi

rm -f "$LOG_FILE"
if [ "$owns_generated_profile" = true ]; then
  rm -f "$GENERATED_PROFILE"
fi
exit "$exitcode"

