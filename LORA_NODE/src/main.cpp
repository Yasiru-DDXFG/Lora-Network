//#define SAMD 1
//#define MEGA 1


#include <LoRa.h>
#include "NODE.h"



#ifdef SAMD
  //samd cs,rst,irq
  NODE mynode(28, 26, 27, gaterxFreq, gatetxFreq);
#elif MEGA
  //mega cs,rst,irq
  NODE mynode(43, 42, 2, gaterxFreq, gatetxFreq);
#endif







void callbackfunc(int packetsize)
{
  mynode.callBack(packetsize);
}

void readIO(IO_Variable *IO)
{
  // Implement IO read mechansim
  //example
  //IO->b |= (1<<<digitalRead(some_pin))  /////////////  implement this
  if (Serial.available()>0)
  {
    char x = Serial.read();
    if (x == '#')
    {
      IO->iovar = Serial.parseInt();
    }
  }
}

/*void myints(){
  Serial.println("Interrupt!");
}*/

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  debug_print("LoRa NODE");
  mynode.Init();
  mynode.setID(9);
  LoRa.onReceive(callbackfunc);
  LoRa.dumpRegisters(Serial);
  
 //attachInterrupt(27,myints,FALLING);
}

void loop()
{ 
  mynode.checkStatus();
}
