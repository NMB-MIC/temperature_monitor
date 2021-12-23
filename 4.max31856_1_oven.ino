#include <ESP8266WiFi.h>
#include <Adafruit_MAX31856.h>
#include <PubSubClient.h>

Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(D3, D4, D6, D8); // Use software SPI: CS, DI, DO, CLK

//Constant WiFi
const char* ssid = "TP-Link_F3B4";
const char* password = "72199657";
const char* mqtt_server = "192.168.1.100"; //เปลี่ยนเป็นIPเครื่อง

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

float value1 = 0;
float value2 = 0;
float value3 = 0;
char cstr1[16];
char cstr2[16];
char cstr3[16];

//---------------------------------------------------setup_wifi()----------------------------------------------------------
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

//---------------------------------------------------reconnect()---------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


//---------------------------------------------------setup()---------------------------------------------------
void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Booting");
  while (!Serial) delay(10);
  Serial.println("MAX31856 thermocouple test");

  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }

}

//---------------------------------------------------Loop()---------------------------------------------------
void loop() {
  maxthermo.triggerOneShot();
  Serial.print("Thermocouple Temp: ");
  Serial.println(maxthermo.readThermocoupleTemperature());

  delay(1000);
  // check for conversion complete and read temperature
  if (maxthermo.conversionComplete()) {
    Serial.println(maxthermo.readThermocoupleTemperature()); //read convert Temp value
  } else {
    Serial.println("Conversion not complete!");
  }
  Serial.print("Cold Junction Temp: ");
  Serial.println(maxthermo.readCJTemperature());
  Serial.print("Thermocouple Temp: ");
  Serial.println(maxthermo.readThermocoupleTemperature());

  // Check and print any faults
  uint8_t fault = maxthermo.readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault"); return;
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  }
  delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  // Publishes new temperature and humidity every 2 seconds
  if (now - lastMsg > 2000) {
    lastMsg = now;

    float h = maxthermo.readCJTemperature();
    float c = maxthermo.readThermocoupleTemperature();

    Serial.print("value: ");
    Serial.print(h);
    Serial.print(" / ");
    Serial.print(c);
    Serial.print(" / ");

    // --------------------------------------------------------
    client.publish("area", "Over_test");
    client.publish("Cold_Junction_Temp", dtostrf(h, 4, 2, cstr1));
    client.publish("Thermocouple_Temp", dtostrf(c, 4, 2, cstr2));
    //client.publish("outTopic_f", dtostrf(f,4,2,cstr3));
  }

}
