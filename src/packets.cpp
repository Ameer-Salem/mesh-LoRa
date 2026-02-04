#include "packets.h"

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
    buffer.insert(buffer.end(), packet.source, packet.source + sizeof(packet.source));
    buffer.insert(buffer.end(), packet.uuid, packet.uuid + sizeof(packet.uuid));
    buffer.push_back(packet.TTL);
    buffer.push_back(packet.neighborsCount);
    buffer.insert(buffer.end(), packet.neighbors, packet.neighbors + sizeof(packet.neighbors));

    return buffer;
}

DataPacket dataFromRaw(uint8_t buffer[], int len)
{
    DataPacket dataPacket{};
    size_t offset = 0;
    dataPacket.type = buffer[offset++];
    dataPacket.TTL = buffer[offset++];
    for (int i = 0; i < sizeof(dataPacket.source); i++)
    {
        dataPacket.source[i] = buffer[offset++];
    }
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
    memset((DiscoveryPacket *)&packet, 0, sizeof(packet));

    packet.type = buffer[0];
    for (int i = 0; i < sizeof(packet.source); i++)
    {
        packet.source[i] = buffer[sizeof(packet.type) + i];
    }
    for (int i = 0; i < sizeof(packet.uuid); i++)
    {
        packet.uuid[i] = buffer[sizeof(packet.type) + sizeof(packet.source) + i];
    }
    packet.TTL = buffer[sizeof(packet.type) + sizeof(packet.source) + sizeof(packet.uuid)];
    packet.neighborsCount = buffer[sizeof(packet.type) + sizeof(packet.source) + sizeof(packet.uuid) + sizeof(packet.TTL)];

    for (int i = 0; i < sizeof(packet.neighbors); i++)
    {
        packet.neighbors[i] = buffer[sizeof(packet.type) +
                                     sizeof(packet.source) +
                                     sizeof(packet.uuid) +
                                     sizeof(packet.TTL) +
                                     sizeof(packet.neighborsCount) + i];
    }
    return packet;
}