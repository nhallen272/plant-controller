// state_control.h
#ifndef State_control_h
#define State_control_h
#include "control.h"



// **** Eventually add a State type with bool enabled, with TriState and OutputType as composition members

class StateController
{   
public:
    StateController();
    void updateLights(TriState newState);
    void updateDehumidifier(TriState newState);
    TriState getState(OutputType t);
    void loop(); // send signals

private: 
    TriState lights;
    TriState dehumidifier;
};

#endif