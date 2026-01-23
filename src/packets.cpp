#include "packets.h"

const int MAX_NEIGHBORS = 10;
Neighbor neighbors[MAX_NEIGHBORS];

std::vector<uint8_t> toRaw(Packet &packet)
{
    std::vector<uint8_t> buffer;

    buffer.push_back(packet.type);

    buffer.insert(buffer.end(), packet.source, packet.source + sizeof(packet.source));
    buffer.insert(buffer.end(), packet.destination, packet.destination + sizeof(packet.destination));
    buffer.insert(buffer.end(), packet.uuid, packet.uuid + sizeof(packet.uuid));

    buffer.push_back(packet.segmentIndex);
    buffer.push_back(packet.totalSegments);
    buffer.push_back(packet.length);

    buffer.insert(buffer.end(), packet.payload, packet.payload + packet.length);

    return buffer;
}
Packet fromRaw(uint8_t buffer[], int len)
{
    Packet packet;
    packet.type = buffer[0];
    for (int i = 0; i < sizeof(packet.source); i++)
    {
        packet.source[i] = buffer[sizeof(packet.type) + i];
    }
    for (int i = 0; i < sizeof(packet.destination); i++)
    {
        packet.destination[i] = buffer[sizeof(packet.type) + sizeof(packet.source) + i];
    }
    for (int i = 0; i < sizeof(packet.uuid); i++)
    {
        packet.uuid[i] = buffer[sizeof(packet.type) + sizeof(packet.source) + sizeof(packet.destination) + i];
    }
    packet.segmentIndex = buffer[sizeof(packet.type) +
                                 sizeof(packet.source) +
                                 sizeof(packet.destination) +
                                 sizeof(packet.uuid)];
    packet.totalSegments = buffer[sizeof(packet.type) +
                                  sizeof(packet.source) +
                                  sizeof(packet.destination) +
                                  sizeof(packet.uuid) +
                                  sizeof(packet.segmentIndex)];
    packet.length = buffer[sizeof(packet.type) +
                           sizeof(packet.source) +
                           sizeof(packet.destination) +
                           sizeof(packet.uuid) +
                           sizeof(packet.segmentIndex) +
                           sizeof(packet.totalSegments)];
    int safeLen = min(packet.length, (uint8_t)sizeof(packet.payload));
    for (int i = 0; i < safeLen; i++)
    {
        packet.payload[i] = buffer[sizeof(packet.type) +
                                   sizeof(packet.source) +
                                   sizeof(packet.destination) +
                                   sizeof(packet.uuid) +
                                   sizeof(packet.segmentIndex) +
                                   sizeof(packet.totalSegments) +
                                   sizeof(packet.length) + i];
    }
    packet.length = safeLen;
    return packet;
}