#pragma once
#include <RadioLib.h>
#include "packets.h"
#include <queue>


struct Neighbor 
{
    uint32_t id;
    int8_t   rssi;
    uint32_t lastSeen;
};

extern vector<Neighbor> neighbors;

extern SX1262 lora;
extern int state;

extern bool operationDone;
extern bool transmitFlag;

extern vector<vector<uint8_t>> outgoingQueue;
extern vector<vector<uint8_t>> ingoingQueue;

void loraBegin(float freq, float bw, int sf, int cr );

void sendPacket();
void receive();
void startListening();

void sendDiscoveryPacket(DiscoveryPacket packet =  DiscoveryPacket(), bool reSend = false);
void discoveryCheck(vector<uint8_t> packet);


vector<uint8_t> serializeNeighbors(const vector<Neighbor>& neighbors);