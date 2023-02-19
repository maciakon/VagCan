#include "Arduino.h" 
#include <mcp_can.h>

#define CAN0_INT 4          /* Set INT to pin 4 (This rarely changes)  */
#define LISTEN_ID 0x7EA
#define REPLY_ID 0x7E0
#define FUNCTIONAL_ID 0x7DF 

class VagCanMCP 
{
    public: 
        VagCanMCP(int pin);
        void sendPID(unsigned char pid);
        byte* receivePID();
        char msgString[128];
        void initCan();
    private:
        unsigned long rxID;
        byte dlc;
        byte rxBuf[8];
        MCP_CAN CAN0;
};
