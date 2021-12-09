/* state_processor.cpp
 * Nathan Allen 7/31/2021
 * Implementation for the state preprocessor
*/
#include "state_processor.h"

StateProcessor::StateProcessor(float h)
{
    setHumidity = h;
    // Create initial schedule tuple with default values (6, 24)
    lightSchedule = std::make_tuple(LIGHT_ON, LIGHT_OFF);
}

// Post: set current hr/min. Then set initial times to startup the pumps.
void StateProcessor::init(int hr, int min)
{
    currHr = hr; 
    currMin = min;

}
void StateProcessor::setHum(int h)
{
    setHumidity = h;
}

// Set the lighting schedule
void StateProcessor::setSchedule(int onHour, int offHour)
{
    lightSchedule = std::make_tuple(onHour, offHour);
}

// Post: update the current time
void StateProcessor::updateTime(int hr, int min)
{
    currHr = hr;
    currMin = min;
}

// Determine state of the dehumidifier
TriState StateProcessor::humidistat(float currHumidity) 
{
    if (currHumidity >= setHumidity)
    {
        return TriState::ON; 
    }
    else
    {
        return TriState::OFF;
    }

}
// determine state of lighting
TriState StateProcessor::handleLights()       
{   
    // ON == (currHr >= onHr and currHr < offHr)
    if ((currHr >= std::get<0>(lightSchedule)) && (currHr < std::get<1>(lightSchedule))) 
    {
        return TriState::ON;
    }
    else
    {
        return TriState::OFF;
    }
}
