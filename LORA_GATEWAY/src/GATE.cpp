#include <Arduino.h>
#include "SPI.h"
#include "LoRa.h"
#include "GATE.h"

LoRaClass LoRaTX;
const int maxdevicenum = 10;
unsigned int devptr = 0;
Packet rxp, txp;
Packet *rxptr = &rxp;
Packet *txptr = &txp;
Device Devices[maxdevicenum];

void parseSerialCommands()
{
    /*static char x;
    if (Serial.available()>0)
    {
        x = Serial.read();
        if (x=='#')
        {
         int cmd = Serial.parseInt();   
        }
        
    }*/
}

void sendAckstoResponders(uint16_t devid)
{
    static uint16_t ackIDlist[50];
    static uint16_t ptr = 0;
    static bool avail = false;

    if (devid != 0)
    {
        ackIDlist[ptr] = devid;
        if (ptr <= 48)
        {
            ptr++;
            avail = true;
        }
    }
    else if (devid == 0)
    {
        if (ptr > 0)
        {
            ptr--;
        }
        txp.destinationtID = ackIDlist[ptr];
        txp.command = ACK;
        if (avail)
        {
            pushPacket();
            if (ptr == 0)
            {
                avail = false;
            }
        }
    }
}

void callback(int size)
{
    uint8_t *ptr = (uint8_t *)rxptr;
    LoRaRX.readBytes((uint8_t *)ptr, sizeof(Packet));
    for (unsigned long long i = 0; i < sizeof(Packet); i++)
    {
        ptr[i] ^= PRIVATEKEY;
    }
    PRINTINLINE("Recv! ----> Command:");
    if (rxp.command == STATUS_PUSH)
    {
        PRINTINLINE("Status push, from ID: ");
        PRINT(rxp.sourceID);
        sendAckstoResponders(rxp.sourceID);
    }

    for (int i = 0; i < maxdevicenum; i++)
    {
        if (Devices[i].DEV_ID == rxp.sourceID && rxp.command == STATUS_PUSH)
        {
            Devices[i].connected = true;
            Devices[i].iostate.b = rxp.iovar.b;
            Devices[i].lastseen = millis();
        }
    }
}

void pushPacket()
{
    LoRaRX.idle();
    LoRaTX.beginPacket();
    uint8_t *pt = (uint8_t *)txptr;
    for (unsigned long long i = 0; i < sizeof(Packet); i++)
    {
        LoRaTX.write(pt[i] ^ PRIVATEKEY);
    }
    //LoRaTX.write(pt, sizeof(Packet));
    LoRaTX.endPacket();
    LoRaRX.receive();
    PRINTINLINE("Sent packet! : ");
    if (txp.command == ACK)
    {
        PRINTINLINE("ack to:");
    }
    if (txp.command == AREYOUREADY)
    {
        PRINTINLINE("search to:");
    }
    PRINT(txp.destinationtID);
}

void initdevIDs()
{
    for (int i = 0; i < maxdevicenum; i++)
    {
        Devices[i].connected = false;
        Devices[i].DEV_ID = i + OFFSET_ID;
    }
}

bool timeout(unsigned long timeoutvalue)
{
    static unsigned long last = millis();
    bool timeout = false;
    if ((millis() - last) > timeoutvalue)
    {
        timeout = true;
        last = millis();
    }
    return timeout;
}

void checkDeviceConnectivity()
{
    static int i = 0;
    if (!Devices[i].connected)
    {
        txp.destinationtID = Devices[i].DEV_ID;
        txp.command = AREYOUREADY;
        pushPacket();
        while (!timeout(maxDevicePollTime))
            ;
    }
    if ((millis() - Devices[i].lastseen) > maxLastSeen)
    {
        Devices[i].connected = false;
    }
    i++;
    if (i == maxdevicenum)
    {
        i = 0;
    }
}

void checkDeviceIOchanges()
{
    for (int i = 0; i < maxdevicenum; i++)
    {
        if (Devices[i].iostate.b != Devices[i].lastiostate.b)
        {
            String out;
            out += Devices[i].DEV_ID;
            out += ":    ";
            out += "IOstatechanged";
            PRINT(out);
            Devices[i].lastiostate.b = Devices[i].iostate.b;
        }
    }
}
