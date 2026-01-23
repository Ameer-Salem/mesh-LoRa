
// #include <esp_system.h>
// #include <Arduino.h>
// void setup()
// {

//     Serial.begin(115200);
//     uint64_t mac;
//     esp_read_mac((uint8_t *)&mac, ESP_MAC_BT);
//     Serial.printf("BLE MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
//                   (uint8_t)(mac >> 40),
//                   (uint8_t)(mac >> 32),
//                   (uint8_t)(mac >> 24),
//                   (uint8_t)(mac >> 16),
//                   (uint8_t)(mac >> 8),
//                   (uint8_t)(mac));
// }