#include "LoRa.h"

vector<Neighbor> neighbors;

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
int state;

bool operationDone = false;
bool transmitFlag = false;

vector<vector<uint8_t>> outgoingQueue;
vector<vector<uint8_t>> ingoingQueue;

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
    vector<uint8_t> buffer = outgoingQueue.front();

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
void sendDiscoveryPacket(DiscoveryPacket packet, bool reSend)
{

    sLog(LORA_TAG, "Sending discovery packet...");
    if (!reSend)
    {
        memset((DiscoveryPacket *)&packet, 0, sizeof(packet));

        packet.type = DISCOVERY_TYPE;
        packet.source[0] = (NODE_ID >> 24) & 0xFF;
        packet.source[1] = (NODE_ID >> 16) & 0xFF;
        packet.source[2] = (NODE_ID >> 8) & 0xFF;
        packet.source[3] = NODE_ID & 0xFF;
        packet.TTL = 1;
        packet.neighborsCount = neighbors.size();
        for (int i = 0; i < neighbors.size(); i++)
        {
            packet.neighbors[i * 4] = (neighbors[i].id >> 24) & 0xFF;
            packet.neighbors[i * 4 + 1] = (neighbors[i].id >> 16) & 0xFF;
            packet.neighbors[i * 4 + 2] = (neighbors[i].id >> 8) & 0xFF;
            packet.neighbors[i * 4 + 3] = neighbors[i].id & 0xFF;
        }
        vector<uint8_t> buffer = toRaw(packet);
        outgoingQueue.push_back(buffer);
    }
    else
    {
        packet.TTL -= 1;
        vector<uint8_t> buffer = toRaw(packet);
        outgoingQueue.push_back(buffer);
    }
};

void receive()
{
    vector<uint8_t> buffer;
    int len = lora.getPacketLength();
    buffer.resize(len);

    state = lora.readData(buffer.data(), len);
    if (!state)
    {
        if (buffer[0] == ACK_TYPE)
        {
            sLog(LORA_TAG, "LoRa received ack packet...");
            return;
        }
        else if (buffer[0] == DISCOVERY_TYPE)
        {
            sLog(LORA_TAG, "LoRa received discovery packet...");
            discoveryCheck(buffer);
            ingoingQueue.insert(ingoingQueue.begin(), buffer);
            return;
        }
        else if (buffer[0] == TEXT_TYPE)
        {
            sLog(LORA_TAG, "LoRa received data packet...");
            ingoingQueue.push_back(buffer);
            return;
        }
        logBytes(LORA_TAG, "Type", &buffer[0], 1);
    }
    else
    {
        sLog(LORA_TAG, "LoRa reception failed!");
        while (1)
            ;
    }
}

void discoveryCheck(vector<uint8_t> buffer)
{
    DiscoveryPacket packet;
    memset((DiscoveryPacket *)&packet, 0, sizeof(packet));
    packet = discoveryFromRaw(buffer.data(), buffer.size());

    Neighbor neighbor;
    neighbor.id = ((packet.source[0] << 24) | (packet.source[1] << 16) | (packet.source[2] << 8) | packet.source[3]);
    if (neighbor.id == NODE_ID)
    {
        sLog(LORA_TAG, "Received discovery packet from self!");
        return;
    }
    if (packet.TTL != 0)
    {
        logBytes(LORA_TAG, "resending discovery packet", (uint8_t *)&packet, sizeof(packet));
        sendDiscoveryPacket(packet, true);
    }

    for (auto &i : neighbors)
    {
        if (i.id == neighbor.id)
        {
            sLog(LORA_TAG, "Received discovery packet from known neighbor!");
            i.rssi = lora.getRSSI();
            i.lastSeen = millis();
            return;
        }
    }
    neighbor.rssi = lora.getRSSI();
    neighbor.lastSeen = millis();
    neighbors.push_back(neighbor);

    uint8_t neighborCount = packet.neighborsCount;
    for (int i = 0; i < neighborCount; i++)
    {
        uint32_t neighborId = ((packet.neighbors[i * 4] << 24) | (packet.neighbors[i * 4 + 1] << 16) | (packet.neighbors[i * 4 + 2] << 8) | packet.neighbors[i * 4 + 3]);

        bool found = false;
        for (auto &n : neighbors)
        {
            if (n.id == neighborId)
            {
                n.lastSeen = millis();
                found = true;
                break;
            }
        }

        if (!found)
        {
            sLog(LORA_TAG, "adding new neighbor from discovery packet neighbors...");
            Neighbor neighbor;
            neighbor.id = neighborId;
            neighbor.rssi = lora.getRSSI();
            neighbor.lastSeen = millis();
            neighbors.push_back(neighbor);
        }
        else
        {
            sLog(LORA_TAG, "Neighbor from discovery packet neighbors already exists!");
        }
    }

    for (const auto &n : neighbors)
    {
        sLog(LORA_TAG, "Neighbors : ");
        Serial.printf("ID=%u RSSI=%d lastSeen=%lu\n",
                      n.id, n.rssi, n.lastSeen);
    }
}

vector<uint8_t> serializeNeighbors(const vector<Neighbor> &neighbors)
{
    vector<uint8_t> out;

    // COUNT (max 255 neighbors)
    out.push_back(NEIGHBORS_TYPE);
    out.push_back(neighbors.size());
    
    for (const auto &n : neighbors)
    {
        // id (uint32, big endian)
        out.push_back((n.id >> 24) & 0xFF);
        out.push_back((n.id >> 16) & 0xFF);
        out.push_back((n.id >> 8) & 0xFF);
        out.push_back(n.id & 0xFF);

        // rssi (int8)
        out.push_back((uint8_t)n.rssi);

        // lastSeen (uint32)
        out.push_back((n.lastSeen >> 24) & 0xFF);
        out.push_back((n.lastSeen >> 16) & 0xFF);
        out.push_back((n.lastSeen >> 8) & 0xFF);
        out.push_back(n.lastSeen & 0xFF);
    }

    return out;
}