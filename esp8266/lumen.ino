/////////////////////////////////////////////////////////////////////////////////////////////////
// Reading the Analog Port and sending messgages via HTTP POST to a Cloud Messaging Service
/////////////////////////////////////////////////////////////////////////////////////////////////
//
// ESP8266 based hardware, e.g. ADAFRUIT HUZZAH ESP8266 BREAKOUT
// https://www.adafruit.com/product/2471
// Reading the Analog Port and sending messgages via HTTP POST,
// if certain thresholds are passed
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>

// WIFI Credentials
const char* ssid     = "[WIFI SSID]";
const char* password = "[ROUTER Password]";

// HTTP ADDRESS
const char* host = "gcm-http.googleapis.com";
const int httpPort = 80;
const String path = "/gcm/send";
const String apikey  = "key=[Google Project API KEY]";

// Message parts
const char* p0= "{\"to\":\"/topics/";
const char* p1= "\",\"notification\":{\"title\":\"Light Sensor\",\"body\":\"";
const char* p2= "\",\"icon\":\"ic_lightbulb_";
const char* p3= "\"},\"ttl\":";
const char* p4 = ",\"collapse_key\":\"lumen8266\"}";

// PORTS
const int SENSOR_PIN = A0; // lightsensor on the Analog Input
const int LED_PIN = 0;     // for indicating activity
const int SLEEP = 1000;   // number of ms before next sensor poll

// THRESHOLDS very much sensor and environment dependent
const int D1 = 50;  // small fluctuation
const int D2 = 100; // medium fluctuation
const int D3 = 150; // large fluctuation

// [0 .. darkness .. natural .. aritifical .. 1023]
const int THRESHOLD0 = 10;  // 0 .. darkness
const int THRESHOLD1 = 250; // artificial .. 1023
const int VARIANCE = 15; // desensitize the sensor

// Global Vars
int k0; // last sensor reading

/////////////////////////////////////////////////////////////////////////////////////////////////
// Setup, runs only once, when device boots up
/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(10);

  // Connecting to a WiFi network
  Serial.println();
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // initialial sensor reading
  k0 = analogRead(SENSOR_PIN);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Interpret sensor data and return true on sig. change
/////////////////////////////////////////////////////////////////////////////////////////////////
void interpretData(int k1) {
  String topic = NULL; // one of {D1,D2,D3,dark,naturalartificial}
  String cond  = NULL; // describing the current light condition
  char icon  = 'r';    // icon name associated w/ condition, needs to be avail. in app
  int d = abs(k1 - k0);

  Serial.print("Sensor Value: ");
  Serial.println(k1);
  if (d < VARIANCE) {
    return;
  }

  if (d >= D1) {
    topic = "d1";
    cond = "Small Fluctuation";
  }
  if (d >= D2) {
    topic = "d2";
    cond = "Medium Fluctuation";
  }
  if (d >= D3) {
    topic = "d3";
    cond = "Large Fluctuation";
  }

  if (topic != NULL) {
    cond = cond + " " + k0 + " -> " + k1;
    sendNotification(topic, cond, icon, 0);
    topic= NULL;
  }

  if (k0 < THRESHOLD1 &&  THRESHOLD1 < k1) {
    // natural to artificial
    topic = "artificial";
    icon = 'y';
    cond = "Artificial Lighting";
  } else if (k0 > THRESHOLD1 && THRESHOLD1 > k1) {
    // artificial to natural
    topic = "natural";
    icon = 'g';
    cond = "Natural Lighting";
  } else if (k0 < THRESHOLD0 && THRESHOLD0 < k1) {
    // dark to natural or artificial
    if (k1 < THRESHOLD1) {
      topic = "natural";
      icon = 'g';
      cond = "Natural Lighting";
    } else {
      topic = "artificial";
      icon = 'y';
      cond = "Artificial Lighting";
    }
  } else if (k0 > THRESHOLD0 && THRESHOLD0 > k1) {
    // natural or artificial to darkness
    topic = "dark";
    cond = "Absence of Light";
  }
  if (topic != NULL) {
    cond = cond + " " + k0 + " -> " + k1;
    sendNotification(topic, cond, icon, 900); // 15 minutes
  }

  k0 = k1;
  Serial.println(topic + "\n" + cond);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Send important change in sensor reading to remote server
// using the WiFiClient class to create a TCP connections
// topic is basically the address, the text will be sent to.
// ttl is the number of 0 seconds the server will try to deliver the msg, no more than 4 weeks.
// a valuie of 0 means that the message will be discarded, if can't be delivered immediately.
/////////////////////////////////////////////////////////////////////////////////////////////////

void sendNotification(String topic, String text, char icon, int ttl) {
  WiFiClient client;
  String payload = p0 + topic + p1 + text + p2 + icon + p3 + ttl + p4;
  String request= String("POST ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n" +
               "Authorization: " + apikey + " \r\n" +
               "Content-Type: application/json \r\n" +
               "Content-Length: " + payload.length() + " \r\n" +
               "\r\n" + payload;
  payload= NULL;
  Serial.print("Requesting URL: ");
  Serial.println(path);
  Serial.println(request);
  Serial.println();

  // Send the request to the server
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  client.print(request);
  request= NULL;
  delay(2000);

  // Print to reply from the server to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Main loop, read/interpret/send sensor data, sleep, start-over
/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  interpretData(analogRead(SENSOR_PIN));
  digitalWrite(LED_PIN, HIGH);
  delay(SLEEP);
  digitalWrite(LED_PIN, LOW);
}
