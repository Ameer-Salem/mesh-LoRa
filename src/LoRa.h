#include <RadioLib.h>
#include "packets.h"
#include <queue>

extern SX1262 lora;
extern int state;

extern bool operationDone;
extern bool transmitFlag;

extern std::vector<Packet> outgoingQueue;
extern std::vector<Packet> ingoingQueue;

void loraBegin(float freq, float bw, int sf, int cr );
void startListening();

void sendPacket();
void sendDiscoveryPacket();

void receive();
void discoveryCheck(Packet packet);