/* control.cpp  
 * Nathan Allen 7/31/2021
 * Implementation for the StateController. 
 */
#include "state_control.h"


// Default constructor sets states to OFF initially
StateController::StateController()
{
    // Initially set all states to off.
    lights = TriState::OFF;
    dehumidifier = TriState::OFF;
}

void StateController::updateLights(TriState newState)
{
    if (lights == newState) // return if no change 
    {
        return;
    }
    else
    {
        Serial.println("received state change for lights");
        lights = newState;
        switch (lights)
        {
            case TriState::ON :
            {
                // Send ON to relay
                Serial.println("Lights: ON");
                digitalWrite(int(OutputType::Lights), HIGH); // Normally Closed config (HIGH == ON, LOW == OFF)
                break;
            }
            case TriState::OFF :
            {
                // Send OFF to relay
                Serial.println("Lights: OFF");
                digitalWrite(int(OutputType::Lights), LOW);  
                break;
            }
            case TriState::IDLE : 
            default : 
            {
                // Do nothing
                break;
            }
        }
    }

}
void StateController::updateDehumidifier(TriState newState)
{
    if (dehumidifier == newState) // return if no change 
    {
        return;
    }
    else
    {
        Serial.println("received state change for dehumidifier");
        dehumidifier = newState;
        switch (dehumidifier)
        {
            case TriState::ON :
            {
                // Send to relay
                Serial.println("Dehumidifier: ON");
                digitalWrite(int(OutputType::Dehumidifier), LOW); // Normally Open config (HIGH == OFF, LOW == ON)
                break;
            }
            case TriState::OFF :
            {
                Serial.println("Dehumidifier: OFF");
                digitalWrite(int(OutputType::Dehumidifier), HIGH);  // Normally Open config
                break;
            }
            case TriState::IDLE : 
            default : 
            {
                break;
            }
        }
    }
}


// Post: return the state of OutputType t
TriState StateController::getState(OutputType t) 
{
    switch (t)
    {
        case OutputType::Lights :
        {
            return lights; // return current TriState of the output
        }
        case OutputType::Dehumidifier :
        {
            return dehumidifier;
        }
        default : 
        {
            return TriState::OFF;
        }
    }
}

void StateController::loop()
{
     switch (dehumidifier)
        {
            case TriState::ON :
            {
                // Send to relay
                Serial.println("Dehumidifier: ON");
                digitalWrite(int(OutputType::Dehumidifier), LOW); // Normally Open config (HIGH == OFF, LOW == ON)
                break;
            }
            case TriState::OFF :
            {
                Serial.println("Dehumidifier: OFF");
                digitalWrite(int(OutputType::Dehumidifier), HIGH);  // Normally Open config
                break;
            }
            case TriState::IDLE : 
            default : 
            {
                break;
            }
        }
      switch (lights)
        {
            case TriState::ON :
            {
                // Send ON to relay
                Serial.println("Lights: ON");
                digitalWrite(int(OutputType::Lights), LOW); // Normally Closed config (HIGH == ON, LOW == OFF)
                break;
            }
            case TriState::OFF :
            {
                // Send OFF to relay
                Serial.println("Lights: OFF");
                digitalWrite(int(OutputType::Lights), HIGH);  
                break;
            }
            case TriState::IDLE : 
            default : 
            {
                // Do nothing
                break;
            }
        }

}
