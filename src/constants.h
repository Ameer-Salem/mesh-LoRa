#pragma once
#include "slog.h"

// ===== BLE_UUIDs =====
static const char* SERVICE_UUID =               "ffffffff-ffff-ffff-ffff-ffffffffffff";
static const char* CHARACTERISTIC_UUID_notify = "ffffffff-ffff-ffff-ffff-fffffffffff0";
static const char* CHARACTERISTIC_UUID_read =   "ffffffff-ffff-ffff-ffff-ffffffffff00";
static const char* CHARACTERISTIC_UUID_write =  "ffffffff-ffff-ffff-ffff-fffffffff000";

// ===== Log Tags =====
static const String BLE_TAG  = "[BLE]    ";
static const String LORA_TAG = "[SX1262] ";

// ===== Node Info =====
extern uint32_t NODE_ID;

// ===== Protocol Constants =====
static constexpr int ACK_TYPE = 0x00;
static constexpr int DISCOVERY_TYPE = 0x01;
static constexpr int TEXT_TYPE = 0x02;
