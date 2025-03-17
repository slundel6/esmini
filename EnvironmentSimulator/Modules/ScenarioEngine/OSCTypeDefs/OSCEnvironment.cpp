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

#include "OSCEnvironment.hpp"

using namespace scenarioengine;

void OSCEnvironment::SetAtmosphericPressure(double atmosphericpressure)
{
    atmosphericpressure_ = atmosphericpressure;
}

double OSCEnvironment::GetAtmosphericPressure() const
{
    return atmosphericpressure_.value();
}

bool OSCEnvironment::IsAtmosphericPressureSet() const
{
    return atmosphericpressure_.has_value();
}

void OSCEnvironment::SetTemperature(double temperature)
{
    temperature_ = temperature;
}

double OSCEnvironment::GetTemperature() const
{
    return temperature_.value();
}

bool OSCEnvironment::IsTemperatureSet() const
{
    return temperature_.has_value();
}

void OSCEnvironment::SetCloudState(CloudState cloudstate)
{
    cloudstate_ = cloudstate;
}

CloudState scenarioengine::OSCEnvironment::GetCloudState() const
{
    return cloudstate_.value();
}

bool OSCEnvironment::IsCloudStateSet() const
{
    return cloudstate_.has_value();
}

void OSCEnvironment::SetFog(const Fog& fog)
{
    fog_ = fog;
}

void OSCEnvironment::SetFog(const float visualrange)
{
    fog_ = Fog{visualrange, std::nullopt};
}

Fog OSCEnvironment::GetFog() const
{
    return fog_.value();
}

bool OSCEnvironment::IsFogSet() const
{
    return fog_.has_value();
}

bool scenarioengine::OSCEnvironment::IsFogBoundingBoxSet() const
{
    return fog_.has_value() && fog_->boundingbox.has_value();
}

void OSCEnvironment::SetWind(const Wind& wind)
{
    wind_ = wind;
}

Wind OSCEnvironment::GetWind() const
{
    return wind_.value();
}

bool OSCEnvironment::IsWindSet() const
{
    return wind_.has_value();
}

void OSCEnvironment::SetPrecipitation(const Precipitation& precipitation)
{
    precipitation_ = precipitation;
}

Precipitation OSCEnvironment::GetPrecipitation() const
{
    return precipitation_.value();
}

bool OSCEnvironment::IsPrecipitationSet() const
{
    return precipitation_.has_value();
}

bool scenarioengine::OSCEnvironment::IsPrecipitationIntensitySet() const
{
    return IsPrecipitationSet() && GetPrecipitation().precipitationintensity.has_value();
}

double scenarioengine::OSCEnvironment::GetPrecipitationIntensity() const
{
    return GetPrecipitation().precipitationintensity.value();
}

void OSCEnvironment::SetSun(const Sun& sun)
{
    sun_ = sun;
}

Sun OSCEnvironment::GetSun() const
{
    return sun_.value();
}

bool OSCEnvironment::IsSunSet() const
{
    return sun_.has_value();
}

void OSCEnvironment::SetTimeOfDay(const TimeOfDay& timeofday)
{
    timeofday_ = timeofday;
}

TimeOfDay OSCEnvironment::GetTimeOfDay() const
{
    return timeofday_.value();
}

bool OSCEnvironment::IsTimeOfDaySet() const
{
    return timeofday_.has_value();
}

void OSCEnvironment::SetRoadCondition(const RoadCondition& roadcondition)
{
    roadcondition_ = roadcondition;
}

void OSCEnvironment::SetRoadCondition(const double friction)
{
    roadcondition_ = RoadCondition{friction, std::nullopt, std::nullopt};
}

RoadCondition OSCEnvironment::GetRoadCondition() const
{
    return roadcondition_.value();
}

bool OSCEnvironment::IsRoadConditionSet() const
{
    return roadcondition_.has_value();
}

void OSCEnvironment::UpdateEnvironment(const OSCEnvironment& new_environment)
{
    if (new_environment.IsAtmosphericPressureSet())
    {
        SetAtmosphericPressure(new_environment.GetAtmosphericPressure());
    }
    if (new_environment.IsTemperatureSet())
    {
        SetTemperature(new_environment.GetTemperature());
    }
    if (new_environment.IsCloudStateSet())
    {
        SetCloudState(new_environment.GetCloudState());
    }
    if (new_environment.IsFogSet())
    {
        SetFog(new_environment.GetFog());
    }
    if (new_environment.IsWindSet())
    {
        SetWind(new_environment.GetWind());
    }
    if (new_environment.IsPrecipitationSet())
    {
        SetPrecipitation(new_environment.GetPrecipitation());
    }
    if (new_environment.IsSunSet())
    {
        SetSun(new_environment.GetSun());
    }
    if (new_environment.IsRoadConditionSet())
    {
        SetRoadCondition(new_environment.GetRoadCondition());
    }
    if (new_environment.IsTimeOfDaySet())
    {
        SetTimeOfDay(new_environment.GetTimeOfDay());
    }
}

bool OSCEnvironment::IsEnvironment() const
{
    return IsTemperatureSet() || IsAtmosphericPressureSet() || IsCloudStateSet() || IsFogSet() || IsWindSet() || IsPrecipitationSet() || IsSunSet() ||
           IsRoadConditionSet() || IsTimeOfDaySet();
}
