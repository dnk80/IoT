
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
//#include <NTPClient.h> 
#include "NTPClient.h"

 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 
const char* message_loop;
int count_gl;
int size_gl;
int led_gl;
const int timeZone = 0; 
const char* ntp_primary = "pool.ntp.org";
const char* ntp_secondary = "time.nist.gov";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_primary, timeZone);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);


void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setBufferSize(1500);  //Set packege size 1500, deafault was 128
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

// Message format
// {
//   "message": "Hello from AWS IoT console", "count":"01", "size":"100"
// }

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
  // const char* count = doc["count"];
  int count = doc["count"];
  count_gl = count;
  Serial.print(" Count is: ");
  Serial.println(count);
  int size = doc["size"];
  size_gl = size;
  Serial.print(" Size is: ");
  Serial.println(size);
  int led = doc["led"];
  led_gl = led;
}
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
//  dht.begin();
  timeClient.begin();
  pinMode(22, OUTPUT); // Set GPIO22 as digital output pin
}
 
void loop()
{
 
  if (led_gl == 1) {
    digitalWrite(22, HIGH);
  }
  else if (led_gl == 0) {
    digitalWrite(22, LOW);
  }


  timeClient.update(); // Update the time
  unsigned long epochTime = timeClient.getEpochTime();
  unsigned long milliseconds = millis() % 1000;
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted time: ");
  Serial.println(formattedTime);
  Serial.println(formattedTime + "." + milliseconds);
  Serial.print("Epoch time: ");
  Serial.println(epochTime);
  Serial.print("Milliseconds: ");
  Serial.println(milliseconds);
  //String dtmf = formattedTime + "." + milliseconds;
  publishMessageSerial(count_gl,size_gl,generateMessage(size_gl));
  publishMessageAWS(count_gl,size_gl,generateMessage(size_gl));
  client.loop();

  delay(1000);
}

//          Function Part         //
void publishMessageAWS(int count, int size, String message) {
  if (count > 0 && count < 1000 && size > 0) {
    for (int i = 0; i < count; i++) {
      StaticJsonDocument<1024> doc;
      //DynamicJsonDocument doc(512);
      doc["size"] = size_gl;
      doc["package"] = message;
      doc["epochtime"] = timeClient.getEpochTime();
      //doc["milliseconds"] = millis() % 1000;
      //int ms = timeClient.get_millis() % 1;
      doc["milliseconds"] = (int)timeClient.get_millis();
      char jsonBuffer[2048];
      //char jsonBuffer[512];
      serializeJson(doc, jsonBuffer); // print to client
      client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    }
    size_gl=0;
    count_gl=0;
  }
}
//          Function Part         //
void publishMessageSerial(int count, int size, String message) {
  if (count > 0 && count < 1000 && size > 0) {
    for (int i = 0; i < count; i++) {
      Serial.println(message);
    }
  }
  // else {
  //   Serial.println("Wrong Count Number");
  // }
}
//          Function Part         //
String generateMessage(int length) {
  String message = "";
  if (length > 0 && length <= 1460) {
    for (int i = 0; i < length; i++) {
      // Generate random ASCII symbol ('A' до 'Z')
      char randomChar = 'A';
      //char randomChar = random('A', 'Z' + 1);
      message += randomChar;
    }
  }
  // else {
  //   Serial.println("Wrong Length Size");
  // }
  return message;
}
