#!/usr/bin/env python3

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

import yaml


RULES = ("INCLUDE", "DEVIATION", "EXCLUDE", "NOT_SUPPORTED")
VALIDATION_TEST_PREFIX = "osc_validation/validation/"
ESMINI_EXECUTABLE = Path(__file__).resolve().parents[1] / "bin" / "esmini"


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Run osc-validation test permutations from a manifest"
    )
    parser.add_argument(
        "--manifest",
        required=True,
        help="path to the osc_validation_manifest.yml file",
    )
    parser.add_argument(
        "-vp",
        "--validation-prefix",
        required=True,
        help="path to the osc-validation repository checkout",
    )
    parser.add_argument(
        "-g",
        "--generate-profile",
        help="path for the generated test profile (default: temporary file)",
    )
    parser.add_argument(
        "-esmini",
        "--esmini-executable",
        help="path to the esmini executable (default: ./bin/esmini)"
    )
    return parser.parse_args()


def load_rules(manifest_path):
    try:
        with manifest_path.open(encoding="utf-8") as manifest_file:
            document = yaml.safe_load(manifest_file)
    except (OSError, yaml.YAMLError) as error:
        raise ValueError(f"Unable to read manifest {manifest_path}: {error}") from error

    if not isinstance(document, dict) or not isinstance(document.get("rules"), dict):
        raise ValueError(f"{manifest_path} must contain a 'rules' mapping")

    rules = document["rules"]
    unknown_rules = set(rules) - set(RULES)
    if unknown_rules:
        raise ValueError(
            f"Unknown rule(s) in {manifest_path}: {', '.join(sorted(unknown_rules))}"
        )

    parsed_rules = {}
    for rule_name in RULES:
        entries = rules.get(rule_name, [])
        if not isinstance(entries, list):
            raise ValueError(f"'rules.{rule_name}' must be a list")

        parsed_entries = []
        for index, entry in enumerate(entries, start=1):
            if not isinstance(entry, dict) or not isinstance(entry.get("id"), str):
                raise ValueError(
                    f"'rules.{rule_name}[{index}]' must contain a string 'id'"
                )

            test_id = entry["id"].strip()
            if not test_id:
                raise ValueError(f"'rules.{rule_name}[{index}].id' must not be empty")

            reason = entry.get("reason")
            if rule_name == "DEVIATION" and (
                not isinstance(reason, str) or not reason.strip()
            ):
                raise ValueError(
                    f"DEVIATION test '{test_id}' 'reason' must not be empty"
                )

            parsed_entries.append({"id": test_id, "reason": reason})
        parsed_rules[rule_name] = parsed_entries

    return parsed_rules


def toml_string(value):
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\b", "\\b")
        .replace("\t", "\\t")
        .replace("\n", "\\n")
        .replace("\f", "\\f")
        .replace("\r", "\\r")
    )


def profile_test_id(test_id):
    if test_id.startswith(VALIDATION_TEST_PREFIX):
        return test_id[len(VALIDATION_TEST_PREFIX) :]
    return test_id


def write_test_profile(profile_path, deviations):
    with profile_path.open("w", encoding="utf-8") as profile_file:
        for deviation in deviations:
            test_id = toml_string(profile_test_id(deviation["id"]))
            reason = toml_string(deviation["reason"])
            profile_file.write(
                f'[[xfail]]\ntest = "{test_id}"\nreason = "{reason}"\n\n'
            )


def run_pytest(tests, profile_path, validation_prefix, esmini_exe):
    command = [
        sys.executable,
        "-m",
        "pytest",
        *tests,
        "--toolpath",
        str(esmini_exe),
        "--tool",
        "ESMini",
        "--test-profile",
        str(profile_path),
    ]
    process = subprocess.Popen(
        command,
        cwd=validation_prefix,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    output = []
    assert process.stdout is not None
    for line in process.stdout:
        print(line, end="", flush=True)
        output.append(line)

    return process.wait(), "".join(output)


def result_count(output, outcome):
    matches = re.findall(rf"\b(\d+) {re.escape(outcome)}\b", output)
    return int(matches[-1]) if matches else 0


def create_summary(rules, output):
    included_count = len(rules["INCLUDE"])
    deviation_count = len(rules["DEVIATION"])
    excluded_count = len(rules["EXCLUDE"])
    not_supported_count = len(rules["NOT_SUPPORTED"])
    total_count = included_count + deviation_count + excluded_count + not_supported_count

    sections = []
    version_notice = os.environ.get("VERSION_NOTICE")
    if version_notice:
        sections.append(f"### osc-validation version check\n\n{version_notice}")

    summary = f"""### osc-validation explicit permutations

**Whitelist:**
- Included: {included_count}
- Deviations: {deviation_count}
- Not supported: {not_supported_count}
- Excluded: {excluded_count}
- Total: {total_count}

**Run results:**
- Passed: {result_count(output, "passed")}
- Failed: {result_count(output, "failed")}
- Deviated: {result_count(output, "xfailed")}
Not run:
- Not supported: {not_supported_count}
- Excluded: {excluded_count}"""

    unexpectedly_passed = result_count(output, "xpassed")
    if unexpectedly_passed:
        summary += f"\n- Unexpectedly passed (no longer deviates): {unexpectedly_passed}"
    sections.append(summary)

    if rules["DEVIATION"]:
        deviations = ["### osc-validation deviations"]
        for deviation in rules["DEVIATION"]:
            deviations.append(
                f"- {profile_test_id(deviation['id'])}\n  {deviation['reason']}"
            )
        sections.append("\n\n".join(deviations))

    if rules["NOT_SUPPORTED"]:
        not_supported = ["### osc-validation not supported"]
        for scenario in rules["NOT_SUPPORTED"]:
            item = f"- {profile_test_id(scenario['id'])}"
            if scenario["reason"]:
                item += f"\n  {scenario['reason']}"
            not_supported.append(item)
        sections.append("\n\n".join(not_supported))

    if rules["EXCLUDE"]:
        exclusions = ["### osc-validation exclusions"]
        for exclusion in rules["EXCLUDE"]:
            item = f"- {profile_test_id(exclusion['id'])}"
            if exclusion["reason"]:
                item += f"\n  {exclusion['reason']}"
            exclusions.append(item)
        sections.append("\n\n".join(exclusions))

    sections.append("\n\nFor more details, click the CI job and expand step 'Run osc-validation'->'Run explicit permutations'")

    return "\n\n".join(sections) + "\n"


def main():
    args = parse_arguments()
    manifest_path = Path(args.manifest).resolve()
    validation_prefix = Path(args.validation_prefix).resolve()

    if not validation_prefix.is_dir():
        print(
            f"osc-validation checkout not found: {validation_prefix}", file=sys.stderr
        )
        return 1

    try:
        rules = load_rules(manifest_path)
    except ValueError as error:
        print(error, file=sys.stderr)
        return 1

    tests = [entry["id"] for name in ("INCLUDE", "DEVIATION") for entry in rules[name]]
    for entry in rules["EXCLUDE"]:
        reason = f" ({entry['reason']})" if entry["reason"] else ""
        print(f"Excluding: {entry['id']}{reason}")

    print(f"Using manifest:      {manifest_path}")
    print(f"Using validation:    {validation_prefix}")
    print(f"Using esmini binary: {ESMINI_EXECUTABLE}")
    print(f"Included tests:  {len(rules['INCLUDE'])}")
    print(f"Excluded tests:  {len(rules['EXCLUDE'])}")
    print(f"Deviation tests: {len(rules['DEVIATION'])}")
    print(f"Total tests:     {sum(len(rules[name]) for name in RULES)}")

    if not tests:
        print(f"No INCLUDE/DEVIATION tests found in {manifest_path}", file=sys.stderr)
        return 1

    print("Running tests:")
    for test_id in tests:
        print(f"  {test_id}")

    owns_profile = args.generate_profile is None
    if owns_profile:
        profile_file = tempfile.NamedTemporaryFile(delete=False)
        profile_file.close()
        profile_path = Path(profile_file.name)
    else:
        profile_path = Path(args.generate_profile).resolve()


    esmini_exe = ESMINI_EXECUTABLE
    if args.esmini_executable:
        esmini_exe = Path(args.esmini_executable).resolve()

    try:
        write_test_profile(profile_path, rules["DEVIATION"])
        print(f"Generated test profile: {profile_path}")
        exit_code, output = run_pytest(tests, profile_path, validation_prefix, esmini_exe)

        print(f"Passed:              {result_count(output, 'passed')}")
        print(f"Failed:              {result_count(output, 'failed')}")
        print(f"Deviated (xfailed):  {result_count(output, 'xfailed')}")
        print(f"Not run:")
        print(f"Not supported:       {len(rules['NOT_SUPPORTED'])}")
        print(f"Excluded:            {len(rules['EXCLUDE'])}")
        print(f"Unexpectedly passed: {result_count(output, 'xpassed')}")

        summary = create_summary(rules, output)
        print(summary, end="")
        summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
        if summary_path:
            with open(summary_path, "a", encoding="utf-8") as summary_file:
                summary_file.write(summary)
        return exit_code
    finally:
        if owns_profile:
            profile_path.unlink(missing_ok=True)


if __name__ == "__main__":
    sys.exit(main())
