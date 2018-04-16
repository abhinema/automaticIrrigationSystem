/***************************************************
  Automatic Irrigation System
  with ESP8266 Sensor Module
  

 ****************************************************/

// Libraries
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

// DHT 11 sensor
#define DHTPIN 2       //GPIO2 
#define DHTTYPE DHT11  // Use DHT22 if you have DHT22 sensor

// WiFi parameters
#define WLAN_SSID       "<__FILL_SSID__>"
#define WLAN_PASS       "<__FILL_PASSWORD__>"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "<__FILL_AIO_USER_NAME__>"
#define AIO_KEY         "<__FILL_AIO_KEY__>"

// DHT sensor
DHT dht(DHTPIN, DHTTYPE, 15);

// Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;


// Store the MQTT server, client ID, username, and password in flash memory.
const char MQTT_SERVER[]     = AIO_SERVER;  //PROGMEM
 
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[]   = AIO_KEY __DATE__ __TIME__;  //Removed PROGMEM to avoid stacktrace
const char MQTT_USERNAME[]   = AIO_USERNAME;               //Removed PROGMEM to avoid stacktrace 
const char MQTT_PASSWORD[]   = AIO_KEY;                    //Removed PROGMEM to avoid stacktrace

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup feeds for temperature & humidity
const char TEMPERATURE_FEED[]  = AIO_USERNAME "/feeds/temperature";  //PROGMEM_1
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[]  = AIO_USERNAME "/feeds/humidity";  //PROGMEM_1
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

/* Added soil moisture feed */
const char SOILMOISTURE_FEED[]  = AIO_USERNAME "/feeds/soilMoisture";  //PROGMEM_1
Adafruit_MQTT_Publish soilmoisture = Adafruit_MQTT_Publish(&mqtt, SOILMOISTURE_FEED);

/* Added support for Soil Moisture Sesnsor */
int PUMP= D0; // PUMP Indicator at Digital PIN D0
int sense_Pin= 0; // Soil Sensor input at Analog PIN A0

/*************************** Sketch Code ************************************/
void setup() {

  // Init sensor
  dht.begin();

  Serial.begin(115200);
  Serial.println(F("Adafruit IO Example"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  // connect to adafruit io
  connect();
  // Set PUMP to output 
  pinMode(PUMP, OUTPUT);
  delay(2000);
}

void loop() {

  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // Grab the current state of the sensor
  int humidity_data = (int)dht.readHumidity();
  int temperature_data = (int)dht.readTemperature();
  Serial.print("MOISTURE LEVEL : ");
  int soilmoisture_data = analogRead(sense_Pin);
  soilmoisture_data = soilmoisture_data/(10*2);
  Serial.println(soilmoisture_data);
  if(soilmoisture_data<40)  {
      digitalWrite(PUMP, HIGH);//Pump OFF
   }  else   {
      digitalWrite(PUMP,LOW); //PUMP ON
   }

  // Publish data
  if (! temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));

  if (! humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));

  if (! soilmoisture.publish(soilmoisture_data))
    Serial.println(F("Failed to publish soilmoisture"));
  else
    Serial.println(F("soilmoisture published!"));
  // Repeat every 10 seconds
  delay(10000);
}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);
  }

  Serial.println(F("Adafruit IO Connected!"));
}
/* End Of File */
