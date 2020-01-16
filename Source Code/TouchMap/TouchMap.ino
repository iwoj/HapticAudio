// =========================
// Must upload with No OTA (Large APP) partition scheme


// WiFi Stuff ---------
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
WebServer   Server;
AutoConnect Portal(Server);
WiFiClient client;
#define touchMapServer        "designcards.mooo.com"
#define touchMapServerPort    3000


// BLE Stuff ---------
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// TODO:
// - Load these variables continuously from server.
#define PROXIMITY_LIMIT_RSSI  -60
#define MAX_CLOSE_DEVICES     10
char myMACAddress[25];
// TODO: 
// - figure out how to quicken the scan time without crashing. 3 seems to be the max.
// - use a queue?
byte scanTime = 5; // In seconds.
BLEScan* pBLEScan;
bool runningScan = false;
byte deviceCounter = 0;
BLEAdvertisedDevice closeDevices[MAX_CLOSE_DEVICES];
BLEAdvertisedDevice previousCloseDevices[MAX_CLOSE_DEVICES];
BLEAdvertisedDevice nullDevice = BLEAdvertisedDevice();

// Capacitive Touch Stuff ---------
const int TOUCH_SENSOR_THRESHOLD = 80;
int touchSensor1Value = 0;
int touchSensor2Value = 0;
int touchSensor3Value = 0;
boolean touch1Start = false;
boolean touch2Start = false;
boolean touch3Start = false;



const byte LED_PIN = 5; // Thing's onboard LED



#define DEBUG     false



class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveRSSI()) {
        if ((int)advertisedDevice.getRSSI() > PROXIMITY_LIMIT_RSSI) {
          //          Serial.printf("Advertised Device: %s", advertisedDevice.toString().c_str());
          //          Serial.printf(", Rssi: %d \n", (int)advertisedDevice.getRSSI());
          if (deviceCounter < MAX_CLOSE_DEVICES) {
            closeDevices[deviceCounter] = advertisedDevice;
            deviceCounter++;
          }
        }
      }
    }
};



void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  connectWiFi();
  registerExhibit();
  setupBLE();
  clearDevices(previousCloseDevices);
  clearDevices(closeDevices);
  runBLEScan();
}



void loop() {
  Portal.handleClient();
  readSensors();
  senseTouchEvents();

  // TODO:
  // - Handle connection queue
  //   - If there's something in the queue, deal with it in priority order
  //   - One connection per loop?
  
  // if there's incoming data from the net connection.
  // send it out the serial port. This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    if (DEBUG) Serial.write(c);
  }
}



void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}



void connectWiFi()
{
  Server.on("/", rootPage);
  if (Portal.begin()) {
    if (DEBUG) Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}



void setupBLE() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
}



void runBLEScan() {
  if (DEBUG) Serial.println("Scanning BLE...");
  clearDevices(previousCloseDevices);
  bleadCopy(closeDevices, previousCloseDevices, MAX_CLOSE_DEVICES);
  clearDevices(closeDevices);
  deviceCounter = 0;
  runningScan = true;
  pBLEScan->start(scanTime, scanComplete);
}



void scanComplete(BLEScanResults foundDevices) {
  runningScan = false;
  if (DEBUG) sortDevices(closeDevices, deviceSortByRSSI, MAX_CLOSE_DEVICES);
  if (DEBUG) Serial.println("closeDevices:");
  if (DEBUG) printDeviceList(closeDevices, MAX_CLOSE_DEVICES);
  if (DEBUG) Serial.println("previousCloseDevices:");
  if (DEBUG) printDeviceList(previousCloseDevices, MAX_CLOSE_DEVICES);
  handleBLEProximity();
  pBLEScan->clearResults();
  if (DEBUG) Serial.println("");
  runBLEScan();
}



BLEAdvertisedDevice *arrSubtract(BLEAdvertisedDevice a[], BLEAdvertisedDevice b[]) {
  if (DEBUG) Serial.println("Got to arrSubtract");
  BLEAdvertisedDevice combined[MAX_CLOSE_DEVICES*2];
  BLEAdvertisedDevice duplicates[MAX_CLOSE_DEVICES];
  byte duplicatesIndex = 0;
  static BLEAdvertisedDevice differences[MAX_CLOSE_DEVICES];
  byte differencesIndex = 0;
  
  clearDevices(duplicates);
  clearDevices(differences);
  
  if (DEBUG) Serial.println("b:");
  if (DEBUG) printDeviceList(b, MAX_CLOSE_DEVICES);
  
  // Combine a + b
  for (byte i = 0; i < MAX_CLOSE_DEVICES; i++) {
    combined[i] = a[i];
  }
  for (byte i = 0; i < MAX_CLOSE_DEVICES; i++) {
    combined[MAX_CLOSE_DEVICES+i] = b[i];
  }
  if (DEBUG) Serial.println("Combined a + b");
  
  sortDevices(combined, deviceSortByAddress, MAX_CLOSE_DEVICES*2);
  
  // Find duplicates
  for (byte i = 0; i < MAX_CLOSE_DEVICES*2 - 1; i++) {
    if (combined[i].getAddress().toString() == combined[i+1].getAddress().toString()) {
      duplicates[duplicatesIndex] = combined[i];
      duplicatesIndex++;
      i++;
    }
  }
  if (DEBUG) Serial.printf("Searched for duplicates. Found %d:\n", deviceCount(duplicates));
  if (DEBUG) printDeviceList(duplicates, MAX_CLOSE_DEVICES);
  
  // Perform subtraction by storing non-duplicates
  boolean isDuplicate;
  for (byte i = 0; i < MAX_CLOSE_DEVICES; i++) {
    if (a[i].getAddress().toString() == nullDevice.getAddress().toString()) continue; // Ignore nullDevices
    isDuplicate = false;
    for (byte j = 0; j < MAX_CLOSE_DEVICES; j++) {
      if (duplicates[j].getAddress().toString() == nullDevice.getAddress().toString()) continue; // Ignore nullDevices
      if (a[i].getAddress().toString() == duplicates[j].getAddress().toString()) {
        if (DEBUG) Serial.println("Duplicate found.");
        isDuplicate = true;
      }
    }
    if (!isDuplicate) {
      differences[differencesIndex] = a[i];
      differencesIndex++;
    }
  }
  if (DEBUG) Serial.println("Performed subtraction");
  
  return differences;
}



void printDeviceList(BLEAdvertisedDevice a[], byte arrLen) {
  byte maxDevices = deviceCount(a);
  byte deviceCounter = 0;
  for (byte i = 0; i < arrLen - 1; i++) {
    if (a[i].getAddress().toString() != nullDevice.getAddress().toString()) {
      deviceCounter++;
      Serial.print(a[i].getAddress().toString().c_str());
      if (deviceCounter != maxDevices) Serial.print(", ");
    }
  }
  if (arrLen > 0 && a[arrLen - 1].getAddress().toString() != nullDevice.getAddress().toString()) {
    Serial.print(a[arrLen - 1].getAddress().toString().c_str());
  }
  Serial.println("");
}



void handleBLEProximity() {
  if (!runningScan) {
    byte numCloseDevices = deviceCount(closeDevices);
    
    if (numCloseDevices == 0) {
      Serial.println("No devices close by.");
      return;
    }
    
    sortDevices(closeDevices, deviceSortByRSSI, MAX_CLOSE_DEVICES);
    
    Serial.println(deviceCount(closeDevices) + (String)" device(s) close by.");
    Serial.printf("Closest device: %s\n", closeDevices[0].getAddress().toString().c_str());
    
    if (DEBUG) Serial.print("closeDevices: ");
    if (DEBUG) printDeviceList(closeDevices, MAX_CLOSE_DEVICES);
    if (DEBUG) Serial.print("\n");
    if (DEBUG) Serial.print("previousCloseDevices: ");
    if (DEBUG) printDeviceList(previousCloseDevices, MAX_CLOSE_DEVICES);
    if (DEBUG) Serial.print("\n");
    
    BLEAdvertisedDevice *newDevices = arrSubtract(closeDevices, previousCloseDevices);
    Serial.print("Number of new devices: ");
    Serial.println(deviceCount(newDevices));
    if (DEBUG) Serial.print("New devices: ");
    if (DEBUG) printDeviceList(newDevices, MAX_CLOSE_DEVICES);
    if (DEBUG) Serial.print("\n");

    
    BLEAdvertisedDevice *lostDevices = arrSubtract(previousCloseDevices, closeDevices);
    Serial.print("Number of devices lost: ");
    Serial.println(deviceCount(lostDevices));
    if (DEBUG) Serial.print("Lost devices: ");
    if (DEBUG) printDeviceList(lostDevices, MAX_CLOSE_DEVICES);
    if (DEBUG) Serial.print("\n");
    
    postBLEScanData();
  }
}



void readSensors() {
  byte numberOfReadings = 10;
  
  touchSensor1Value = 0;
  touchSensor2Value = 0;
  touchSensor3Value = 0;
  
  for (byte i = 0; i < numberOfReadings; i++)
  {
    touchSensor1Value += touchRead(T0);
    touchSensor2Value += touchRead(T2);
    touchSensor3Value += touchRead(T3);
  }
  
  touchSensor1Value = touchSensor1Value / numberOfReadings;
  touchSensor2Value = touchSensor2Value / numberOfReadings;
  touchSensor3Value = touchSensor3Value / numberOfReadings;
}



void senseTouchEvents() {
  sortDevices(closeDevices, deviceSortByRSSI, MAX_CLOSE_DEVICES);
  if (touchSensor1Value < TOUCH_SENSOR_THRESHOLD && !touch1Start) {
    Serial.print("Touch1 start (");
    Serial.print(touchSensor1Value);
    Serial.println(")");
    touch1Start = true;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 1, ";
    payload += "\"buttonState\": \"down\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }
  else if (touchSensor1Value > TOUCH_SENSOR_THRESHOLD && touch1Start) {
    Serial.println("Touch1 end");
    touch1Start = false;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 1, ";
    payload += "\"buttonState\": \"up\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }
  if (touchSensor2Value < TOUCH_SENSOR_THRESHOLD && !touch2Start) {
    Serial.print("Touch2 start (");
    Serial.print(touchSensor2Value);
    Serial.println(")");
    touch2Start = true;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 2, ";
    payload += "\"buttonState\": \"down\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }
  else if (touchSensor2Value > TOUCH_SENSOR_THRESHOLD && touch2Start) {
    Serial.println("Touch2 end");
    touch2Start = false;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 2, ";
    payload += "\"buttonState\": \"up\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }

  if (touchSensor3Value < TOUCH_SENSOR_THRESHOLD && !touch3Start) {
    Serial.print("Touch3 start (");
    Serial.print(touchSensor3Value);
    Serial.println(")");
    touch3Start = true;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 3, ";
    payload += "\"buttonState\": \"down\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }
  else if (touchSensor3Value > TOUCH_SENSOR_THRESHOLD && touch3Start) {
    Serial.println("Touch3 end");
    touch3Start = false;
    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"deviceString\": \"" + String(closeDevices[0].toString().c_str()) + "\",";
    payload += "\"buttonID\": 3, ";
    payload += "\"buttonState\": \"up\"";
    payload += "}]";
    postJSONData("/methods/touchevents.addEvent", payload);
  }
}



void sortDevices(BLEAdvertisedDevice devices[], int8_t (*comparator)(BLEAdvertisedDevice &a, BLEAdvertisedDevice &b), byte arrLen) {
  boolean swapped;
  BLEAdvertisedDevice temp;
  do
  {
      swapped = false;
      for (byte i = 0; i < arrLen - 1; i++)
      {
          if (DEBUG) Serial.print("Loop ");
          if (DEBUG) Serial.println(i);
          if (comparator(devices[i], devices[i+1]) > 0)
          {
              temp = devices[i];
              devices[i] = devices[i + 1];
              devices[i + 1] = temp;
              swapped = true;
          }
      }
  } while (swapped);
}



int8_t deviceSortByAddress(BLEAdvertisedDevice &a, BLEAdvertisedDevice &b) {
  int8_t result = 0;
  if (DEBUG) Serial.print("Comparing ");
  if (DEBUG) Serial.print(a.getAddress().toString().c_str());
  if (DEBUG) Serial.print(" to ");
  if (DEBUG) Serial.print(b.getAddress().toString().c_str());
  if (DEBUG) Serial.print(": ");
  if (b.getAddress().toString() == "00:00:00:00:00:00") result = -1; // Keep empty devices at the end of list.
  else if (a.getAddress().toString() == "00:00:00:00:00:00") result = 1; // Keep empty devices at the end of list.
  else result = strcmp(a.getAddress().toString().c_str(),
                      b.getAddress().toString().c_str());
  if (DEBUG) Serial.println(result);
  return result;
}

int8_t deviceSortByRSSI(BLEAdvertisedDevice &a, BLEAdvertisedDevice &b) {
  int8_t result = 0;
  if (DEBUG) Serial.print("Comparing ");
  if (DEBUG) Serial.print(a.getAddress().toString().c_str());
  if (DEBUG) Serial.print(" (");
  if (DEBUG) Serial.print(a.getRSSI());
  if (DEBUG) Serial.print(") to ");
  if (DEBUG) Serial.print(b.getAddress().toString().c_str());
  if (DEBUG) Serial.print(" (");
  if (DEBUG) Serial.print(b.getRSSI());
  if (DEBUG) Serial.print("): ");
  
  if (a.getRSSI() > b.getRSSI()) result = -1;
  else if (a.getRSSI() < b.getRSSI()) result = 1;
  
  if (DEBUG) Serial.println(result);
  return result;
}



void clearDevices(BLEAdvertisedDevice devices[]) {
  for (byte i = 0; i < MAX_CLOSE_DEVICES; i++) {
    if (DEBUG) Serial.printf("Clearing device %d\n", (int)i);
    devices[i] = nullDevice;
  }
}



byte deviceCount(BLEAdvertisedDevice devices[]) {
  byte count = 0;
  for (byte i = 0; i < MAX_CLOSE_DEVICES; i++) {
    if (devices[i].getAddress().toString() != nullDevice.getAddress().toString())
    {
      count++;
    }
  }
  return count;
}


void bleadCopy(BLEAdvertisedDevice arrayOriginal[], BLEAdvertisedDevice arrayCopy[], byte arraySize){ //Copy function
  for(byte i=0; i<arraySize; i++){
    arrayCopy[i]=arrayOriginal[i];  
  }
}

// TODO:
// - Use a connection queue with high priority
bool postJSONData(String route, String payload) {
  if (DEBUG) Serial.println("postJSONData");
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  
  // if there's a successful connection:
  if (client.connect(touchMapServer, touchMapServerPort) > 0) {
    Serial.print("Connected... ");
    // send the HTTP POST request:
    
    // Build HTTP request.
    String toSend = "POST ";
    toSend += route;
    toSend += " HTTP/1.1\r\n";
    toSend += "Host:";
    toSend += touchMapServer;
    toSend += "\r\n" ;
    toSend += "Content-Type: application/json\r\n";
    toSend += "User-Agent: Arduino\r\n";
    toSend += "Accept-Version: ~0\r\n";
    toSend += "Content-Length: "+String(payload.length())+"\r\n";
    toSend += "\r\n";
    toSend += payload;
    if (DEBUG) Serial.println(toSend);
    client.println(toSend);
    Serial.println("sent.");
    return true;
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    return false;
  }
}

// TODO:
// - Use a connection queue with low priority
bool postBLEScanData() {
  if (DEBUG) Serial.println("postBLEScanData");
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  
  // if there's a successful connection:
  if (client.connect(touchMapServer, touchMapServerPort) > 0) {
    Serial.print("Connected... ");
    // send the HTTP POST request:
    byte numDevices = deviceCount(closeDevices);
    
    // Build HTTP request.
    String toSend = "POST /methods/exhibitdevices.addSample HTTP/1.1\r\n";
    toSend += "Host:";
    toSend += touchMapServer;
    toSend += "\r\n" ;
    toSend += "Content-Type: application/json\r\n";
    toSend += "User-Agent: Arduino\r\n";
    toSend += "Accept-Version: ~0\r\n";

    String payload = "[{\"exhibitMACAddress\": \"" + String(myMACAddress) + "\", ";
    payload += "\"devices\":[";
    
    for (byte i = 0; i < numDevices; i++){
      payload += "{\"address\": \"" + String(closeDevices[i].getAddress().toString().c_str()) + "\",";
//      payload += "\"serviceDataUUID\": \"" + String(closeDevices[i].getServiceDataUUID().toString().c_str()) + "\",";
//      payload += "\"serviceUUID\": \"" + String(closeDevices[i].getServiceUUID().toString().c_str()) + "\",";
//      payload += "\"name\": \"" + String(closeDevices[i].getName().c_str()) + "\",";
      payload += "\"string\": \"" + String(closeDevices[i].toString().c_str()) + "\",";
//      payload += "\"manufacturerData\": \"" + String(closeDevices[i].getManufacturerData().c_str()) + "\",";
      // See: https://forum.arduino.cc/index.php?topic=626200.0
      payload +="\"signalStrength\": " + String(closeDevices[i].getRSSI()) + "}";
      if (i < numDevices - 1)
        payload += ",";
    }
    
    payload += "]}]";
    
    toSend += "Content-Length: "+String(payload.length())+"\r\n";
    toSend += "\r\n";
    toSend += payload;
    if (DEBUG) Serial.println(toSend);
    client.println(toSend);
    Serial.println("sent.");
    return true;
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    return false;
  }
}

void registerExhibit() {
  uint8_t address[6];
  esp_efuse_mac_get_default(address);
  Serial.println("");
  Serial.printf("My MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n", address[0], address[1], address[2], address[3], address[4], address[5]);
  sprintf(myMACAddress, "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
  postJSONData("/methods/registerexhibit", "[{\"macAddress\": \"" + String(myMACAddress) +"\"}]");
}
