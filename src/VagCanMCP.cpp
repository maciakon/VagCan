#include "VagCanMCP.h"

VagCanMCP::VagCanMCP(int pin) : CAN0(pin) { }

void VagCanMCP::initCan()
{
    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
    
    // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
    if (CAN0.begin(MCP_NORMAL, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
        Serial.println("MCP2515 Initialized Successfully!");
    else
    {
        Serial.println("Error Initializing MCP2515... Permanent failure!  Check your code & connections");
        while (1)
            ;
    }

    // Standard ID Filters
    CAN0.init_Mask(0, 0x7F00000); // Init first mask...
    CAN0.init_Filt(0, 0x7DF0000); // Init first filter...
    CAN0.init_Filt(1, 0x7E10000); // Init second filter...

    CAN0.init_Mask(1, 0x7F00000); // Init second mask...
    CAN0.init_Filt(2, 0x7DF0000); // Init third filter...
    CAN0.init_Filt(3, 0x7E10000); // Init fouth filter...
    CAN0.init_Filt(4, 0x7DF0000); // Init fifth filter...
    CAN0.init_Filt(5, 0x7E10000); // Init sixth filter...

    CAN0.setMode(MCP_LOOPBACK); // Set operation mode to normal so the MCP2515 sends acks to received data.
    pinMode(CAN0_INT, INPUT);
}

void VagCanMCP::sendPID(unsigned char pid)
{
    unsigned char txData[8] = {0x02, 0x01, pid, 0, 0, 0, 0, 0}; // AVAILABLE PIDS

    byte sndStat = CAN0.sendMsgBuf(FUNCTIONAL_ID, 0, 8, txData);

    if (sndStat == CAN_OK)
    {
        Serial.print("PID sent: 0x");
        Serial.println(pid, HEX);
    }
    else
    {
        Serial.println("Error Sending Message...");
    }
};

byte *VagCanMCP::receivePID()
{
    CAN0.readMsgBuf(&rxID, &dlc, rxBuf); // Read data: len = data length, buf = data byte(s)

    // Display received CAN data as we receive it.
    sprintf(msgString, "Standard ID: 0x%.3lX, DLC: %1d, Data: ", rxID, dlc);
    Serial.print(msgString);

    for (byte i = 0; i < dlc; i++)
    {
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
    }
    Serial.println("");
    return rxBuf;
}