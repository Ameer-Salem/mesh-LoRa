#include "ble.h"

String bleMessage = "";
bool hasNewMessage = false;

BLEAdvertising *advertising;
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *notifyChar;
BLECharacteristic *writeChar;
BLECharacteristic *neighborChar;
uint16_t negotiatedMTU = 247;

uint32_t NODE_ID;

void bleSetup()
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    NODE_ID = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];

    sLog(BLE_TAG, "Node ID: " + String(NODE_ID));

    BLEDevice::init(String(NODE_ID).c_str());
    pServer = BLEDevice::createServer();
    BLEDevice::setMTU(512);
    pService = pServer->createService(SERVICE_UUID);

    notifyChar = pService->createCharacteristic(CHARACTERISTIC_UUID_notify, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    writeChar = pService->createCharacteristic(CHARACTERISTIC_UUID_write, BLECharacteristic::PROPERTY_WRITE);

    notifyChar->addDescriptor(new BLE2902());
    writeChar->setCallbacks(new RXCallback());
    pServer->setCallbacks(new MyServerCallbacks());

    pService->start();

    advertising = pServer->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();
}

void MyServerCallbacks::onConnect(BLEServer *pServer)
{
    BLEDevice::setMTU(negotiatedMTU);
    sLog(BLE_TAG, "Client connected");
    negotiatedMTU = pServer->getPeerMTU(pServer->getConnId());
    Serial.print("Negotiated MTU: ");
    Serial.println(negotiatedMTU);
    bleMessage = "";
    hasNewMessage = false;
    operationDone = false;
    transmitFlag = false;
};
void MyServerCallbacks::onDisconnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client disconnected");
    advertising->start();
};

void RXCallback::onWrite(BLECharacteristic *characteristic)
{
    uint8_t *pValue = characteristic->getData();
    int len = characteristic->getLength();
    vector<uint8_t> buffer(pValue, pValue + len);

    if (len > 1)
    {
        buffer.insert(buffer.begin() + 1, TTL);
        if (pValue[0] == NEIGHBORS_TYPE)
        {
            sLog(BLE_TAG, "Transmiting neighbors packet...");

            ingoingQueue.push_back(serializeNeighbors(neighbors));
        }
        else if (pValue[0] == TEXT_TYPE)
        {
            sLog(BLE_TAG, "Transmiting data packet...");
            outgoingQueue.push_back(buffer);
        }
        else if (pValue[0] == ACK_TYPE)
        {
            sLog(BLE_TAG, "Transmiting ACK...");
            outgoingQueue.insert(outgoingQueue.begin(), buffer);
        }
    }
}
void notifyBLE()
{
    vector<uint8_t> buffer = ingoingQueue.front();
    notifyChar->setValue(buffer.data(), buffer.size());
    notifyChar->notify();
    ingoingQueue.erase(ingoingQueue.begin());
}
