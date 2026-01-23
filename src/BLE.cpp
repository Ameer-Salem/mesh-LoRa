#include "ble.h"

String bleMessage = "";
bool hasNewMessage = false;

BLEAdvertising *advertising;
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *notifyChar;
BLECharacteristic *writeChar;
BLECharacteristic *readChar;


uint32_t NODE_ID;

void bleSetup(){
    uint8_t  mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    NODE_ID = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];

    sLog(BLE_TAG, "Node ID: " + String(NODE_ID));

    BLEDevice::init(String(NODE_ID).c_str());
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);


    notifyChar = pService->createCharacteristic(CHARACTERISTIC_UUID_notify, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    writeChar = pService->createCharacteristic(CHARACTERISTIC_UUID_write, BLECharacteristic::PROPERTY_WRITE);

    notifyChar ->addDescriptor(new BLE2902());
    writeChar->setCallbacks(new RXCallback());
    pServer->setCallbacks(new MyServerCallbacks());

    pService->start();

    advertising = pServer->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();
}

void MyServerCallbacks::onConnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client connected");
    bleMessage = "";
    hasNewMessage = false;
};
void MyServerCallbacks::onDisconnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client disconnected");
    advertising->start();
};

void RXCallback::onWrite(BLECharacteristic *characteristic) {

}
void notifyBLE(std::vector<uint8_t> buffer)
{
}
