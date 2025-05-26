import re
import sys
from test_common import *
import unittest
import argparse
import os.path

ESMINI_PATH = '../'
COMMON_ESMINI_ARGS = '--headless --fixed_timestep 0.1 --record sim.dat --osi_file --osi_crop_dynamic 0,300 '

BLACKLIST = ["sumo", "sloppy"]

class TestSuite(unittest.TestCase):
    pass

def make_memory_test(path: str, sce: str, level: int):
    def test_func(self):
        disable_controllers = False
        
        with open(os.path.join(path, sce), "r", encoding="utf-8") as xosc:
            for line in xosc:
                if "ParameterDistribution" in line:
                    self.skipTest(f"'{sce}' (contains 'ParameterDistribution')")
                    break
                if "interactiveDriver" in line or "InteractiveController" in line or "externalController" in line or "externalControllerWithGhost" in line:
                    disable_controllers = True
                    break

        EXTRA_ARGS = "--disable_controllers " if disable_controllers else ""
        _, _, _, errlog = run_scenario(os.path.join(path, sce), COMMON_ESMINI_ARGS + EXTRA_ARGS + '--log_level debug', None, None, False, False, level)

        # Check for memory leaks
        self.assertIsNotNone(re.search('.definitely lost. 0 bytes in 0 blocks', errlog))
        self.assertIsNotNone(re.search('.indirectly lost. 0 bytes in 0 blocks', errlog))
        self.assertIsNotNone(re.search('.possibly lost. 0 bytes in 0 blocks', errlog))

    return test_func

def skip_scenario(scenario: str):
    for item in BLACKLIST:
        if item in scenario:
            print(f"Skipping '{scenario}' (blacklisted)")
            return True
    return False

if __name__ == "__main__":
    # execute only if run as a script

    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--timeout", type=int, default=600, help="timeout per testcase")
    parser.add_argument("-p", "--path", nargs="+", help="relative path to the testcases directory")
    parser.add_argument("-sc", "--scenario", nargs="?", help="run only this scenario")
    args = parser.parse_args()

    print("timeout:", args.timeout, file=sys.stderr)
    set_timeout(args.timeout)
    
    # Loop over all paths given by the user
    # Then loop over all scenarios in each path
    # If scenario is in blacklist, skip it
    for scenario_path in args.path:
        scenario_path = os.path.join(ESMINI_PATH, scenario_path)
        if not os.path.exists(scenario_path):
            print(f"Path {scenario_path} does not exist. Skipping.")
            continue

        for scenario in os.listdir(scenario_path):
            if not scenario.endswith(".xosc") or skip_scenario(scenario_path):
                continue

            test_name = f"test_{os.path.splitext(scenario)[0]}"
            check_level = "full" if args.scenario else "summary"
            setattr(TestSuite, test_name, make_memory_test(scenario_path, scenario, check_level))
    if args.scenario:
        # Add test case name as argument to run only that test
        # example: smoke_test.py test_follow_ghost
        unittest.main(argv=['ignored', '-v', 'TestSuite.' + "test_" + args.scenario])
    else:
        unittest.main(argv=[''], verbosity=2)
