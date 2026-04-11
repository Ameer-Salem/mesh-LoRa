#pragma once
#include "constants.h"

struct DataPacket
{
    uint8_t type = TEXT_TYPE;
    uint8_t TTL;
    uint32_t source;
    uint32_t destination;
    uint8_t uuid[6];
    uint8_t segmentIndex;
    uint8_t totalSegments;
    uint8_t length;
    uint8_t payload[200];
};

struct DiscoveryPacket
{
    uint8_t type = DISCOVERY_TYPE;
    uint8_t TTL;
    uint8_t neighborsCount;
    uint8_t source[4];
    uint8_t latitude[4];
    uint8_t longitude[4];
    uint8_t neighbors[4*6];
};


vector<uint8_t> toRaw(DataPacket &packet);
vector<uint8_t> toRaw(DiscoveryPacket &packet);
DataPacket dataFromRaw(uint8_t buffer[], int len);
DiscoveryPacket discoveryFromRaw(uint8_t buffer[], int len);



