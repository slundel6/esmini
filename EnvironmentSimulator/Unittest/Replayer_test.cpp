#include <gtest/gtest.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <dirent.h>

#include "ScenarioEngine.hpp"
#include "esminiLib.hpp"
#include "CommonMini.hpp"
#include "DatLogger.hpp"
#include "Dat2csv.hpp"
#include "Replay.hpp"

using namespace datLogger;

TEST(TestReplayer, WithOneObject)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;
    int    no_of_obj    = 1;
    int    total_time   = 6;

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    delete logger;

    std::unique_ptr<scenarioengine::Replay> replayer_ = std::make_unique<scenarioengine::Replay>(fileName);

    ASSERT_EQ(replayer_->pkgs_.size(), 21);  // header not stored, 1 scenario end pkg added
    ASSERT_EQ(replayer_->scenarioState.sim_time, replayer_->GetTimeFromCnt(1));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->GetTimeFromCnt(2), 1.1220000000000001);

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(2));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
}

TEST(TestReplayer, WithTwoObject)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj  = 2;
    int total_time = 6;

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    delete logger;

    std::unique_ptr<scenarioengine::Replay> replayer_ = std::make_unique<scenarioengine::Replay>(fileName);

    ASSERT_EQ(replayer_->pkgs_.size(), 35);  // header not stored, 1 scenario end pkg added
    ASSERT_EQ(replayer_->scenarioState.sim_time, replayer_->GetTimeFromCnt(1));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->GetTimeFromCnt(2), 1.1220000000000001);

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(2));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(2));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(4));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(4));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(5));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
}

TEST(TestReplayer, WithTwoObjectAndAddAndDelete)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj  = 3;
    int total_time = 6;

    // calc
    //  3obj- one obj deleted so 1 pos + speed pkg less
    //  1 hdr, 6 time, 18 obj id, 17 pos, 17 speed, 1 dele pkg, 3 obj added  = 63 pkg  received
    //  1 hdr, 6 time, 18 obj id, 7 pos, 14 speed, 1 dele pkg, 4 obj added  = 50 pkg

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    delete logger;

    std::unique_ptr<scenarioengine::Replay> replayer_ = std::make_unique<scenarioengine::Replay>(fileName);
    ASSERT_EQ(replayer_->pkgs_.size(), 51);  // header not stored.

    ASSERT_EQ(replayer_->scenarioState.sim_time, replayer_->GetTimeFromCnt(1));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->GetTimeFromCnt(2), 1.122);

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(2));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[1].time_, replayer_->GetTimeFromCnt(2));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(3));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(replayer_->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[2].id, 2);
    ASSERT_EQ(replayer_->scenarioState.obj_states[2].active, false);  // obj deleted
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(3));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(3));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(4));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);  // obj added
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(replayer_->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[2].id, 2);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[0].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[1].time_, replayer_->GetTimeFromCnt(4));

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(5));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[1].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[0].time_, replayer_->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[2].pkgs[1].time_, replayer_->GetTimeFromCnt(4));
}

TEST(TestReplayer, RepeatedObjectState)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;
    int    no_of_obj    = 1;
    int    total_time   = 6;

    for (int i = 0; i < total_time; i++)
    {
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    delete logger;

    std::unique_ptr<scenarioengine::Replay> replayer_ = std::make_unique<scenarioengine::Replay>(fileName);
    ASSERT_EQ(replayer_->pkgs_.size(), 7);  // header not stored, last time added

    ASSERT_EQ(replayer_->scenarioState.sim_time, replayer_->GetTimeFromCnt(1));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replayer_->GetTimeFromCnt(1), 0.033000000000000002);

    replayer_->MoveToTime(replayer_->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[0].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetTimeFromCnt(1));
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    EXPECT_NEAR(replayer_->GetTimeFromCnt(2), 5.4779999999999998, 1E-3);
}

TEST(TestReplayer, SimpleScenario)
{
    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    scenarioengine::Replay* replayer_ = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replayer_->pkgs_.size(), 3170);

    replayer_->MoveToTime(2.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 2.00, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[6].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[11].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 2.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 2.0, 1E-3);

    replayer_->MoveToTime(8.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 8.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 8.0, 1E-3);

    replayer_->MoveToTime(15.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 15.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 15.0, 1E-3);
}

TEST(TestReplayer, SpeedChangeScenario)
{
    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/speed_change.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    scenarioengine::Replay* replayer_ = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replayer_->pkgs_.size(), 3211);

    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    replayer_->MoveToTime(18);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, 17.200000256299973);  // pos
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 17.250000257045031);  // speed
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replayer_->MoveToTime(19.5);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, 17.200000256299973);  // pos
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 17.250000257045031);  // speed
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replayer_->MoveToTime(21);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, 21.000000312924385);  // pos
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 21.000000312924385);  // speed
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[4].time_, 0);
}

TEST(TestReplayer, TwoSimpleScenarioMerge)
{
    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario.xosc", "--record", "simple_scenario_.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    const char* args1[] = {"--osc",
                           "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario_reversed.xosc",
                           "--record",
                           "simple_scenario_reversed.dat",
                           "--fixed_timestep",
                           "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args1) / sizeof(char*), args1), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }
    SE_Close();

    std::unique_ptr<scenarioengine::Replay> replayer_ = std::make_unique<scenarioengine::Replay>(".", "simple_scenario_", "");
    ASSERT_EQ(replayer_->pkgs_.size(), 5737);

    replayer_->MoveToTime(2.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 2.00, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[6].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[11].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 2.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 2.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[6].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[11].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[2].time_, 2.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[7].time_, 2.0, 1E-3);

    replayer_->MoveToTime(8.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 8.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 8.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[7].time_, 8.0, 1E-3);

    replayer_->MoveToTime(15.0);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 15.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[0].pkgs[7].time_, 15.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[5].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[9].time_, 0.0, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[2].time_, 7.2, 1E-3);
    EXPECT_NEAR(replayer_->scenarioState.obj_states[1].pkgs[7].time_, 15.0, 1E-3);
}

TEST(TestReplayer, ShowAndNotShowRestart)
{
    const char* args[] = {"--osc",
                          "../../../EnvironmentSimulator/Unittest/xosc/timing_scenario_with_restarts.xosc",
                          "--record",
                          "new_sim.dat",
                          "--fixed_timestep",
                          "0.1"};
    SE_AddPath("../../../resources/xosc");
    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.01f);
    }

    SE_Close();

    scenarioengine::Replay* replayer_ = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replayer_->pkgs_.size(), 10426);

    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);

    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[1].time_, replayer_->GetStartTime());  // pos
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[2].time_, replayer_->GetStartTime());  // speed
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[3].time_, replayer_->GetStartTime());
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.obj_states[0].pkgs[4].time_, replayer_->GetStartTime());
    std::string name;
    replayer_->GetName(replayer_->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[0].id), -1.5, 1E-3);

    replayer_->GetName(replayer_->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[1].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[1].id), -4.5, 1E-3);

    replayer_->GetName(replayer_->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[2].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[2].id), -1.5, 1E-3);

    // with show restart
    replayer_->SetShowRestart(true);
    replayer_->GetRestartTimes();

    ASSERT_EQ(replayer_->restartTimes.size(), 2);
    ASSERT_EQ(replayer_->restartTimes[0].restart_index_, 2158);
    ASSERT_EQ(replayer_->restartTimes[0].next_index_, 2525);
    ASSERT_DOUBLE_EQ(replayer_->restartTimes[0].restart_time_, 2.009999955072999);
    ASSERT_DOUBLE_EQ(replayer_->restartTimes[0].next_time_, 2.0199999548494829);
    ASSERT_EQ(replayer_->restartTimes[1].restart_index_, 8062);
    ASSERT_EQ(replayer_->restartTimes[1].next_index_, 8429);
    ASSERT_DOUBLE_EQ(replayer_->restartTimes[1].restart_time_, 8.00999982096255);
    ASSERT_DOUBLE_EQ(replayer_->restartTimes[1].next_time_, 8.019999820739022);

    replayer_->MoveToTime(replayer_->restartTimes[0].restart_time_);  // first restart frame
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[0].restart_time_);
    replayer_->GetName(replayer_->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[2].id), 60.099, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replayer_->MoveToTime(replayer_->restartTimes[0].next_time_);  // shall go first restart frame
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, -0.93000004515051837);
    replayer_->GetName(replayer_->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[1].id), 50.399, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[2].id), 10.000, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replayer_->MoveToTime(replayer_->restartTimes[1].restart_time_);  // second restart frame
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[1].restart_time_);

    replayer_->MoveToTime(replayer_->restartTimes[1].next_time_);  // shall go second restart frame
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, 5.0699998207390324);

    // with no show restart
    replayer_->InitiateStates();
    replayer_->SetShowRestart(false);
    replayer_->GetRestartTimes();

    replayer_->MoveToTime(replayer_->restartTimes[0].restart_time_);  // first restart frame
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    replayer_->GetName(replayer_->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[0].restart_time_);
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[2].id), 60.099, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replayer_->MoveToTime(replayer_->restartTimes[0].next_time_);  // shall go next frame
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[0].next_time_);
    replayer_->GetName(replayer_->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replayer_->GetName(replayer_->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replayer_->GetX(replayer_->scenarioState.obj_states[2].id), 23.724, 1E-3);
    EXPECT_NEAR(replayer_->GetY(replayer_->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replayer_->MoveToTime(replayer_->restartTimes[1].restart_time_);  // second restart frame
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[1].restart_time_);

    replayer_->MoveToTime(replayer_->restartTimes[1].next_time_);  // shall go next frame
    ASSERT_DOUBLE_EQ(replayer_->scenarioState.sim_time, replayer_->restartTimes[1].next_time_);
}

TEST(TestReplayer, StopAtEachTimeFrame)
{
    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.5"};
    SE_AddPath("../../../resources/xosc");
    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.5f);
    }

    SE_Close();

    scenarioengine::Replay* replayer_ = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replayer_->pkgs_.size(), 341);

    ASSERT_EQ(replayer_->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replayer_->scenarioState.obj_states.size(), 1);
    replayer_->MoveToTime(0.0);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 0.0, 1E-3);

    replayer_->MoveToTime(0.5);
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 0.5, 1E-3);

    replayer_->MoveToTime(3, false, true);  // go through each frame
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 1, 1E-3);

    replayer_->MoveToTime(1.5, false, true);  // go through each frame
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 1.5, 1E-3);

    replayer_->MoveToTime(3, false, true);  // go through each frame
    EXPECT_NEAR(replayer_->scenarioState.sim_time, 2.0, 1E-3);
}

TEST(TestDat2Csv, TimeModes)
{
    const char* args[] = {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/test_time_mode.xosc", "--record", "sim.dat"};
    SE_AddPath("../../../../resources/xosc");
    SE_AddPath("../../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.01f);
    }

    SE_Close();

    std::unique_ptr<Dat2csv> dat_to_csv;
    dat_to_csv = std::make_unique<Dat2csv>("sim.dat");

    dat_to_csv->SetLogMode(Dat2csv::log_mode::ORIGINAL);
    dat_to_csv->CreateCSV();

    // Also check a few entries in the csv log file, focus on scenario controlled entity "Target"
    std::vector<std::vector<std::string>> csv_original;
    ASSERT_EQ(SE_ReadCSVFile("sim.csv", csv_original, 0), 0);
    EXPECT_NEAR(std::stod(csv_original[2][0]), 0.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[2][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[3][0]), 4.020, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[3][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[4][0]), 8.340, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[4][3]), 50.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[5][0]), 10.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_original[5][3]), 50.0, 1E-3);

    std::unique_ptr<Dat2csv> dat_to_csv1;
    dat_to_csv1 = std::make_unique<Dat2csv>("sim.dat");

    dat_to_csv1->SetLogMode(Dat2csv::log_mode::MIN_STEP);
    dat_to_csv1->CreateCSV();

    std::vector<std::vector<std::string>> csv_min_step;
    ASSERT_EQ(SE_ReadCSVFile("sim.csv", csv_min_step, 0), 0);
    EXPECT_NEAR(std::stod(csv_min_step[2][0]), 0.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[2][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[3][0]), 1.670, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[3][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[4][0]), 3.340, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[4][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[5][0]), 5.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[5][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[6][0]), 6.680, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[6][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[7][0]), 8.350, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[7][3]), 50.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[8][0]), 10.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step[8][3]), 50.0, 1E-3);

    std::unique_ptr<Dat2csv> dat_to_csv2;
    dat_to_csv2 = std::make_unique<Dat2csv>("sim.dat");

    dat_to_csv2->SetLogMode(Dat2csv::log_mode::MIN_STEP_MIXED);
    dat_to_csv2->CreateCSV();

    std::vector<std::vector<std::string>> csv_min_step_mixed;
    ASSERT_EQ(SE_ReadCSVFile("sim.csv", csv_min_step_mixed, 0), 0);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[2][0]), 0.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[2][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[3][0]), 1.670, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[3][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[4][0]), 3.340, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[4][3]), 10.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[5][0]), 4.020, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[5][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[6][0]), 5.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[6][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[7][0]), 6.680, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[7][3]), 25.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[8][0]), 8.340, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[8][3]), 50.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[9][0]), 8.350, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[9][3]), 50.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[10][0]), 10.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_min_step_mixed[10][3]), 50.0, 1E-3);

    std::unique_ptr<Dat2csv> dat_to_csv3;
    dat_to_csv3 = std::make_unique<Dat2csv>("sim.dat");

    dat_to_csv3->SetLogMode(Dat2csv::log_mode::TIME_STEP);
    dat_to_csv3->SetStepTime(1);
    dat_to_csv3->CreateCSV();

    std::vector<std::vector<std::string>> csv_time_step;
    ASSERT_EQ(SE_ReadCSVFile("sim.csv", csv_time_step, 0), 0);
    EXPECT_NEAR(std::stod(csv_time_step[2][0]), 0.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[2][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[3][0]), 1.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[3][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[4][0]), 2.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[4][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[5][0]), 3.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[5][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[6][0]), 4.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[6][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[7][0]), 5.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[7][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[8][0]), 6.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[8][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[9][0]), 7.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[9][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[10][0]), 8.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[10][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[11][0]), 9.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[11][3]), 50.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[12][0]), 10.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[12][3]), 50.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step[13][0]), 10.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step[13][3]), 50.0, 1E-3);

    std::unique_ptr<Dat2csv> dat_to_csv4;
    dat_to_csv4 = std::make_unique<Dat2csv>("sim.dat");

    dat_to_csv4->SetLogMode(Dat2csv::log_mode::TIME_STEP_MIXED);
    dat_to_csv4->SetStepTime(1);
    dat_to_csv4->CreateCSV();

    std::vector<std::vector<std::string>> csv_time_step_mixed;
    ASSERT_EQ(SE_ReadCSVFile("sim.csv", csv_time_step_mixed, 0), 0);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[2][0]), 0.0, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[2][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[3][0]), 1.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[3][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[4][0]), 2.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[4][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[5][0]), 3.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[5][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[6][0]), 4.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[6][3]), 10.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[7][0]), 4.020, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[7][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[8][0]), 5.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[8][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[9][0]), 6.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[9][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[10][0]), 7.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[10][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[11][0]), 8.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[11][3]), 25.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[12][0]), 8.340, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[12][3]), 50.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[13][0]), 9.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[13][3]), 50.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[14][0]), 10.000, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[14][3]), 50.0, 1E-3);

    EXPECT_NEAR(std::stod(csv_time_step_mixed[15][0]), 10.010, 1E-3);
    EXPECT_NEAR(std::stod(csv_time_step_mixed[15][3]), 50.0, 1E-3);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}