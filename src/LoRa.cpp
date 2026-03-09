#include "LoRa.h"

vector<Neighbor> neighbors;

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
int state;

bool operationDone = false;
bool transmitFlag = false;

float latitude = 0;
float longitude = 0;

vector<vector<uint8_t>> outgoingQueue;
vector<vector<uint8_t>> ingoingQueue;

void loraBegin(float freq, float bw, int sf, int cr)
{
    state = lora.begin(freq, bw, sf, cr);
    if (state)
    {
        sLog(LORA_TAG, "LoRa initialization failed!");
        while (1)
            ;
    }
};
void startListening()
{
    state = lora.startReceive();
    if (state)
    {
        sLog(LORA_TAG, "LoRa listening failed!");
        while (1)
            ;
    }
}

void sendPacket()
{
    if (lora.getIrqFlags() || transmitFlag)
        return;
    vector<uint8_t> buffer = outgoingQueue.front();
    state = lora.startTransmit(buffer.data(), buffer.size());
    if (!state)
    {
        outgoingQueue.erase(outgoingQueue.begin());
        transmitFlag = true;
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
    if (!reSend)
    {
        packet.type = DISCOVERY_TYPE;
        packet.TTL = 1;
        packet.source[0] = (NODE_ID >> 24) & 0xFF;
        packet.source[1] = (NODE_ID >> 16) & 0xFF;
        packet.source[2] = (NODE_ID >> 8) & 0xFF;
        packet.source[3] = NODE_ID & 0xFF;
        size_t maxNeighbors = sizeof(packet.neighbors) / 4;
        size_t count = min(neighbors.size(), maxNeighbors);
        packet.neighborsCount = count;

        for (int i = 0; i < count; i++)
        {
            packet.neighbors[i * 4] = (neighbors[i].id >> 24) & 0xFF;
            packet.neighbors[i * 4 + 1] = (neighbors[i].id >> 16) & 0xFF;
            packet.neighbors[i * 4 + 2] = (neighbors[i].id >> 8) & 0xFF;
            packet.neighbors[i * 4 + 3] = neighbors[i].id & 0xFF;
        }
        memcpy(packet.latitude, &latitude, 4);
        memcpy(packet.longitude, &longitude, 4);

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
    if (!state && len > 0)
    {
        if (buffer[0] == DISCOVERY_TYPE)
        {
            discoveryCheck(buffer);
            return;
        }
        DataPacket packet = dataFromRaw(buffer.data(), buffer.size());
        if (packet.TTL > 0)
        {
            buffer[1]--;
            outgoingQueue.insert(outgoingQueue.begin(), buffer);
        }
        buffer.erase(buffer.begin() + 1);
        if (packet.destination == NODE_ID)
        {
            if (packet.type == ACK_TYPE)
            {
                sLog(LORA_TAG, "ACK packet received! ");
                ingoingQueue.insert(ingoingQueue.begin(), buffer);
                return;
            }
            else if (packet.type == TEXT_TYPE)
            {
                sLog(LORA_TAG, "DATA packet received...");
                ingoingQueue.push_back(buffer);
                return;
            }
        }
    }
}

void discoveryCheck(vector<uint8_t> buffer)
{
    DiscoveryPacket packet = discoveryFromRaw(buffer.data(), buffer.size());
    Neighbor neighbor;

    neighbor.id = ((packet.source[0] << 24) | (packet.source[1] << 16) | (packet.source[2] << 8) | packet.source[3]);
    if (neighbor.id == NODE_ID)
        return;
    if (packet.TTL > 0)
        sendDiscoveryPacket(packet, true);

    // ---------- HANDLE SOURCE NEIGHBOR ----------
    bool sourceFound = false;
    for (auto &i : neighbors)
    {
        if (i.id == neighbor.id)
        {
            i.rssi = lora.getRSSI();
            i.lastSeen = millis();

            memcpy(&i.latitude, packet.latitude, 4);
            memcpy(&i.longitude, packet.longitude, 4);

            sourceFound = true;
            break;
        }
    }
    if (!sourceFound)
    {
        neighbor.rssi = lora.getRSSI();
        neighbor.lastSeen = millis();

        memcpy(&neighbor.latitude, packet.latitude, 4);
        memcpy(&neighbor.longitude, packet.longitude, 4);

        neighbors.push_back(neighbor);
    }
    // ---------- HANDLE RELAYED NEIGHBORS ----------
    uint8_t neighborCount = packet.neighborsCount;
    for (int i = 0; i < neighborCount; i++)
    {
        uint32_t neighborId = ((packet.neighbors[i * 4] << 24) | (packet.neighbors[i * 4 + 1] << 16) | (packet.neighbors[i * 4 + 2] << 8) | packet.neighbors[i * 4 + 3]);
        if (neighborId == NODE_ID)
            continue;
        bool found = false;

        for (auto &n : neighbors)
        {
            if (n.id == neighborId)
            {
                n.lastSeen = millis();
                n.rssi = lora.getRSSI();
                memcpy(&n.latitude, packet.latitude, 4);
                memcpy(&n.longitude, packet.longitude, 4);
                found = true;
                break;
            }
        }
        if (!found)
        {
            Neighbor neighbor;
            neighbor.id = neighborId;
            neighbor.rssi = lora.getRSSI();
            neighbor.lastSeen = millis();
            memcpy(&neighbor.latitude, packet.latitude, 4);
            memcpy(&neighbor.longitude, packet.longitude, 4);
            neighbors.push_back(neighbor);
        }
    }

    // ---------- ALWAYS LOG NEIGHBORS ----------
    sLog(LORA_TAG, "Neighbors:");
    for (const auto &n : neighbors)
    {
        Serial.printf(
            "ID=%u RSSI=%d lastSeen=%lu latitude=%f longitude=%f\n",
            n.id,
            n.rssi,
            n.lastSeen,
            n.latitude,
            n.longitude);
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

        float lat = n.latitude;
        uint8_t *p = (uint8_t *)&lat;
        out.insert(out.end(), p, p + 4);

        float lon = n.longitude;
        p = (uint8_t *)&lon;
        out.insert(out.end(), p, p + 4);
    }

    return out;
}