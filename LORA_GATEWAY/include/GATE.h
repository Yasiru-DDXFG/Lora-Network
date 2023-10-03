#include "Arduino.h"

#define DEBUG_PRINT
#define gatetxFreq 433E6
#define gaterxFreq 433.5E6
#define PRIVATEKEY 0xB5
#define OFFSET_ID 0x12;
#define maxDevicePollTime 5000 // -------------------->  TIMEOUT for polling devices
#define maxLastSeen 10000     //900000L    // -------------------->  15 minutes  =  15 * 60 * 1000 ms   last seen allowed

#define LoRaRX LoRa

#ifdef DEBUG_PRINT
#define PRINT(str) Serial.println(str)
#define PRINTINLINE(str) Serial.print(str)
#else
#define PRINT(str)
#endif

#define LoratxCS 32
#define LoratxRST 31
#define LoratxIRQ 3

////////////////  SPI 2 interface pins

#define LorarxCS 43  //pa18
#define LorarxRST 42 //pa27
#define LorarxIRQ 2  //pb3
#define LorarxSERCOM sercom1
#define LorarxMISOpin 12 //pa19
#define LorarxCLKpin 13  //pa17
#define LorarxMOSIpin 11 //pa16

extern LoRaClass LoRaTX;

typedef struct
{
  byte b;
} IO_Variable;

typedef struct
{
  uint16_t sourceID;
  uint16_t destinationtID;
  uint8_t command;
  IO_Variable iovar;
} Packet;

enum States
{
  POWER_ON,
  IDLE,
  WAITFORACK,
  SOMETHING_HAPPENED
};

enum Commands
{
  AREYOUREADY = 69,
  STATUS_PUSH,
  ACK,
  REGISTER_ME,
  REGISTER_ACK
};

typedef struct
{
  uint16_t DEV_ID;
  volatile bool connected;
  IO_Variable iostate;
  IO_Variable lastiostate;
  unsigned long lastseen;
} Device;

void initdevIDs();
void pushPacket();
void checkDeviceConnectivity();
void callback(int size);
void checkDeviceIOchanges();
void sendAckstoResponders(uint16_t ID);