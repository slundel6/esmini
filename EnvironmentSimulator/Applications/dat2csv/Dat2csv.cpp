/*
 * esmini - Environment Simulator Minimalistic
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

/*
 * This application uses the Replay class to read and binary recordings and print content in ascii format to stdout
 */

#include "Dat2csv.hpp"
#include "CommonMini.hpp"
#include "DatLogger.hpp"
#include "Replay.hpp"

#define MAX_LINE_LEN 2048

Dat2csv::Dat2csv(std::string filename) : log_mode_(log_mode::ORIGINAL), step_time_(0.05)
{
    std::string filename_ = FileNameWithoutExtOf(filename) + ".csv";

    file.open(filename_);
    if (!file.is_open())
    {
        LOG_AND_QUIT("Failed to create file %s\n", filename_.c_str());
    }

    // Create replayer object for parsing the binary data file
    try
    {
        player = std::make_unique<scenarioengine::Replay>(filename);
    }
    catch (const std::exception& e)
    {
        LOG_AND_QUIT("%s", e.what());
    }

    static char       line[MAX_LINE_LEN];
    datLogger::DatHdr headerNew_;
    headerNew_ = *reinterpret_cast<datLogger::DatHdr*>(player->header_.content);

    // First output header and CSV labels
    snprintf(line,
             MAX_LINE_LEN,
             "Version: %d, OpenDRIVE: %s, 3DModel: %s\n",
             headerNew_.version,
             headerNew_.odrFilename.string,
             headerNew_.modelFilename.string);
    if (include_refs)
    {
        if (!extended)
        {
            file << line;
            snprintf(line, MAX_LINE_LEN, "time, id, name, x, y, z, h, p, r, speed, wheel_angle, wheel_rot\n");
            file << line;
        }
        else
        {
            file << line;
            snprintf(line, MAX_LINE_LEN, "time, id, name, x, y, z, h, p, r, roadId, laneId, offset, t, s, speed, wheel_angle, wheel_rot\n");
            file << line;
        }
    }
    // player->SetShowRestart(true); // include restart details always in csv files
}

Dat2csv::~Dat2csv()
{
}

void Dat2csv::CreateCSV()
{
    static char line[MAX_LINE_LEN];
    if (log_mode_ == log_mode::MIN_STEP || log_mode_ == log_mode::TIME_STEP || log_mode_ == log_mode::TIME_STEP_MIXED)
    {  // delta time setting, write for each delta time+sim time. Will skips original time frame
        double requestedTime = SMALL_NUMBER;
        double delta_time    = SMALL_NUMBER;
        if (log_mode_ == log_mode::MIN_STEP)
        {
            delta_time = player->deltaTime_;
        }
        else
        {
            delta_time = step_time_;
        }
        while (true)
        {
            for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
            {
                if (player->scenarioState.obj_states[i].active == true)  // only for active members may be its deleted
                {
                    int         obj_id = player->scenarioState.obj_states[i].id;
                    std::string name;
                    player->GetName(player->scenarioState.obj_states[i].id, name);
                    if (!extended)
                    {
                        snprintf(line,
                                 MAX_LINE_LEN,
                                 "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                 player->scenarioState.sim_time,
                                 obj_id,
                                 name.c_str(),
                                 player->GetX(player->scenarioState.obj_states[i].id),
                                 player->GetY(player->scenarioState.obj_states[i].id),
                                 player->GetZ(player->scenarioState.obj_states[i].id),
                                 player->GetH(player->scenarioState.obj_states[i].id),
                                 player->GetP(player->scenarioState.obj_states[i].id),
                                 player->GetR(player->scenarioState.obj_states[i].id),
                                 player->GetSpeed(player->scenarioState.obj_states[i].id),
                                 player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                                 player->GetWheelRot(player->scenarioState.obj_states[i].id));
                        file << line;
                    }
                    else
                    {
                        snprintf(line,
                                 MAX_LINE_LEN,
                                 "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                 player->scenarioState.sim_time,
                                 obj_id,
                                 name.c_str(),
                                 player->GetX(player->scenarioState.obj_states[i].id),
                                 player->GetY(player->scenarioState.obj_states[i].id),
                                 player->GetZ(player->scenarioState.obj_states[i].id),
                                 player->GetH(player->scenarioState.obj_states[i].id),
                                 player->GetP(player->scenarioState.obj_states[i].id),
                                 player->GetR(player->scenarioState.obj_states[i].id),
                                 player->GetRoadId(player->scenarioState.obj_states[i].id),
                                 player->GetLaneId(player->scenarioState.obj_states[i].id),
                                 player->GetPosOffset(player->scenarioState.obj_states[i].id),
                                 static_cast<double>(player->GetPosT(player->scenarioState.obj_states[i].id)),
                                 static_cast<double>(player->GetPosS(player->scenarioState.obj_states[i].id)),
                                 player->GetSpeed(player->scenarioState.obj_states[i].id),
                                 player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                                 player->GetWheelRot(player->scenarioState.obj_states[i].id));
                        file << line;
                    }
                }
            }

            if (player->GetTime() > player->GetStopTime() - SMALL_NUMBER)
            {
                break;  // reached end of file
            }
            else if (delta_time < SMALL_NUMBER)
            {
                LOG("Warning: Unexpected delta time zero found! Can't process remaining part of the file");
                break;
            }
            else
            {
                if (log_mode_ == log_mode::MIN_STEP || log_mode_ == log_mode::TIME_STEP)
                {
                    player->MoveToTime(player->GetTime() + delta_time);  // continue
                }
                else
                {
                    if (isEqualDouble(player->GetTime(), requestedTime) || isEqualDouble(player->GetTime(), player->GetStartTime()))
                    {  // first time frame or until reach requested time frame reached, dont move to next time frame
                        requestedTime = player->GetTime() + delta_time;
                        player->MoveToTime(player->GetTime() + delta_time, false, true);  // continue
                    }
                    else
                    {
                        player->MoveToTime(requestedTime, false, true);  // continue
                    }
                }
            }
        }
    }
    else if (log_mode_ == log_mode::MIN_STEP_MIXED)
    {  // write for each delta time+sim time and also original time frame if available in between. Dont skip original time frame.
        double requestedTime = SMALL_NUMBER;
        while (true)
        {
            for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
            {
                if (player->scenarioState.obj_states[i].active == true)  // only for active members may be its deleted
                {
                    int         obj_id = player->scenarioState.obj_states[i].id;
                    std::string name;
                    player->GetName(player->scenarioState.obj_states[i].id, name);
                    if (!extended)
                    {
                        snprintf(line,
                                 MAX_LINE_LEN,
                                 "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                 player->scenarioState.sim_time,
                                 obj_id,
                                 name.c_str(),
                                 player->GetX(player->scenarioState.obj_states[i].id),
                                 player->GetY(player->scenarioState.obj_states[i].id),
                                 player->GetZ(player->scenarioState.obj_states[i].id),
                                 player->GetH(player->scenarioState.obj_states[i].id),
                                 player->GetP(player->scenarioState.obj_states[i].id),
                                 player->GetR(player->scenarioState.obj_states[i].id),
                                 player->GetSpeed(player->scenarioState.obj_states[i].id),
                                 player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                                 player->GetWheelRot(player->scenarioState.obj_states[i].id));
                        file << line;
                    }
                    else
                    {
                        snprintf(line,
                                 MAX_LINE_LEN,
                                 "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                 player->scenarioState.sim_time,
                                 obj_id,
                                 name.c_str(),
                                 player->GetX(player->scenarioState.obj_states[i].id),
                                 player->GetY(player->scenarioState.obj_states[i].id),
                                 player->GetZ(player->scenarioState.obj_states[i].id),
                                 player->GetH(player->scenarioState.obj_states[i].id),
                                 player->GetP(player->scenarioState.obj_states[i].id),
                                 player->GetR(player->scenarioState.obj_states[i].id),
                                 player->GetRoadId(player->scenarioState.obj_states[i].id),
                                 player->GetLaneId(player->scenarioState.obj_states[i].id),
                                 player->GetPosOffset(player->scenarioState.obj_states[i].id),
                                 static_cast<double>(player->GetPosT(player->scenarioState.obj_states[i].id)),
                                 static_cast<double>(player->GetPosS(player->scenarioState.obj_states[i].id)),
                                 player->GetSpeed(player->scenarioState.obj_states[i].id),
                                 player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                                 player->GetWheelRot(player->scenarioState.obj_states[i].id));
                        file << line;
                    }
                }
            }

            if (player->GetTime() > player->GetStopTime() - SMALL_NUMBER)
            {
                break;  // reached end of file
            }
            else if (player->deltaTime_ < SMALL_NUMBER)
            {
                LOG("Warning: Unexpected delta time zero found! Can't process remaining part of the file");
                break;
            }
            else
            {
                if (isEqualDouble(player->GetTime(), requestedTime) || isEqualDouble(player->GetTime(), player->GetStartTime()))
                {  // first time frame or until reach requested time frame reached, dont move to next time frame
                    requestedTime = player->GetTime() + player->deltaTime_;
                    player->MoveToTime(player->GetTime() + player->deltaTime_, false, true);  // continue
                }
                else
                {
                    player->MoveToTime(requestedTime, false, true);  // continue
                }
            }
        }
    }
    else if (log_mode_ == log_mode::ORIGINAL)
    {  // default setting, write time stamps available only in dat file
        for (size_t j = 0; j < player->pkgs_.size(); j++)
        {
            if (player->pkgs_[j].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES))
            {
                double timeTemp = *reinterpret_cast<double*>(player->pkgs_[j].content);

                // next time
                player->SetTime(timeTemp);
                player->SetIndex(static_cast<int>(j));

                player->CheckObjAvailabilityForward();
                player->UpdateCache();
                player->scenarioState.sim_time = timeTemp;
                for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
                {
                    if (player->scenarioState.obj_states[i].active == true)  // only for active members may be its deleted
                    {
                        int         obj_id = player->scenarioState.obj_states[i].id;
                        std::string name;
                        player->GetName(player->scenarioState.obj_states[i].id, name);
                        if (!extended)
                        {
                            snprintf(line,
                                     MAX_LINE_LEN,
                                     "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                     player->scenarioState.sim_time,
                                     obj_id,
                                     name.c_str(),
                                     player->GetX(obj_id),
                                     player->GetY(obj_id),
                                     player->GetZ(obj_id),
                                     player->GetH(obj_id),
                                     player->GetP(obj_id),
                                     player->GetR(obj_id),
                                     player->GetSpeed(obj_id),
                                     player->GetWheelAngle(obj_id),
                                     player->GetWheelRot(obj_id));
                            file << line;
                        }
                        else
                        {
                            snprintf(line,
                                     MAX_LINE_LEN,
                                     "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                                     player->scenarioState.sim_time,
                                     obj_id,
                                     name.c_str(),
                                     player->GetX(player->scenarioState.obj_states[i].id),
                                     player->GetY(player->scenarioState.obj_states[i].id),
                                     player->GetZ(player->scenarioState.obj_states[i].id),
                                     player->GetH(player->scenarioState.obj_states[i].id),
                                     player->GetP(player->scenarioState.obj_states[i].id),
                                     player->GetR(player->scenarioState.obj_states[i].id),
                                     player->GetRoadId(player->scenarioState.obj_states[i].id),
                                     player->GetLaneId(player->scenarioState.obj_states[i].id),
                                     player->GetPosOffset(player->scenarioState.obj_states[i].id),
                                     static_cast<double>(player->GetPosT(player->scenarioState.obj_states[i].id)),
                                     static_cast<double>(player->GetPosS(player->scenarioState.obj_states[i].id)),
                                     player->GetSpeed(player->scenarioState.obj_states[i].id),
                                     player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                                     player->GetWheelRot(player->scenarioState.obj_states[i].id));
                            file << line;
                        }
                    }
                }
            }
        }
    }
    file.close();
}