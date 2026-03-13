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

    if (len > 0)
    {
        switch (buffer[0])
        {
        case NEIGHBORS_TYPE:
            ingoingQueue.push_back(serializeNeighbors(neighbors));
            break;
        case LOCATION_TYPE:
        {
            int32_t latInt;
            int32_t lonInt;

            memcpy(&latInt, &buffer[1], 4);
            memcpy(&lonInt, &buffer[5], 4);

            latitude = latInt / 1000000.0f;
            longitude = lonInt / 1000000.0f;
            sLog(BLE_TAG, "Location: " + String(latitude, 6) + ", " + String(longitude, 6));
            break;
        }
        case TEXT_TYPE:
            buffer.insert(buffer.begin() + 1, TTL);
            sLog(BLE_TAG, "Transmitting data packet...");
            outgoingQueue.push_back(buffer);
            break;
        case ACK_TYPE:
            buffer.insert(buffer.begin() + 1, TTL);
            sLog(BLE_TAG, "Transmitting ACK packet...");
            outgoingQueue.insert(outgoingQueue.begin(), buffer);
            break;
        default:
            break;
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
