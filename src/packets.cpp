#include "packets.h"

unsigned long long lastseen = millis();
vector<uint8_t> toRaw(DataPacket &Datapacket)
{
    vector<uint8_t> buffer;

    buffer.push_back(Datapacket.type);
    buffer.insert(buffer.end(), Datapacket.source, Datapacket.source + sizeof(Datapacket.source));
    buffer.insert(buffer.end(), Datapacket.destination, Datapacket.destination + sizeof(Datapacket.destination));
    buffer.insert(buffer.end(), Datapacket.uuid, Datapacket.uuid + sizeof(Datapacket.uuid));
    buffer.push_back(Datapacket.segmentIndex);
    buffer.push_back(Datapacket.totalSegments);
    buffer.push_back(Datapacket.length);
    buffer.insert(buffer.end(), Datapacket.payload, Datapacket.payload + Datapacket.length);

    return buffer;
}
vector<uint8_t> toRaw(DiscoveryPacket &packet)
{
    vector<uint8_t> buffer;

    buffer.push_back(packet.type);
    buffer.push_back(packet.TTL);
    buffer.push_back(packet.neighborsCount);
    buffer.insert(buffer.end(), packet.source, packet.source + sizeof(packet.source));
    buffer.insert(buffer.end(), packet.latitude, packet.latitude + sizeof(packet.latitude));
    buffer.insert(buffer.end(), packet.longitude, packet.longitude + sizeof(packet.longitude));
    buffer.insert(buffer.end(), packet.neighbors, packet.neighbors + sizeof(packet.neighbors));

    return buffer;
}

DataPacket dataFromRaw(uint8_t buffer[], int len)
{
    DataPacket dataPacket{};
    size_t offset = 0;

    dataPacket.type = buffer[offset++];
    dataPacket.TTL = buffer[offset++];

    dataPacket.source = (uint32_t(buffer[offset++]) << 24) |
                             (uint32_t(buffer[offset++]) << 16) |
                             (uint32_t(buffer[offset++]) << 8) |
                             (uint32_t(buffer[offset++]));

    dataPacket.destination = (uint32_t(buffer[offset++]) << 24) |
                             (uint32_t(buffer[offset++]) << 16) |
                             (uint32_t(buffer[offset++]) << 8) |
                             (uint32_t(buffer[offset++]));

    for (int i = 0; i < sizeof(dataPacket.uuid); i++)
    {
        dataPacket.uuid[i] = buffer[offset++];
    }
    
    dataPacket.segmentIndex = buffer[offset++];
    dataPacket.totalSegments = buffer[offset++];
    dataPacket.length = buffer[offset++];
    int safeLen = min(dataPacket.length, (uint8_t)sizeof(dataPacket.payload));
    for (int i = 0; i < safeLen; i++)
    {
        dataPacket.payload[i] = buffer[sizeof(offset++)];
    }
    dataPacket.length = safeLen;
    return dataPacket;
}
DiscoveryPacket discoveryFromRaw(uint8_t buffer[], int len)
{
    DiscoveryPacket packet;
    size_t offset = 0;
    packet.type = buffer[offset++];
    packet.TTL = buffer[offset++];
    packet.neighborsCount = buffer[offset++];

    for (int i = 0; i < sizeof(packet.source); i++)
    {
        packet.source[i] = buffer[offset++];
    }

    for (int i = 0; i < sizeof(packet.latitude); i++)
    {
        packet.latitude[i] = buffer[offset++];
    }
    for (int i = 0; i < sizeof(packet.longitude); i++)
    {
        packet.longitude[i] = buffer[offset++];
    }

    for (int i = 0; i < packet.neighborsCount * 4; i++)
    {
        packet.neighbors[i] = buffer[offset++];
    }
    return packet;
}