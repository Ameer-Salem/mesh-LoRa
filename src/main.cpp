
#include <RadioLib.h>
#include "BLE.h"

static unsigned long lastDiscoverySend = 0;

void setFlag(void)
{
    operationDone = true;
}

void setup()
{
    randomSeed(analogRead(1));
    Serial.begin(115200);

    bleSetup();
    loraBegin(433.0, 125.0, 8, 6);
    lora.setDio1Action(setFlag);
    startListening();

    sendDiscoveryPacket();
}

void loop()
{
    if (operationDone)
    {
        operationDone = false;
        if (transmitFlag)
        {
            startListening();
            transmitFlag = false;
            delay(random(250, 750));
        }
        else
        {
            receive();
        }
    }
    if (!outgoingQueue.empty())
    {
        sendPacket();
    }
    if (!ingoingQueue.empty())
    {
        notifyBLE();
    }

    unsigned long now = millis();
    if (now - lastDiscoverySend >= 30e3)
    {
        sendDiscoveryPacket();
        lastDiscoverySend = now;
    }
}
