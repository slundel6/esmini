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
 * This controller let the user control the vehicle interactively by the arrow keys
 */

#pragma once

#include <string>
#include "Controller.hpp"
#include "pugixml.hpp"
#include "Parameters.hpp"
#include "vehicle.hpp"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#elif __linux__
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>           // For open()
#include <unistd.h>          // For read() and close()
#include <linux/joystick.h>  // For joystick event structure and ioctl commands
#endif

#define CONTROLLER_HID_TYPE_NAME "HIDController"

namespace scenarioengine
{
    // base class for controllers
    class ControllerHID : public Controller
    {
    public:
        enum HID_INPUT
        {
            HID_AXIS_X       = 0,
            HID_AXIS_Y       = 1,
            HID_AXIS_Z       = 2,
            HID_AXIS_RX      = 3,
            HID_AXIS_RY      = 4,
            HID_AXIS_RZ      = 5,
            HID_BTN_0        = 6,  // button starts on ID 0 in Linux, 1 in Windows
            HID_BTN_1        = 7,
            HID_BTN_2        = 8,
            HID_BTN_3        = 9,
            HID_BTN_4        = 10,
            HID_BTN_5        = 11,
            HID_BTN_6        = 12,
            HID_BTN_7        = 13,
            HID_BTN_8        = 14,
            HID_BTN_9        = 15,
            HID_NR_OF_INPUTS = 16
        };

        ControllerHID(InitArgs* args);
        ~ControllerHID() override;

        void Init();
        void Step(double timeStep);
        int  Activate(ControlActivationMode lat_activation_mode,
                      ControlActivationMode long_activation_mode,
                      ControlActivationMode light_activation_mode,
                      ControlActivationMode anim_activation_mode);

        void Deactivate() override;

        static const char* GetTypeNameStatic()
        {
            return CONTROLLER_HID_TYPE_NAME;
        }
        virtual const char* GetTypeName()
        {
            return GetTypeNameStatic();
        }
        static int GetTypeStatic()
        {
            return Controller::Type::CONTROLLER_TYPE_HID;
        }
        virtual int GetType()
        {
            return GetTypeStatic();
        }
        int  OpenHID(int device_id);
        void CloseHID();
        int  ReadHID(double& throttle, double& steering);
        int  ParseHIDInputType(const std::string& axis, HID_INPUT& axis_type);

    private:
        vehicle::Vehicle vehicle_;
        double           steering_           = 0.0;
        double           throttle_           = 0.0;
        double           steering_rate_      = 0.0;
        int              device_id_          = 0;
        HID_INPUT        steering_input_     = HID_INPUT::HID_AXIS_X;
        HID_INPUT        throttle_input_     = HID_INPUT::HID_AXIS_RZ;
        HID_INPUT        brake_input_        = HID_INPUT::HID_AXIS_RZ;
        int              device_id_internal_ = -1;
        int64_t          values_[HID_INPUT::HID_NR_OF_INPUTS];
#ifdef _WIN32
        JOYINFOEX joy_info_;
#endif
    };

    Controller* InstantiateControllerHID(void* args);
}  // namespace scenarioengine