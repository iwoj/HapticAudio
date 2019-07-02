// WiFi Stuff ---------
#include <WiFi.h>         
#include <WebServer.h>    
#include <AutoConnect.h>
WebServer   Server;
AutoConnect Portal(Server);



// BLE Stuff ---------
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <LinkedList.h>

#define PROXIMITY_LIMIT_RSSI -55
int scanTime = 1; //In seconds
BLEScan* pBLEScan;
bool runningScan = false;
LinkedList<BLEAdvertisedDevice> closeDevices;
LinkedList<BLEAdvertisedDevice> previousCloseDevices;



// Capacitive Touch Stuff ---------
const int TOUCH_SENSOR_THRESHOLD = 20;
int touchSensor1Value=0;
int touchSensor2Value=0;
int touchSensor3Value=0;
boolean touch1Start = false;
boolean touch2Start = false;
boolean touch3Start = false;



const int LED_PIN = 5; // Thing's onboard LED



class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveRSSI()){
        if ((int)advertisedDevice.getRSSI() > PROXIMITY_LIMIT_RSSI) {
//          Serial.printf("Advertised Device: %s", advertisedDevice.toString().c_str());
//          Serial.printf(", Rssi: %d \n", (int)advertisedDevice.getRSSI());
          closeDevices.add(advertisedDevice);
        }
      }
    }
};



void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  connectWiFi();
  setupBLE();
  runBLEScan();
}



void loop() {
  Portal.handleClient();
  readSensors();
  senseTouchEvents();
}



void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}



void connectWiFi()
{
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}



void setupBLE() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
}



void runBLEScan() {
  Serial.println("Scanning BLE...");
//  clearDevices(previousCloseDevices);
  previousCloseDevices = closeDevices;
//  clearDevices(closeDevices);
  closeDevices = LinkedList<BLEAdvertisedDevice>();
  runningScan = true;
  pBLEScan->start(scanTime, scanComplete);
}



void scanComplete(BLEScanResults foundDevices) {  
  runningScan = false;
  closeDevices.sort(sortDevices);
  handleBLEProximity();
  pBLEScan->clearResults();
  Serial.println("");
  runBLEScan();
}



LinkedList<BLEAdvertisedDevice> listDiff(LinkedList<BLEAdvertisedDevice> a, LinkedList<BLEAdvertisedDevice> b) {
  LinkedList<BLEAdvertisedDevice> combined;
  LinkedList<int> toRemove;
  for (int i = 0; i < a.size(); i++) {
    combined.add(a.get(i));
  }
  for (int i = 0; i < b.size(); i++) {
    combined.add(b.get(i));
  }
  combined.sort(sortDevices);
//  for (int i = 0; i < combined.size(); i++) {
//    if (i >= combined.size() - 1 && 
//        combined.get(i).getAddress().toString() == combined.get(i+1).getAddress().toString()) {
//      toRemove.add(i);
//      toRemove.add(i+1);
//    }
//  }
//  toRemove.sort(sortIntReverse);
//  for (int i = 0; i < toRemove.size(); i++) {
//    combined.remove(toRemove.get(i));
//  }
  return combined;
}



LinkedList<BLEAdvertisedDevice> listNotIn(LinkedList<BLEAdvertisedDevice> a, LinkedList<BLEAdvertisedDevice> b) {
  LinkedList<BLEAdvertisedDevice> diffList = listDiff(a, b);
  return listDiff(diffList, a);
}





void printDeviceList(LinkedList<BLEAdvertisedDevice> a) {
  for (int i = 0; i < a.size() - 1; i++) {
    Serial.print(a.get(i).getAddress().toString().c_str());
    Serial.print(", ");
  }
  Serial.print(a.get(a.size() - 1).getAddress().toString().c_str());
}




void handleBLEProximity() {
  if (!runningScan) {
    if (closeDevices.size() > 0) {
      Serial.println(closeDevices.size() + (String)" device(s) close by!");
      // Do stuff.
    }
    else if (closeDevices.size() == 0) {
      Serial.println("No devices close by.");
      // Do stuff.
    }
//    LinkedList<BLEAdvertisedDevice> ld = listDiff(closeDevices, previousCloseDevices);
//    printDeviceList(ld);
//    ld.clear();
    printDeviceList(listDiff(closeDevices, previousCloseDevices));
    Serial.print("\n");
    
    // Send insert/remove commands to Meteor
//    LinkedList<BLEAdvertisedDevice> newDevices = listNotIn(closeDevices, previousCloseDevices);
//    Serial.print("New devices: ");
//    printDeviceList(newDevices);
//    Serial.print("\n");
    
//    LinkedList<BLEAdvertisedDevice> removedDevices = listNotIn(previousCloseDevices, closeDevices);
//    Serial.print("Removed devices: ");
//    printDeviceList(removedDevices);
//    Serial.print("\n");
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



int sortDevices(BLEAdvertisedDevice &a, BLEAdvertisedDevice &b) {
  return strcmp(a.getAddress().toString().c_str(), 
                b.getAddress().toString().c_str());
}



int sortIntReverse(int &a, int &b) {
  return a < b ? 1 : -1;
}


