import re
import sys
from sys import platform
from test_common import *
import unittest
import argparse
import os.path

ESMINI_PATH = '../'
COMMON_ESMINI_ARGS = '--headless --fixed_timestep 0.01 --record sim.dat '
COMMON_REPLAYER_ARGS = '--file sim.dat --headless --time_scale 10 --res_path ../resources --quit_at_end '


class TestSuite(unittest.TestCase):

    def build_type(self, build_type):
        result = subprocess.run(
            ["cmake", "-B", "../build", "-N", "-L"],
            capture_output=True,
            text=True,
            check=True,
            shell=False
        )
        return result.stdout.find("CMAKE_BUILD_TYPE:STRING=" + build_type) != -1

    def test_close_vehicle_crossing(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/CloseVehicleCrossing.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*CloseVehicleCrossing.xosc', log)  is not None)
        '''
        self.assertTrue(re.search('.*Added route waypoint 1, 209.10: road_id 15 lane_id -1 s 4.91 \\(intermediate\\)', log)  is not None)

        # Check some scenario events
        self.assertTrue(re.search('\\n.5.510.* Synchronize masterTimeToDest \\(0.003\\) reached within this timestep \\(0.010\\)', log)  is not None)
        self.assertTrue(re.search('\\n.5.510.* QuitCondition: true, delay: 4.00, speed: 7.00 == 7.00, edge: rising', log)  is not None)
        self.assertTrue(re.search('\\n.9.510.* DoneCondition: true\\n', log)  is not None)
        self.assertTrue(re.search('\\n.9.510.* QuitCondition: true\\n', log)  is not None)

        # Check vehicle key positions
        csv = generate_csv()
        self.assertTrue(re.search('\\n6.500, 0, Ego, 28.542, -7.876, 0.000, 1.779, 0.000, 0.000, 10.000, -0.006, 3.502', csv))
        self.assertTrue(re.search('\\n6.500, 1, NPC, 24.456, 0.305, 0.000, 5.394, 0.000, 0.000, 7.000, 0.310, 5.147', csv))
        '''

    def test_cut_in(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/CutIn.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*CutIn.xosc', log)  is not None)

    def test_double_lanechanger(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/DoubleLaneChanger.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*DoubleLaneChanger.xosc', log)  is not None)

    def test_end_of_traffic_jam_neighboring_lane_occupied(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/EndofTrafficJamNeighboringLaneOccupied.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*EndofTrafficJamNeighboringLaneOccupied.xosc', log)  is not None)

    def test_end_of_traffic_jam(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/EndOfTrafficJam.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*EndOfTrafficJam.xosc', log)  is not None)

    def test_fast_overtake_with_reinitialization(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/FastOvertakeWithReInitialization.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*FastOvertakeWithReInitialization.xosc', log)  is not None)

    def test_overtaker(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/Overtaker.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*Overtaker.xosc', log)  is not None)

    def test_sequantal_events_0_100_0_explicit(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SequentialEvents_0-100-0kph_Explicit.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*SequentialEvents_0-100-0kph_Explicit.xosc', log)  is not None)

    def test_sequantal_events_0_100_0_implicit(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SequentialEvents_0-100-0kph_Implicit.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*SequentialEvents_0-100-0kph_Implicit.xosc', log)  is not None)

    def test_simple_overtake(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SimpleOvertake.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*SimpleOvertake.xosc', log)  is not None)

    def test_slow_preceding_vehicle_determenistic_parameter_set(self):
        scenario = os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SlowPrecedingVehicleDeterministicParameterSet.xosc')

        _, _, _, output = run_scenario(scenario, '--return_nr_permutations', ignoreReturnCode=True, read_log=False)
        match = re.search(r'Nr permutations: (\d+)', output)
        self.assertTrue(match is not None, output)
        permutations = int(match.group(1))

        self.assertGreater(permutations, 0)

        _, _, _, _ = run_scenario(scenario, COMMON_ESMINI_ARGS + '--disable_controllers', read_log=False)

        for i in range(1, permutations + 1):
            with open(f"log_{i}_of_{permutations}.txt", 'r') as logfile:
                log = logfile.read()
            self.assertTrue(re.search(r'Loading .*SlowPrecedingVehicle\.xosc', log) is not None)
            self.assertTrue(re.search(rf'Parameter permutation {i}/{permutations}\b', log) is not None)
            if i == 1:
                self.assertTrue(re.search(r'Loaded OpenDRIVE: ../ASAM_OpenSCENARIO_v1\.4\.0_Examples/Databases/AB_RQ31_R2000\.xodr', log) is not None)
            else:
                self.assertTrue(re.search(r'Reusing OpenDRIVE: ../ASAM_OpenSCENARIO_v1\.4\.0_Examples/Databases/AB_RQ31_R2000\.xodr', log) is not None)
            os.remove(f"log_{i}_of_{permutations}.txt")

    def test_slow_preceding_vehicle(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SlowPrecedingVehicle.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*SlowPrecedingVehicle.xosc', log)  is not None)

    def test_synchronized_arrival_to_intersection(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/SynchronizedArrivalToIntersection.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*SynchronizedArrivalToIntersection.xosc', log)  is not None)

    def test_traffic_jam(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/TrafficJam.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*TrafficJam.xosc', log)  is not None)

    def test_trailer_connect(self):
        log, _, _, _ = run_scenario(os.path.join(ESMINI_PATH, 'ASAM_OpenSCENARIO_v1.4.0_Examples/TrailerConnect.xosc'), COMMON_ESMINI_ARGS \
            + '--disable_controllers')

        # Check some initialization steps
        self.assertTrue(re.search('Loading .*TrailerConnect.xosc', log)  is not None)


if __name__ == "__main__":
    # execute only if run as a script

    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--timeout", type=int, default=40, help="timeout per testcase")
    parser.add_argument("testcase", nargs="?", help="run only this testcase")
    args = parser.parse_args()

    print("timeout:", args.timeout, file=sys.stderr)
    set_timeout(args.timeout)

    if args.testcase:
        # Add test case name as argument to run only that test
        # example: smoke_test.py test_follow_ghost
        unittest.main(argv=['ignored', '-v', 'TestSuite.' + args.testcase])
    else:
        unittest.main(argv=[''], verbosity=2)
