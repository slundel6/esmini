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

using namespace scenarioengine;

Controller* scenarioengine::InstantiateControllerHID(void* args)
{
    Controller::InitArgs* initArgs = static_cast<Controller::InitArgs*>(args);

    return new ControllerHID(initArgs);
}

int ControllerHID::ParseHIDInputType(const std::string& type_str, HID_INPUT& type)
{
    if (type_str == "AXIS_X")
    {
        type = HID_INPUT::HID_AXIS_X;
    }
    else if (type_str == "AXIS_Y")
    {
        type = HID_INPUT::HID_AXIS_Y;
    }
    else if (type_str == "AXIS_Z")
    {
        type = HID_INPUT::HID_AXIS_Z;
    }
    else if (type_str == "AXIS_RX")
    {
        type = HID_INPUT::HID_AXIS_RX;
    }
    else if (type_str == "AXIS_RY")
    {
        type = HID_INPUT::HID_AXIS_RY;
    }
    else if (type_str == "AXIS_RZ")
    {
        type = HID_INPUT::HID_AXIS_RZ;
    }
    else if (type_str == "BTN_1")
    {
        type = HID_INPUT::HID_BTN_1;
    }
    else if (type_str == "BTN_2")
    {
        type = HID_INPUT::HID_BTN_2;
    }
    else if (type_str == "BTN_3")
    {
        type = HID_INPUT::HID_BTN_3;
    }
    else if (type_str == "BTN_4")
    {
        type = HID_INPUT::HID_BTN_4;
    }
    else if (type_str == "BTN_5")
    {
        type = HID_INPUT::HID_BTN_5;
    }
    else if (type_str == "BTN_6")
    {
        type = HID_INPUT::HID_BTN_6;
    }
    else if (type_str == "BTN_7")
    {
        type = HID_INPUT::HID_BTN_7;
    }
    else if (type_str == "BTN_8")
    {
        type = HID_INPUT::HID_BTN_8;
    }
    else if (type_str == "BTN_9")
    {
        type = HID_INPUT::HID_BTN_9;
    }
    else
    {
        LOG_ERROR("Invalid input type: {}", type_str);
        type = HID_INPUT::HID_AXIS_X;
        return -1;
    }
    return 0;
}

ControllerHID::ControllerHID(InitArgs* args) : Controller(args)
{
    if (args && args->properties)
    {
        if (args->properties->ValueExists("steeringRate"))
        {
            steering_rate_ = strtod(args->properties->GetValueStr("steeringRate"));
        }

        if (args->properties->ValueExists("deviceID"))
        {
            device_id_ = strtoi(args->properties->GetValueStr("deviceID"));
        }

        if (args->properties->ValueExists("steeringInput"))
        {
            std::string input_str = args->properties->GetValueStr("steeringInput");
            if (ParseHIDInputType(input_str, steering_input_) != 0)
            {
                LOG_ERROR_AND_QUIT("Failed to initialize HID controller {} reading steeringInput", GetName());
            }

            if (steering_input_ >= HID_INPUT::HID_BTN_1)
            {
                LOG_ERROR_AND_QUIT("Steering on button ({}) not supported. Must be on an axis.", steering_input_ - HID_INPUT::HID_BTN_1 + 1);
            }
        }

        if (args->properties->ValueExists("throttleInput"))
        {
            std::string input_str = args->properties->GetValueStr("throttleInput");
            if (ParseHIDInputType(input_str, throttle_input_) != 0)
            {
                LOG_ERROR_AND_QUIT("Failed to initialize HID controller {} reading throttleInput", GetName());
            }
        }

        if (args->properties->ValueExists("brakeInput"))
        {
            std::string input_str = args->properties->GetValueStr("brakeInput");
            if (ParseHIDInputType(input_str, brake_input_) != 0)
            {
                LOG_ERROR_AND_QUIT("Failed to initialize HID controller {} reading brakeInput", GetName());
            }
        }
        else
        {
            // brake input not specified, assume same as
            if (throttle_input_ < HID_INPUT::HID_BTN_1)
            {
                // throttle is an axis, use it also as brake input
                brake_input_ = throttle_input_;
            }
            else
            {
                // throttle is a button, can't combine
                LOG_ERROR_AND_QUIT("Throttle input is a button ({}), can only combine throttle and brake on axis. Add brake on separate button.",
                                   throttle_input_ - HID_INPUT::HID_BTN_1 + 1);
            }
        }
    }

    align_to_road_heading_on_deactivation_ = true;
    align_to_road_heading_on_activation_   = true;
}

ControllerHID::~ControllerHID()
{
    CloseHID();
}

void ControllerHID::Init()
{
    OpenHID(device_id_);
    Controller::Init();
}

void ControllerHID::Step(double timeStep)
{
    int return_val = ReadHID(throttle_, steering_);

    if (return_val < 0)
    {
        return;
    }
    else if (return_val > 0)
    {
        LOG_DEBUG("X {} Y {} Z {} RX {} RY {} RZ {} BTN {},{},{},{},{},{},{},{},{} steering: {:.2f} throttle: {:.2f} (events: {})",
                  values_[HID_AXIS_X],
                  values_[HID_AXIS_Y],
                  values_[HID_AXIS_Z],
                  values_[HID_AXIS_RX],
                  values_[HID_AXIS_RY],
                  values_[HID_AXIS_RZ],
                  values_[HID_BTN_1],
                  values_[HID_BTN_2],
                  values_[HID_BTN_3],
                  values_[HID_BTN_4],
                  values_[HID_BTN_5],
                  values_[HID_BTN_6],
                  values_[HID_BTN_7],
                  values_[HID_BTN_8],
                  values_[HID_BTN_9],
                  steering_,
                  throttle_,
                  return_val);
    }

    vehicle_.SetMaxSpeed(object_->GetMaxSpeed());

    // Update vehicle motion
    vehicle_.DrivingControlAnalog(timeStep, throttle_, steering_);

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

    object_->SetJunctionSelectorStrategy(roadmanager::Junction::JunctionStrategyType::SELECTOR_ANGLE);
    object_->SetJunctionSelectorAngle(0.0);

    for (auto& v : values_)
        v = 0;

    return Controller::Activate(lat_activation_mode, long_activation_mode, light_activation_mode, anim_activation_mode);
}

void ControllerHID::Deactivate()
{
    CloseHID();
    Controller::Deactivate();
}

#ifdef _WIN32

int ControllerHID::OpenHID(int device_id)
{
    UINT numDevs = joyGetNumDevs();
    if (numDevs == 0)
    {
        LOG_ERROR("No joystick devices available");
        return -1;
    }

    joy_info_         = {};
    joy_info_.dwSize  = sizeof(JOYINFOEX);
    joy_info_.dwFlags = JOY_RETURNALL;

    device_id_internal_ = JOYSTICKID1 + device_id;

    MMRESULT res = joyGetPosEx(static_cast<unsigned int>(JOYSTICKID1 + device_id), &joy_info_);
    if (res != JOYERR_NOERROR)
    {
        LOG_ERROR("Joystick with device id {} not ready or not connected", device_id_internal_);
        device_id_internal_ = -1;  // Reset fd
        return -1;
    }
    else
    {
        LOG_INFO("Opened joystick device id {}", device_id_internal_);
    }

    return 0;
}

int ControllerHID::ReadHID(double& throttle, double& steering)
{
    if (device_id_internal_ < 0)
    {
        return -1;  // Device not opened or invalid
    }

    MMRESULT res = joyGetPosEx(device_id_internal_, &joy_info_);
    if (res == JOYERR_NOERROR)
    {
        // register axes
        values_[HID_INPUT::HID_AXIS_X]  = joy_info_.dwXpos;
        values_[HID_INPUT::HID_AXIS_Y]  = joy_info_.dwYpos;
        values_[HID_INPUT::HID_AXIS_Z]  = joy_info_.dwZpos;
        values_[HID_INPUT::HID_AXIS_RX] = joy_info_.dwVpos;
        values_[HID_INPUT::HID_AXIS_RY] = joy_info_.dwUpos;
        values_[HID_INPUT::HID_AXIS_RZ] = joy_info_.dwRpos;

        // register buttons
        for (unsigned int i = 0; i < 9; i++)
        {
            values_[HID_INPUT::HID_BTN_1 + i] = joy_info_.dwButtons & (1 << i) ? 1 : 0;
        }

        if (throttle_input_ == brake_input_)
        {
            // throttle and brake on same axis, normalize to [-1, 1] and adjust sign for correct motion direction
            throttle = -(values_[throttle_input_] - 32767) / 32767.0;
        }
        else
        {
            double throttle_tmp = static_cast<double>(values_[throttle_input_]);
            double brake_tmp    = static_cast<double>(values_[brake_input_]);

            if (throttle_input_ < HID_INPUT::HID_BTN_1)
            {
                // throttle is an axis, scale to [0:1]
                throttle_tmp /= 65536.0;
            }

            if (brake_input_ < HID_INPUT::HID_BTN_1)
            {
                // brake is an axis, scale to [0:1]
                brake_tmp /= 65536.0;
            }

            // combine throttle and brake input into throttle [-1:1]
            throttle = throttle_tmp - brake_tmp;
        }

        steering = -(values_[steering_input_] - 32767) / 32767.0;  // Normalize to [-1, 1] and adjust sign for correct steering angle
    }
    else
    {
        LOG_ERROR("Error reading joystick data: %d", res);
        return -1;
    }

    return 1;
}

void ControllerHID::CloseHID()
{
}

#elif defined(__linux__)

int ControllerHID::OpenHID(int device_id)
{
    std::string joystick_path;

    joystick_path = "/dev/input/js" + std::to_string(device_id);

    // Try to open the device file
    device_id_internal_ = open(joystick_path.c_str(), O_RDONLY | O_NONBLOCK);
    if (device_id_internal_ < 0)
    {
        LOG_ERROR("Joystick with device id {} not ready or not connected", device_id);
        return -1;
    }

    // Device opened successfully. Now query its name.
    char name[128];
    if (ioctl(device_id_internal_, JSIOCGNAME(sizeof(name)), name) < 0)
    {
        // Fallback if name cannot be read (e.g., older kernel or device)
        snprintf(name, sizeof(name), "Unknown Joystick %d", device_id);
    }

    int num_axes = 0;
    ioctl(device_id_internal_, JSIOCGAXES, &num_axes);

    int num_buttons = 0;
    ioctl(device_id_internal_, JSIOCGBUTTONS, &num_buttons);

    if (num_axes > 0 || num_buttons > 0)
    {
        LOG_INFO("Opened {} Axis: {} Buttons: {}", joystick_path + " " + name, num_axes, num_buttons);
    }
    else
    {
        LOG_ERROR("Skipping {} (no axes/buttons detected).", joystick_path + " " + name);
        close(device_id_internal_);  // Close this non-joystick device
        device_id_internal_ = -1;    // Reset fd
        return -1;
    }

    return 0;
}

int ControllerHID::ReadHID(double& throttle, double& steering)
{
    if (device_id_internal_ < 0)
    {
        return -1;  // Device not opened or invalid
    }

    struct js_event js_event;

    // To avoid getting stuck reading infitite stream of input set a limit for number of read loops
    // ensuring progress. An alternative would be, if needed, to put js reading in a separate thread.
    unsigned int max_reads = 16;
    unsigned int i         = 0;
    for (; i < max_reads; i++)
    {
        if (read(device_id_internal_, &js_event, sizeof(struct js_event)) != static_cast<ssize_t>(sizeof(struct js_event)))
        {
            break;
        }

        switch (js_event.type & ~JS_EVENT_INIT)
        {
            case JS_EVENT_BUTTON:
                values_[HID_BTN_1 + js_event.number - 1] = js_event.value;
                break;
            case JS_EVENT_AXIS:
                values_[js_event.number] = js_event.value;
                break;
        }
    }

    if (i == 0)
    {
        // no input to process
        return 0;
    }

    if (throttle_input_ == brake_input_)
    {
        // throttle and brake on same axis
        throttle = static_cast<double>(-values_[throttle_input_]) / 32767.0;  // Normalize to [-1, 1] and adjust sign for correct motion direction
    }
    else
    {
        double throttle_tmp = static_cast<double>(values_[throttle_input_]);
        double brake_tmp    = static_cast<double>(values_[brake_input_]);

        if (throttle_input_ < HID_INPUT::HID_BTN_1)
        {
            // throttle is an axis, scale to [0:1]
            throttle_tmp /= 65536.0;
        }

        if (brake_input_ < HID_INPUT::HID_BTN_1)
        {
            // brake is an axis, scale to [0:1]
            brake_tmp /= 65536.0;
        }

        // combine throttle and brake input into throttle [-1:1]
        throttle = throttle_tmp - brake_tmp;
    }

    steering = static_cast<double>(-values_[steering_input_]) / 32767.0;  // Scale to [0:1] and adjust sign for correct steering angle

    return static_cast<int>(i);
}

void ControllerHID::CloseHID()
{
    close(device_id_internal_);
}

#else  // unsupported platform, e.g. macOS

int ControllerHID::OpenHID(int device_id)
{
    return -1;
}

int ControllerHID::ReadHID(double& throttle, double& steering)
{
    return -1;
}

void ControllerHID::CloseHID()
{

}

#endif
