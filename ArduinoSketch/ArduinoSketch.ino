#include <Arduino.h>
#include <HTS221Sensor.h>
#include "RGB_LED.h"
#include <AZ3166WiFi.h>
#include "LPS22HBSensor.h"
#include "SystemTime.h"
#include "AudioClassV2.h"
#include "MQTTClient.h"
#include "MQTTNetwork.h"

/*************************************************
    THESE SETTINGS MUST BE CHANGED TO
    BE ABLE TO CONNECT TO YOUR WIFI
    AND THE PC WHERE THE ROOMZ-STACK IS RUNNING ON
 *********************************************/

const char* mqttServer = "wolfram";
char* SSID = "<MY WIFI SSID>";
char* KEY = "<MY WIFI KEY>";

char* room = "bedroom";

int port = 1883;
char* topic = "room-status";

DevI2C *i2c;
HTS221Sensor *sensor;
LPS22HBSensor *psensor;

// There is still a lot of dummy/unused-code in here. Will clean this up someday

static RGB_LED rgbLed;
static int interval = 1000;
static float humidity;
static float temperature;
static const char* currentFirmwareVersion = "1.0.0";
static int USERLED = PC_13;
static int WIFILED = PB_2;
static bool hasWifi = false;
static const int BTN_A = PA_4;
static const int BTN_B = PA_10;

int arrivedcount = 0;

MQTTNetwork mqttNetwork;
MQTT::Client<MQTTNetwork, Countdown> client = MQTT::Client<MQTTNetwork, Countdown>(mqttNetwork);
#define RGB_LED_BRIGHTNESS 32

int getInterval()
{
  return interval;
}

void blinkLED()
{
  rgbLed.turnOff();
  rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
  delay(500);
  rgbLed.turnOff();
}

void blinkSendConfirmation()
{
  rgbLed.turnOff();
  rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
  delay(500);
  rgbLed.turnOff();
}

float readTemperature()
{
  sensor->reset();

  float temperature = 0;
  sensor->getTemperature(&temperature);

  return temperature;
}

float readHumidity()
{
  sensor->reset();

  float humidity = 0;
  sensor->getHumidity(&humidity);

  return humidity;
}

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;

  char msgInfo[60];
  sprintf(msgInfo, "Message arrived: qos %d, retained %d, dup %d, packetid %d", message.qos, message.retained, message.dup, message.id);
  Serial.println(msgInfo);

  sprintf(msgInfo, "Payload: %s", (char*)message.payload);
  Serial.println(msgInfo);
  ++arrivedcount;
}


void initMQTT() {
  Serial.println("Connecting to MQTT");

  int rc = mqttNetwork.connect(mqttServer, port);
  if (rc != 0) {
    Serial.println("Could not connect to MQTT");
    Screen.print(0, "NO MQTT");
  } else {
    Serial.println("Connected to MQTT");
  }

  Serial.println("Trying to register as Client");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = room;
  //data.clientID.cstring+= WiFi.localIP().get_address();
  data.username.cstring = "testuser";
  data.password.cstring = "testpassword";

  if ((rc = client.connect(data)) != 0) {
    Serial.println("MQTT client connect to server failed");
  } else {
    Serial.println("Registered as client");
  }

  // Not used by now... TODO
  if ((rc = client.subscribe("room-control", MQTT::QOS2, messageArrived)) != 0) {
    Serial.println("MQTT client subscribe from server failed");
  } else {
    Serial.println("Subscribed to topic");
  }
}

static void InitWifi()
{
  Screen.print(0, "ROOMZ BOOTING");
  Screen.print(2, "Connecting WIFI");

  rgbLed.turnOff();

  rgbLed.setColor(0, 0, 255);

  if (WiFi.begin(SSID, KEY) == WL_CONNECTED)
  {
    rgbLed.turnOff();
    rgbLed.setColor(0, 255, 0 );
    delay(500);
    rgbLed.turnOff();
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(0, ip.get_address());

  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
    for (int i = 0; i < 5; i++) {
      rgbLed.setColor(255, 0, 0);
      delay(200);
      rgbLed.turnOff();
      delay(200);
    }
    rgbLed.setColor(255, 0, 0);
  }
  initMQTT();
  digitalWrite(WIFILED, LOW);
}

void setup()
{
  //  Screen.init();
  Serial.begin(115200);
  Serial.println("ROOMZ Booting");

  pinMode(BTN_A, INPUT);
  pinMode(BTN_B, INPUT);


  pinMode(USERLED, OUTPUT);
  digitalWrite(USERLED, LOW);
  pinMode(WIFILED, OUTPUT);
  digitalWrite(WIFILED, LOW);


  InitWifi();
  Screen.print(1, "initializing....");

  i2c = new DevI2C(D14, D15);
  sensor = new HTS221Sensor(*i2c);
  sensor->init(NULL);

  psensor = new LPS22HBSensor(*i2c);
  psensor->init(NULL);

  humidity = -1;
  temperature = -1000;
}

long last = 0L;
bool last_A, last_B;
int sent = 0;
bool showscreen = true;
float t, h, pressure;
void show() {
  char line1[20];
  Screen.print(0,  WiFi.localIP().get_address());
  sprintf(line1, "Temp: %.1f deg", t);
  Screen.print(1, line1);
  sprintf(line1, "Hum: %.0f %%", h);
  Screen.print(2, line1);
  sprintf(line1, "Prs: %.0f mBar", pressure);
  Screen.print(3, line1);
}

void loop()
{

  bool cur_a = digitalRead(BTN_A);
  bool cur_b = digitalRead(BTN_B);

  if (cur_a != last_A) {
    last_A = cur_a;
    if (cur_a == 0) {
      showscreen = !showscreen;
    }
    if (!showscreen) {
      Screen.clean();
    } else {
      show();
    }
  }

  if (cur_b != last_B) {
    last_B = cur_b;
    Serial.println(cur_b);
  }

  // Do this every 10secs
  if (millis() - last > 10000L) {

    // Check if we lost WIFI
    if (WiFi.status() != WL_CONNECTED) {
      // Reconnect WIFI /MQTT
      InitWifi();
      delay(1000);
    }

    // Check if we lost MQTT only
    if (!client.isConnected()) {
      mqttNetwork.disconnect();
      delay(1000);
      initMQTT();
    }

    // digitalWrite(USERLED, HIGH);
    //digitalWrite(WIFILED, HIGH);

    t = readTemperature();
    h = readHumidity();

    psensor->getPressure(&pressure);

    if (showscreen == true) {
      show();
    }

    digitalWrite(USERLED, LOW);
    digitalWrite(WIFILED, LOW);

    char buf[100];
    MQTT::Message message;
    message.retained = false;
    message.dup = false;
    sprintf(buf, "{\"room\":\"%s\",\"index\":%d,\"temp\":%.2f,\"hum\":%.2f,\"pressure\":%.0f}", room, sent, t, h, pressure);
    sent++;
    message.qos = MQTT::QOS0;
    message.payloadlen = strlen(buf) ;
    message.payload = (void*)buf;
    int rc = client.publish(topic, message);
    if (rc == -1) {
      mqttNetwork.disconnect();
      delay(1000);
    }
    initMQTT();
    last = millis();
  }
  client.yield(100);
}
