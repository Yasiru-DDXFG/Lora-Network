#include "LoRa.h"
#include "NODE.h"

NODE::NODE(int cs, int rst, int irq, long tx, long rx)
{
    this->csPin = cs;
    this->irqPin = irq;
    this->resetPin = rst;
    this->txFrequency = tx;
    this->rxFrequency = rx;
}

void NODE::Init()
{
    packet_ptr = &datapacket;
    data = (uint8_t *)packet_ptr;
    MY_ID = 0xffff;
    datapacket.sourceID = MY_ID;
    datapacket.command = STATUS_PUSH;
    LoRa.setPins(csPin, resetPin, irqPin);
    debug_println(csPin);
    debug_println(resetPin);
    debug_println(irqPin);
    debug_println(txFrequency);
    debug_println(rxFrequency);

    while (!LoRa.begin(rxFrequency))
    {
        debug_println("LoRa init failed. Check your connections.");
        delay(1000); // if failed, keep trying every second
    }
    debug_println("Lora init success!");
    LoRa.receive();
    readIO(&lastIOstate);
    readIO(&currentIOstate);
}

void NODE::setID(uint16_t ID)
{
    MY_ID = ID + OFFSET_ID;
    datapacket.sourceID = MY_ID;
    datapacket.destinationtID = 0;
}

void NODE::pushPacket()
{
    //setPacketData(&lastIOstate, ptr);
    LoRa.setFrequency(txFrequency);
    LoRa.beginPacket();
    //uint8_t *pt = (uint8_t *)packet_ptr;
    for (unsigned long long i = 0; i < sizeof(Packet); i++)
    {
        LoRa.write(data[i] ^ PRIVATEKEY);
    }
    //LoRa.write(pt, sizeof(Packet));
    LoRa.endPacket();
    LoRa.setFrequency(rxFrequency);
    LoRa.receive();
}

void NODE::callBack(int packetSize)
{

    if (packetSize == 0)
        return; // if there's no packet, return

    LoRa.readBytes(data, sizeof(Packet));
    for (unsigned long long i = 0; i < sizeof(Packet); i++)
    {
        data[i] ^= PRIVATEKEY;
    }
    debug_print("Recv! ----> Command:");
    if (datapacket.command == AREYOUREADY)
    {
        debug_print("areyouready?   ");
    }
    if (datapacket.command == ACK)
    {
        debug_print("ack from gateway!   ");
    }
    if (datapacket.command == STATUS_PUSH)
    {
        debug_print("status push!   ");
    }
    debug_print("source ID:");
    debug_print(datapacket.sourceID);
    debug_print("   destination ID:");
    debug_println(datapacket.destinationtID);

    if (STATE == POWER_ON)
    { // ON POWER ON CHECK FOR REGISTER PACKET FROM GATEWAY THEN PUSH A FIRST PACKET
        if (datapacket.command == AREYOUREADY && datapacket.destinationtID == MY_ID)
        {
            debug_println("Register request from gateway!");
            STATE = SOMETHING_HAPPENED;
        }
    }
    else if (STATE == SOMETHING_HAPPENED)
    { // WAIT FOR ACK FROM GATEWAY DURING IO PUSH AND GO BACK TO IDLE
        if (datapacket.command == ACK && datapacket.destinationtID == MY_ID)
        {
            debug_println("GATEWAY acknowledged data");
            STATE = IDLE;
        }
    }
    else if (STATE == IDLE)
    { // WAIT FOR ACK FROM GATEWAY DURING IO PUSH AND GO BACK TO IDLE
        if (datapacket.command == AREYOUREADY && datapacket.destinationtID == MY_ID)
        {
            debug_println("Register request from gateway!");
            STATE = SOMETHING_HAPPENED;
        }
    }
}

void NODE::checkStatus()
{
    // check for IO pin changes
    if (STATE == POWER_ON)
    {
    }
    else if (STATE == IDLE)
    { // AFTER REGISTRATION GO TO IDLE MODE... POLL FOR IO CHANGES

        readIO(&currentIOstate); // read IO pins into  current ioSTATE
        //debug_print("Current state:");
        //debug_print(currentIOstate.iovar);
        //debug_print("   last state:");
        //debug_print(lastIOstate.iovar);
        if (currentIOstate.iovar != lastIOstate.iovar)
        {
            STATE = SOMETHING_HAPPENED;
            debug_println("State changed! Attempting to push data..");
            lastIOstate.iovar = currentIOstate.iovar;
        }
    }
    else if (STATE == SOMETHING_HAPPENED)
    { // IO CHANGED!  TRY TO PUSH IO DATA TO GATEWAY AT RANDOM INTERVALS
        if (randomElapsedWithin(1000, 5000))
        {
            datapacket.command = STATUS_PUSH;
            datapacket.sourceID = MY_ID;
            datapacket.data = currentIOstate;
            pushPacket();
            debug_println("Pushing data...");
        }
    }
}

bool NODE::randomElapsedWithin(unsigned long from_ms, unsigned long to_ms)
{
    static unsigned long last = millis();
    static unsigned long target = rand() % ((to_ms - from_ms) + 1) + from_ms;
    bool elapsed = false;
    if ((millis() - last) > target)
    {
        elapsed = true;
        target = rand() % ((to_ms - from_ms) + 1) + from_ms;
        last = millis();
    }
    return elapsed;
}

NODE::~NODE()
{
}
