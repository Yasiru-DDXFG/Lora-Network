#include <Arduino.h>
#include "SPI.h"
#include "LoRa.h"
#include "GATE.h"

// FOR SAMD USE TWO SEPARATE SPI PORTS
// using lora2 for rx
//SPIClass SPI2(&LorarxSERCOM, LorarxMISOpin, LorarxCLKpin, LorarxMOSIpin, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3); //spi interface for lora2
void callb(int size)
{
  PRINT("recv");
}

void setup()
{
  //SerialUSB.begin(9600);
  Serial.begin(9600);
  delay(1000);
  PRINT("LoRa GATEWAY");

  // IF SAMD21G
  //LoRaRX.setSPI(SPI2);
  LoRaRX.setPins(LorarxCS, LorarxRST, LorarxIRQ); // define pins in gate.h
  LoRaTX.setPins(LoratxCS, LoratxRST, LoratxIRQ); // define pins in gate.h

  while (!LoRaTX.begin(gatetxFreq))
  {
    delay(1000);
    PRINT("Loratx init failed");
  };
  while (!LoRa.begin(gaterxFreq))
  {
    delay(1000);
    PRINT("Lorarx init failed");
  };

  //LoRaTX.dumpRegisters(Serial);
  //LoRaRX.dumpRegisters(Serial);
  //LoRaRX.onReceive(callb);
  LoRaRX.onReceive(callback);
  LoRaRX.receive();
  initdevIDs();
}

void loop()
{
  checkDeviceConnectivity();
  checkDeviceIOchanges();
  sendAckstoResponders(0);
}