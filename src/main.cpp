
#include <RadioLib.h>
#include "BLE.h"

static unsigned long lastDiscoverySend = 0;

void setFlag(void)
{
    operationDone = true;
}

void setup()
{

    Serial.begin(115200);

    bleSetup();
    loraBegin(433.0, 125.0, 9, 6);
    lora.setDio1Action(setFlag);
    startListening();

    sendDiscoveryPacket();
}

void loop()
{
    if (operationDone)
    {
        sLog(LORA_TAG, "Operation done!");
        operationDone = false;
        if (transmitFlag)
        {
            startListening();
            sLog(LORA_TAG, "Previous operation was transmission");
            transmitFlag = false;
            delay(100);
        }
        else
        {
            sLog(LORA_TAG, "Previous operation was reception");
            receive();
        }
    }
    else if (!outgoingQueue.empty())
    {
        sendPacket();
    }
    if (!ingoingQueue.empty())
    {
        notifyBLE();
    }

    unsigned long now = millis();
    if (now - lastDiscoverySend >= 100e3)
    {
        sendDiscoveryPacket();
        lastDiscoverySend = now;
    }
}
