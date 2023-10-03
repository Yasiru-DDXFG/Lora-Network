#include <LoRa.h>

#define DEBUG_PRINT
#define gatetxFreq 433E6
#define gaterxFreq 433.5E6
#define PRIVATEKEY 0xB5
#define OFFSET_ID 0x12;

#ifdef SAMD
  #define Serial SerialUSB
#elif MEGA
  #define Serial Serial
#endif




#ifdef DEBUG_PRINT
  #define debug_print(str) Serial.print(str)
  #define debug_println(str) Serial.println(str)
#else
  #define debug_print(str)
  #define debug_println(str)
#endif








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
  byte iovar;
} IO_Variable;


typedef struct
{
  uint16_t sourceID;
  uint16_t destinationtID;
  uint8_t command;
  IO_Variable data;
} Packet;

class NODE
{
private:
  int csPin;    // LoRa radio chip select
  int resetPin; // LoRa radio reset
  int irqPin;   // change for your board; must be a hardware interrupt pin
  Packet datapacket; // Container to hold tx and rx payload
  volatile uint8_t STATE = POWER_ON;
  IO_Variable lastIOstate;
  IO_Variable currentIOstate;
  Packet *packet_ptr;
  uint8_t *data;
  uint16_t MY_ID = 0xFFF4; // Source ID
  long txFrequency, rxFrequency;
  void pushPacket();
  bool randomElapsedWithin(unsigned long from_ms, unsigned long to_ms);
public: 
  bool stateChanged = false;
  NODE(int csPin, int resetpin, int irqpin, long txfreq, long rxfreq);
  void setID(uint16_t ID);
  void Init();
  void callBack(int packetSize);
  void checkStatus();
  ~NODE();
};

void readIO(IO_Variable *IO);
void callbackfunc(int packetsize);