#include <mcp_can.h>
#include <SPI.h>

// 7E0/8 = Engine ECM
// 7E1/9 = Transmission ECM

  #define LISTEN_ID 0x7EA
  #define REPLY_ID 0x7E0
  #define FUNCTIONAL_ID 0x7DF

// CAN TX Variables
unsigned long prevTx = 0;
unsigned int invlTx = 1000;
byte txData[] = {0x02,0x01,0x00,0x55,0x55,0x55,0x55,0x55}; // AVAILABLE PIDS
// byte txData[] = {0x02,0x01,0x0D,0x55,0x55,0x55,0x55,0x55}; // CAR SPEED

// CAN RX Variables
unsigned long rxID;
byte dlc;
byte rxBuf[8];
char msgString[128];                        // Array to store serial string

// CAN Interrupt and Chip Select Pins
#define CAN0_INT 2                              /* Set INT to pin 2 (This rarely changes)   */
MCP_CAN CAN0(15);                                /* Set CS to pin 9 (Old shields use pin 10) */


void setup(){

  Serial.begin(115200);
  while(!Serial);
 
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_NORMAL, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else{
    Serial.println("Error Initializing MCP2515... Permanent failure!  Check your code & connections");
    while(1);
  }

  // Standard ID Filters
  CAN0.init_Mask(0,0x7F00000);                // Init first mask...
  CAN0.init_Filt(0,0x7DF0000);                // Init first filter...
  CAN0.init_Filt(1,0x7E10000);                // Init second filter...

  CAN0.init_Mask(1,0x7F00000);                // Init second mask...
  CAN0.init_Filt(2,0x7DF0000);                // Init third filter...
  CAN0.init_Filt(3,0x7E10000);                // Init fouth filter...
  CAN0.init_Filt(4,0x7DF0000);                // Init fifth filter...
  CAN0.init_Filt(5,0x7E10000);                // Init sixth filter...

  CAN0.setMode(MCP_NORMAL);                      // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input
 
  Serial.println("Simple CAN OBD-II PID Request");
}

void loop(){

  if(!digitalRead(CAN0_INT)){                         // If CAN0_INT pin is low, read receive buffer
 
    CAN0.readMsgBuf(&rxID, &dlc, rxBuf);             // Get CAN data
  
    // Display received CAN data as we receive it.
    if((rxID & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxID & 0x1FFFFFFF), dlc);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxID, dlc);
 
    Serial.print(msgString);
 
    if((rxID & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<dlc; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
      
    Serial.println();
  }
 
  /* Every 1000ms (One Second) send a request for PID 00           *
   * This PID responds back with 4 data bytes indicating the PIDs  *
   * between 0x01 and 0x20 that are supported by the vehicle.      */
  if((millis() - prevTx) >= invlTx){
    prevTx = millis();
    INT8U result = CAN0.sendMsgBuf(FUNCTIONAL_ID, 8, txData);
    if( result == CAN_OK){
      Serial.println("Message Sent Successfully!");
    } else {
      Serial.println("Error Sending Message...");
      Serial.println(result);
    }
  }
}
