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

#include "ControllerHID.hpp"
#include "CommonMini.hpp"
#include "Entities.hpp"
#include "ScenarioGateway.hpp"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#else
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>           // For open()
#include <unistd.h>          // For read() and close()
#include <linux/joystick.h>  // For joystick event structure and ioctl commands
#endif

using namespace scenarioengine;

// Maximum number of joystick devices to check (e.g., js0 to js9)
#define MAX_JOYSTICKS_TO_CHECK 10


Controller* scenarioengine::InstantiateControllerHID(void* args)
{
    Controller::InitArgs* initArgs = static_cast<Controller::InitArgs*>(args);

    return new ControllerHID(initArgs);
}

ControllerHID::ControllerHID(InitArgs* args) : Controller(args), steering_rate_(4.0), speed_factor_(1.0)
{
    if (args && args->properties)
    {
        if (args->properties->ValueExists("steeringRate"))
        {
            steering_rate_ = strtod(args->properties->GetValueStr("steeringRate"));
        }

        if (args->properties->ValueExists("speedFactor"))
        {
            speed_factor_ = strtod(args->properties->GetValueStr("speedFactor"));
        }
    }
    align_to_road_heading_on_deactivation_ = true;
    align_to_road_heading_on_activation_   = true;
}

void ControllerHID::Init()
{
    Controller::Init();
}

void ControllerHID::Step(double timeStep)
{
    double speed_limit = object_->pos_.GetSpeedLimit();

    if (speed_limit < SMALL_NUMBER)
    {
        // no speed limit defined, set something with regards to number of lanes
        if (object_->pos_.GetRoadById(object_->pos_.GetTrackId())
                ->GetNumberOfDrivingLanesSide(object_->pos_.GetS(), SIGN(object_->pos_.GetLaneId())) > 1)
        {
            speed_limit = 110 / 3.6;
        }
        else
        {
            speed_limit = 60 / 3.6;
        }
    }
    vehicle_.SetMaxSpeed(MIN(speed_factor_ * speed_limit, object_->GetMaxSpeed()));

    if (!(IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LONG))))
    {
        // Fetch speed from Default Controller
        vehicle_.speed_ = object_->GetSpeed();
    }

    // Update vehicle motion
    vehicle_.SetThrottleDisabled(!IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LONG)));
    vehicle_.SetSteeringDisabled(!IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LAT)));
    vehicle_.DrivingControlBinary(timeStep, accelerate, steer);

    if (IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LONG)) &&
        IsNotActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LAT)))
    {
        // Only longitudinal control, move along road
        double steplen = vehicle_.speed_ * timeStep;

        object_->MoveAlongS(steplen);

        // Fetch updated position
        vehicle_.posX_    = object_->pos_.GetX();
        vehicle_.posY_    = object_->pos_.GetY();
        vehicle_.heading_ = object_->pos_.GetH();
    }

    gateway_->updateObjectWorldPosXYH(object_->id_, 0.0, vehicle_.posX_, vehicle_.posY_, vehicle_.heading_);

    // Fetch Z and Pitch from OpenDRIVE position
    vehicle_.posZ_  = object_->pos_.GetZRoad();
    vehicle_.pitch_ = object_->pos_.GetPRoad();

    if (IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LONG)))
    {
        gateway_->updateObjectSpeed(object_->id_, 0.0, vehicle_.speed_);
    }

    if (IsActiveOnDomains(static_cast<unsigned int>(ControlDomains::DOMAIN_LAT)))
    {
        gateway_->updateObjectWheelAngle(object_->id_, 0.0, vehicle_.wheelAngle_);
    }

    Controller::Step(timeStep);
}

int ControllerHID::Activate(ControlActivationMode lat_activation_mode,
                                    ControlActivationMode long_activation_mode,
                                    ControlActivationMode light_activation_mode,
                                    ControlActivationMode anim_activation_mode)
{
    if (object_)
    {
        vehicle_.Reset();
        vehicle_.SetPos(object_->pos_.GetX(), object_->pos_.GetY(), object_->pos_.GetZ(), object_->pos_.GetH());
        vehicle_.SetLength(object_->boundingbox_.dimensions_.length_);
        vehicle_.speed_ = object_->GetSpeed();
        vehicle_.SetMaxAcc(object_->GetMaxAcceleration());
        vehicle_.SetMaxDec(object_->GetMaxDeceleration());
        vehicle_.SetSteeringRate(steering_rate_);
    }

    steer      = vehicle::STEERING_NONE;
    accelerate = vehicle::THROTTLE_NONE;

    object_->SetJunctionSelectorStrategy(roadmanager::Junction::JunctionStrategyType::SELECTOR_ANGLE);
    object_->SetJunctionSelectorAngle(0.0);

    return Controller::Activate(lat_activation_mode, long_activation_mode, light_activation_mode, anim_activation_mode);
}

void ControllerHID::ReportKeyEvent(int key, bool down)
{
    if (key == static_cast<int>(KeyType::KEY_Left))
    {
        if (down)
        {
            steer = vehicle::STEERING_LEFT;
        }
        else
        {
            steer = vehicle::STEERING_NONE;
        }
    }
    else if (key == static_cast<int>(KeyType::KEY_Right))
    {
        if (down)
        {
            steer = vehicle::STEERING_RIGHT;
        }
        else
        {
            steer = vehicle::STEERING_NONE;
        }
    }
    else if (key == static_cast<int>(KeyType::KEY_Up))
    {
        if (down)
        {
            accelerate = vehicle::THROTTLE_ACCELERATE;
        }
        else
        {
            accelerate = vehicle::THROTTLE_NONE;
        }
    }
    else if (key == static_cast<int>(KeyType::KEY_Down))
    {
        if (down)
        {
            accelerate = vehicle::THROTTLE_BRAKE;
        }
        else
        {
            accelerate = vehicle::THROTTLE_NONE;
        }
    }
}

#ifdef _WIN32

int ControllerHID::OpenHID()
{
    UINT numDevs = joyGetNumDevs();
    if (numDevs == 0)
    {
        std::cerr << "No joystick devices available.\n";
        return 1;
    }

    JOYINFOEX joyInfo = {};
    joyInfo.dwSize    = sizeof(JOYINFOEX);
    joyInfo.dwFlags   = JOY_RETURNALL;

    UINT deviceID = JOYSTICKID1;

    MMRESULT res = joyGetPosEx(deviceID, &joyInfo);
    if (res != JOYERR_NOERROR)
    {
        std::cerr << "Joystick " << deviceID << " not ready or not connected.\n";
        return 1;
    }

    std::cout << "Reading joystick state (Ctrl+C to quit):\n";

    while (true)
    {
        res = joyGetPosEx(deviceID, &joyInfo);
        if (res == JOYERR_NOERROR)
        {
            std::cout << "\rX: " << joyInfo.dwXpos << " Y: " << joyInfo.dwYpos << " Z: " << joyInfo.dwZpos << " Buttons: " << joyInfo.dwButtons
                      << "    " << std::flush;
        }
        Sleep(50);
    }

    return 0;
}

#else

int ControllerHID::OpenHID()
{
    int             joystick_fd = -1;
    int             js_nr       = 1;
    std::string     joystick_path;
    struct js_event js_event;

    joystick_path = "/dev/input/js" + std::to_string(js_nr);

    // Try to open the device file
    joystick_fd = open(joystick_path.c_str(), O_RDONLY | O_NONBLOCK);

    if (joystick_fd == -1)
    {
        // If opening fails, it likely means the device doesn't exist
        // or we don't have permissions, so continue to the next.
        return -1;
    }

    // Device opened successfully. Now query its capabilities.
    char name[128];
    if (ioctl(joystick_fd, JSIOCGNAME(sizeof(name)), name) < 0)
    {
        // Fallback if name cannot be read (e.g., older kernel or device)
        snprintf(name, sizeof(name), "Unknown Joystick %d", js_nr);
    }

    char num_axes = 0;
    ioctl(joystick_fd, JSIOCGAXES, &num_axes);  // Get number of axes

    char num_buttons = 0;
    ioctl(joystick_fd, JSIOCGBUTTONS, &num_buttons);  // Get number of buttons

    // Consider it a "real" joystick if it has at least one axis or one button
    if (num_axes > 0 || num_buttons > 0)
    {
        std::cout << "Found joystick: " << joystick_path << std::endl;
        std::cout << "  Name: " << name << std::endl;
        std::cout << "  Axes: " << (int)num_axes << std::endl;
        std::cout << "  Buttons: " << (int)num_buttons << std::endl;
    }
    else
    {
        // If it has no axes or buttons, it's not a joystick for our purpose
        std::cout << "Skipping " << joystick_path << " (no axes/buttons detected)." << std::endl;
        close(joystick_fd);  // Close this non-joystick device
        joystick_fd = -1;    // Reset fd
        return -1;
    }

    // Check if a joystick was found
    if (joystick_fd == -1)
    {
        std::cerr << "Error: No valid joystick device found (checked /dev/input/js0 to js" << MAX_JOYSTICKS_TO_CHECK - 1 << ")." << std::endl;
        std::cerr << "       Make sure your joystick is connected and you have read permissions." << std::endl;
        std::cerr << "       (Try 'sudo apt install jstest-gtk' and 'jstest-gtk' to verify)." << std::endl;
        return -1;
    }

    std::cout << "Reading input from " << joystick_path << ". Press Ctrl+C to exit." << std::endl;

    // 2. Read events in a loop from the found joystick
    while (true)
    {
        if (read(joystick_fd, &js_event, sizeof(struct js_event)) == sizeof(struct js_event))
        {
            // Mask out JS_EVENT_INIT (initial state event)
            switch (js_event.type & ~JS_EVENT_INIT)
            {
                case JS_EVENT_BUTTON:
                    std::cout << "Button " << (int)js_event.number << " " << (js_event.value ? "pressed" : "released") << std::endl;
                    break;
                case JS_EVENT_AXIS:
                    std::cout << "Axis " << (int)js_event.number << " value: " << js_event.value << std::endl;
                    break;
            }
        }
        usleep(1000);  // Sleep for 1ms to prevent busy-waiting
    }

    // 3. Close the device (unreachable in this infinite loop, but good practice)
    close(joystick_fd);
    return 0;
}

#endif
