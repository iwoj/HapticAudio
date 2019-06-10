#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>



// BLE Stuff ---------
#define PROXIMITY_LIMIT_RSSI -55

int scanTime = 1; //In seconds
BLEScan* pBLEScan;
bool runningScan = false;
int numCloseDevices = 0;



// Capacitive Touch Stuff ---------
const int TOUCH_SENSOR_THRESHOLD = 20;

int touchSensor1Value=0;
int touchSensor2Value=0;
int touchSensor3Value=0;
boolean touch1Start = false;
boolean touch2Start = false;
boolean touch3Start = false;



class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveRSSI()){
        if ((int)advertisedDevice.getRSSI() > PROXIMITY_LIMIT_RSSI) {
          Serial.printf("Advertised Device: %s", advertisedDevice.toString().c_str());
          Serial.printf(", Rssi: %d \n", (int)advertisedDevice.getRSSI());
          numCloseDevices++;
        }
      }
    }
};



void setup() {
   Serial.begin(115200);
//   setupIBeacon();
   setupBLE();
   runBLEScan();
}



void loop() {
  readSensors();
  senseTouchEvents();
}



void setupBLE() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
}



void runBLEScan() {
  Serial.println("Scanning BLE...");
  numCloseDevices = 0;
  runningScan = true;
  pBLEScan->start(scanTime, scanComplete);
}



void scanComplete(BLEScanResults foundDevices) {  
  runningScan = false;
  handleBLEProximity();
  pBLEScan->clearResults();
  Serial.println("");
  runBLEScan();
}



void handleBLEProximity() {
  if (!runningScan) {
    if (numCloseDevices > 0) {
      Serial.println(numCloseDevices + (String)" device(s) close by!");
      // Do stuff.
    }
    else if (numCloseDevices == 0) {
      Serial.println("No devices close by.");
      // Do stuff.
    }
  }
}



void readSensors() {
  int numberOfReadings = 10;
  
  touchSensor1Value = 0;
  touchSensor2Value = 0;
  touchSensor3Value = 0;
  
  for(int i=0; i< numberOfReadings; i++)
  {
    touchSensor1Value += touchRead(T0);
    touchSensor2Value += touchRead(T2);
    touchSensor3Value += touchRead(T3);
  }
  
  touchSensor1Value = touchSensor1Value/numberOfReadings;
  touchSensor2Value = touchSensor2Value/numberOfReadings;
  touchSensor3Value = touchSensor3Value/numberOfReadings;
}



void senseTouchEvents() {
  if (touchSensor1Value < TOUCH_SENSOR_THRESHOLD && !touch1Start) {
    Serial.print("Touch1 start (");
    Serial.print(touchSensor1Value);
    Serial.println(")");
    touch1Start = true;
  }
  else if (touchSensor1Value > TOUCH_SENSOR_THRESHOLD && touch1Start) {
    Serial.println("Touch1 end");
    touch1Start = false;
  }
  
  if (touchSensor2Value < TOUCH_SENSOR_THRESHOLD && !touch2Start) {
    Serial.print("Touch2 start (");
    Serial.print(touchSensor2Value);
    Serial.println(")");
    touch2Start = true;
  }
  else if (touchSensor2Value > TOUCH_SENSOR_THRESHOLD && touch2Start) {
    Serial.println("Touch2 end");
    touch2Start = false;
  }
  
  if (touchSensor3Value < TOUCH_SENSOR_THRESHOLD && !touch3Start) {
    Serial.print("Touch3 start (");
    Serial.print(touchSensor3Value);
    Serial.println(")");
    touch3Start = true;
  }
  else if (touchSensor3Value > TOUCH_SENSOR_THRESHOLD && touch3Start) {
    Serial.println("Touch3 end");
    touch3Start = false;
  }
}

//
//void setupIBeacon() {
//  Serial.println("Starting BLE work!");
//
//  BLEDevice::init("Redwoods");
//  BLEServer *pServer = BLEDevice::createServer();
//  BLEService *pService = pServer->createService(SERVICE_UUID);
//  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
//                                         CHARACTERISTIC_UUID,
//                                         BLECharacteristic::PROPERTY_READ |
//                                         BLECharacteristic::PROPERTY_WRITE
//                                       );
//
//  pCharacteristic->setValue("My characteristic value");
//  pService->start();
//  BLEAdvertising *pAdvertising = pServer->getAdvertising();
//  
//  Serial.println("Building up iBeacon advertising ..");
//  BLEBeacon myBeacon;
//  // Setting up according to https://os.mbed.com/blog/entry/BLE-Beacons-URIBeacon-AltBeacons-iBeacon/
//  myBeacon.setManufacturerId(uint16_t (0x1AFF));
//  myBeacon.setMajor(uint16_t (0x0001));
//  myBeacon.setMinor(uint16_t (0x0001));
////  myBeacon.setProximityUUID(uint16_t (0x004C));
//  myBeacon.setProximityUUID(BLEUUID("00000000-0000-0000-0000-000000000001"));
//  myBeacon.setSignalPower(0xC8);
//  
//  BLEAdvertisementData advertisementData;
//  advertisementData.setFlags(ESP_BLE_ADV_FLAG_LIMIT_DISC | ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
//  // Build the iBeacon data structure and store it in std::string beaconData
//  advertisementData.setManufacturerData(myBeacon.getData());
//  pAdvertising->setAdvertisementData(advertisementData);
//  pAdvertising->start();
//  
//  Serial.println("Characteristic defined! Now you can read it in your phone!");
//}
