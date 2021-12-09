/* state_processor.h
 * Nathan Allen 7/21/2021
 * Pre processor for the finite state machine. Contains logic for humidistat, schedules for pumps and lights.
 * Input: raw data from sensors, RPC from TB server or user input.
 * Passes output to the StateController instance.
*/
#ifndef State_Processor_h
#define State_Processor_h
#include <tuple> // for schedules
#include "control.h"


class StateProcessor 
{
public: 
    StateProcessor(float h);                                // init tuples, set temp
    void init(int hr, int min);
    void updateTime(int, int);                              // set the time from the NTP wrapper.
    void setSchedule(int, int);                             // set lighting schedule: onHr, offHr
    void setHum(int h);                                     // update the humidity setting
    TriState humidistat(float currHumidity);                // determine state of dehumidifier
    TriState handleLights();                                // determine state of lighting
    


private: 
    float setHumidity;                          // setting for humidistat functionality.
    int currHr;                                 // hold hr, min
    int currMin;
    std::tuple<int, int> lightSchedule;         // lighting schedule       (ON hr, OFF hr)


};

#endif