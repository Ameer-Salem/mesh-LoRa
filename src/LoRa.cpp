#include "LoRa.h"

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
int state;

bool operationDone = false;
bool transmitFlag = false;

std::vector<Packet> outgoingQueue;
std::vector<Packet> ingoingQueue;

void loraBegin(float freq, float bw, int sf, int cr)
{
    state = lora.begin(freq, bw, sf, cr);
    if (!state)
    {
        sLog(LORA_TAG, "LoRa initialized successfully!");
    }
    else
    {
        sLog(LORA_TAG, "LoRa initialization failed!");
        while (1)
            ;
    }
};
void startListening()
{
    state = lora.startReceive();
    if (!state)
    {
        sLog(LORA_TAG, "LoRa is listening...");
    }
    else
    {
        sLog(LORA_TAG, "LoRa listening failed!");
        while (1)
            ;
    }
}

void sendPacket()
{
    Packet packet = outgoingQueue.front();
    logBytes(LORA_TAG, "Packet :", (uint8_t *)&packet, sizeof(packet));

    std::vector<uint8_t> buffer = toRaw(packet);

    state = lora.startTransmit(buffer.data(), buffer.size());
    if (!state)
    {
        outgoingQueue.erase(outgoingQueue.begin());
        transmitFlag = true;
        sLog(LORA_TAG, "Packet sent successfully!");
    }
    else
    {
        sLog(LORA_TAG, "Packet sending failed!");
        while (1)
            ;
    }
}
void sendDiscoveryPacket()
{
    sLog(LORA_TAG, "Sending discovery packet...");

    Packet packet;
    memset((Packet *)&packet, 0, sizeof(packet));

    packet.type = DISCOVERY_TYPE;

    packet.source[0] = (NODE_ID >> 24) & 0xFF;
    packet.source[1] = (NODE_ID >> 16) & 0xFF;
    packet.source[2] = (NODE_ID >> 8) & 0xFF;
    packet.source[3] = NODE_ID & 0xFF;

    logBytes(LORA_TAG, "Discovery packet", (uint8_t *)&packet, sizeof(packet));
    outgoingQueue.push_back(packet);
};

void receive()
{
    Packet packet;
    std::vector<uint8_t> buffer;
    int len = lora.getPacketLength();
    buffer.resize(len);


    state = lora.readData(buffer.data(), len);
    if (!state)
    {
        packet = fromRaw(buffer.data(), len);

        if (packet.type == ACK_TYPE)
            ingoingQueue.insert(ingoingQueue.begin(), packet);
        else if (packet.type != DISCOVERY_TYPE)
            ingoingQueue.push_back(packet);
        discoveryCheck(packet);
        sLog(LORA_TAG, "LoRa received packet...");
    }
    else
    {
        sLog(LORA_TAG, "LoRa reception failed!");
        while (1)
            ;
    }
}

void discoveryCheck(Packet packet){
    logBytes(LORA_TAG, "Discovery packet", (uint8_t *)&packet, sizeof(packet));
}