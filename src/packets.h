#include "constants.h"

struct Packet
{
    uint8_t type;
    uint8_t source[4];
    uint8_t destination[4];
    uint8_t uuid[6];
    uint8_t segmentIndex;
    uint8_t totalSegments;
    uint8_t length;
    uint8_t payload[200];
};

struct Neighbor 
{
    uint64_t id;
    int8_t   rssi;
    uint32_t lastSeen;
};

extern Neighbor neighbors[];
extern const int MAX_NEIGHBORS;

std::vector<uint8_t> toRaw(Packet &packet);
Packet fromRaw(uint8_t buffer[], int len);



