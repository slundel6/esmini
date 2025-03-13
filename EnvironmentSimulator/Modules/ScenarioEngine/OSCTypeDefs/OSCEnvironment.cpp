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

std::optional<double> OSCEnvironment::GetAtmosphericPressure() const
{
    return atmosphericpressure_;
}

bool OSCEnvironment::IsAtmosphericPressureSet() const
{
    return atmosphericpressure_.has_value();
}

void OSCEnvironment::SetTemperature(double temperature)
{
    temperature_ = temperature;
}

std::optional<double> OSCEnvironment::GetTemperature() const
{
    return temperature_;
}

bool OSCEnvironment::IsTemperatureSet() const
{
    return temperature_.has_value();
}

void OSCEnvironment::SetCloudState(CloudState cloudstate)
{
    cloudstate_ = cloudstate;
}

std::optional<CloudState> scenarioengine::OSCEnvironment::GetCloudState() const
{
    return cloudstate_;
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

std::optional<Fog> OSCEnvironment::GetFog() const
{
    return fog_;
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

std::optional<Wind> OSCEnvironment::GetWind() const
{
    return wind_;
}

bool OSCEnvironment::IsWindSet() const
{
    return wind_.has_value();
}

void OSCEnvironment::SetPrecipitation(const Precipitation& precipitation)
{
    precipitation_ = precipitation;
}

std::optional<Precipitation> OSCEnvironment::GetPrecipitation() const
{
    return precipitation_;
}

bool OSCEnvironment::IsPrecipitationSet() const
{
    return precipitation_.has_value();
}

void OSCEnvironment::SetSun(const Sun& sun)
{
    sun_ = sun;
}

std::optional<Sun> OSCEnvironment::GetSun() const
{
    return sun_;
}

bool OSCEnvironment::IsSunSet() const
{
    return sun_.has_value();
}

void OSCEnvironment::SetTimeOfDay(const TimeOfDay& timeofday)
{
    timeofday_ = timeofday;
}

std::optional<TimeOfDay> OSCEnvironment::GetTimeOfDay() const
{
    return timeofday_;
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

std::optional<RoadCondition> OSCEnvironment::GetRoadCondition() const
{
    return roadcondition_;
}

bool OSCEnvironment::IsRoadConditionSet() const
{
    return roadcondition_.has_value();
}

void OSCEnvironment::UpdateEnvironment(const OSCEnvironment& new_environment)
{
    if (new_environment.IsAtmosphericPressureSet())
    {
        SetAtmosphericPressure(new_environment.GetAtmosphericPressure().value());
    }
    if (new_environment.IsTemperatureSet())
    {
        SetTemperature(new_environment.GetTemperature().value());
    }
    if (new_environment.IsCloudStateSet())
    {
        SetCloudState(new_environment.GetCloudState().value());
    }
    if (new_environment.IsFogSet())
    {
        SetFog(new_environment.GetFog().value());
    }
    if (new_environment.IsWindSet())
    {
        SetWind(new_environment.GetWind().value());
    }
    if (new_environment.IsPrecipitationSet())
    {
        SetPrecipitation(new_environment.GetPrecipitation().value());
    }
    if (new_environment.IsSunSet())
    {
        SetSun(new_environment.GetSun().value());
    }
    if (new_environment.IsRoadConditionSet())
    {
        SetRoadCondition(new_environment.GetRoadCondition().value());
    }
    if (new_environment.IsTimeOfDaySet())
    {
        SetTimeOfDay(new_environment.GetTimeOfDay().value());
    }
}

bool OSCEnvironment::IsEnvironment() const
{
    return IsTemperatureSet() || IsAtmosphericPressureSet() || IsCloudStateSet() || IsFogSet() || IsWindSet() || IsPrecipitationSet() || IsSunSet() ||
           IsRoadConditionSet() || IsTimeOfDaySet();
}
