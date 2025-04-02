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
    if (atmosphericpressure < OSCAtmosphericMin || atmosphericpressure > OSCAtmosphericMax)
    {
        LOG_WARN("Atmospheric pressure clamped between 80000.0 and 120000.0 Pa");
    }
    // Clamp the atmospheric pressure to the defined range
    atmosphericpressure_ = CLAMP(atmosphericpressure, OSCAtmosphericMin, OSCAtmosphericMax);
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
    if (temperature < OSCTemperatureMin || temperature > OSCTemperatureMax)
    {
        LOG_WARN("Temperature clamped between 170.0 and 340.0 K");
    }
    // Clamp the temperature to the defined range
    temperature_ = CLAMP(temperature, OSCTemperatureMin, OSCTemperatureMax);
}

double OSCEnvironment::GetTemperature() const
{
    return temperature_.value();
}

bool OSCEnvironment::IsTemperatureSet() const
{
    return temperature_.has_value();
}

void OSCEnvironment::SetFractionalCloudState(const std::string& fractionalcloudStateStr)
{
    fractionalcloudstate_ = fractionalcloudStateStr;
}

void OSCEnvironment::SetCloudState(CloudState cloudstate)
{
    std::map<CloudState, std::string> stateMap{
        {CloudState::CLOUDLESS, "zeroOktas"},
        {CloudState::SUNNY, "oneOktas"},
        {CloudState::SERENE, "twoOktas"},
        {CloudState::SLIGHTLY_CLOUDY, "threeOktas"},
        {CloudState::LIGHT_CLOUDY, "fourOktas"},
        {CloudState::CLOUDY, "fiveOktas"},
        {CloudState::HEAVILY_CLOUDY, "sixOktas"},
        {CloudState::ALMOST_OVERCAST, "sevenOktas"},
        {CloudState::OVERCAST, "eightOktas"},
        {CloudState::SKY_NOT_VISIBLE, "nineOktas"},
    };
    auto it = stateMap.find(cloudstate);
    if (it == stateMap.end())
    {
        fractionalcloudstate_ = "other";
    }
    else
    {
        fractionalcloudstate_ = stateMap[cloudstate];
    }
}

std::string scenarioengine::OSCEnvironment::GetFractionalCloudState() const
{
    return fractionalcloudstate_.value();
}

bool OSCEnvironment::IsFractionalCloudStateSet() const
{
    return fractionalcloudstate_.has_value();
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

bool scenarioengine::OSCEnvironment::IsWeatherSet() const
{
    return IsTemperatureSet() || IsAtmosphericPressureSet() || IsFractionalCloudStateSet() || IsFogSet() || IsWindSet() || IsPrecipitationSet() ||
           IsSunSet();
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
    LOG_INFO("Setting sun elevation: " + std::to_string(sun.elevation) + " azimuth: " + std::to_string(sun.azimuth) +
             " intensity: " + std::to_string(sun.intensity));
    if (sun.elevation < OSCSunElevationMin || sun.elevation > OSCSunElevationMax)
    {
        LOG_WARN("Sun elevation clamped between -90.0 and 90.0 degrees");
    }
    if (sun.azimuth < OSCSunAzimuthMin || sun.azimuth > OSCSunAzimuthMax)
    {
        LOG_WARN("Sun azimuth clamped between 0.0 and 360.0 degrees");
    }
    Sun sunNew;
    // Clamp the sun elevation and azimuth to the defined ranges
    sunNew.elevation = CLAMP(sun.elevation, OSCSunElevationMin, OSCSunElevationMax);
    sunNew.azimuth   = CLAMP(sun.azimuth, OSCSunAzimuthMin, OSCSunAzimuthMax);
    sunNew.intensity = sun.intensity;
    LOG_INFO("Setting clamped sun elevation: " + std::to_string(sunNew.elevation) + " azimuth: " + std::to_string(sunNew.azimuth) +
             " intensity: " + std::to_string(sunNew.intensity));
    sun_ = sunNew;
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
    if (new_environment.IsFractionalCloudStateSet())
    {
        SetFractionalCloudState(new_environment.GetFractionalCloudState());
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
    return IsTemperatureSet() || IsAtmosphericPressureSet() || IsFractionalCloudStateSet() || IsFogSet() || IsWindSet() || IsPrecipitationSet() ||
           IsSunSet() || IsRoadConditionSet() || IsTimeOfDaySet();
}
