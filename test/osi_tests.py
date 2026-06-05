import re
from test_common import *
import unittest
import subprocess
import os.path

ESMINI_PATH = '../'
COMMON_ESMINI_ARGS = '--headless --fixed_timestep 0.1 '

class TestSuite(unittest.TestCase):

    def test_osi_to_csv(self):
        osi_log_name = "gt_to_convert.osi"
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'resources/xosc/cut-in.xosc'), COMMON_ESMINI_ARGS + "--osi_file " + osi_log_name)

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*cut-in.xosc', log)  is not None)

        args = ["python3", "../scripts/osi2csv.py", osi_log_name]
        subprocess.run(args, cwd=os.path.dirname(os.path.realpath(__file__)), check=True)

        csv = osi_log_name.split(".")[0] + ".csv"
        csv_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), csv)
        with open(csv_path, "r", encoding="utf-8") as csv_file:
            csv_content = csv_file.read()

        # Check some scenario events
        self.assertTrue(re.search('0.00.*, 36, obj36, CAR, 8.17.*', csv_content))
        self.assertTrue(re.search('10.00.*, 37, obj37, CAR, 11.70.*', csv_content))
        self.assertTrue(re.search('22.30.*, 37, obj37, CAR, 15.42.*', csv_content))

if __name__ == "__main__":
    unittest.main(argv=[''], verbosity=2)