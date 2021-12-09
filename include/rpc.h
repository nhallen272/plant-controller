/* rpc_func.h  
 * rpc functions for the plant controller
 * Nathan Allen 08/04/2021
*/
#ifndef rpc_func_h
#define rpc_func_h
#include "control.h"

// Set the lighting schedule
RPC_Response processSetLights(const RPC_Data &data, const )
{
    int onHr = data["on"];
    int offHr = data["off"];
    

}

// Get current State of the lights
RPC_Response processGetLightStatus(const RPC_Data &data, const)
{

}

// Set the aeroponic schedule
RPC_Response processSetAero(const RPC_Data &data)
{

}

// RPC handlers
RPC_Callback callbacks[] = {
  { "setLights",         processSetLights },
  { "getValue",         processGetLightStatus },
  { "setGpioStatus",    processSetAero },
};