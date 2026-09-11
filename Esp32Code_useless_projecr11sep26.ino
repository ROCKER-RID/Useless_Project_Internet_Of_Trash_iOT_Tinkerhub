#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include "esp_timer.h"

// ================= AP CONFIG =================
const char* ap_ssid = "SyncSpace_Setup_Useless";
const char* ap_password = "12345678";

// ================= RESET BUTTON =================
#define RESET_BUTTON 0

// ================= WEB SERVER & DNS =================
WebServer server(80);
DNSServer dnsServer;    
Preferences preferences;
const byte DNS_PORT = 53; 

// ================= MQTT =================
WiFiClient espClient;
PubSubClient client(espClient);

String mqttIP = "";
IPAddress mqtt_server_ip;

// ================= WIFI LIST =================
String ssidList = "";
bool setupMode = false;

// ================= RELAY & TRIAC PINS =================
const int relayPin = 27; // Single relay control on pin 27

const int triacPins[3] = {4, 16, 17};
const int zcdPin = 14;

// Index for the 3rd Triac (Halogen channel)
const int HALOGEN_CH = 2;

// ================= STATES =================
bool relayState = false;
int fanSpeed[3] = {0, 0, 0};
volatile int activeSpeed[3] = {0, 0, 0}; 
bool ghostMode = false;                  

// Safety timer & 50ms ticker
unsigned long lastFlickerUpdate = 0;
unsigned long ghostStartTime = 0; 

// ================= TRIAC =================
volatile unsigned long zeroCrossTime = 0;
esp_timer_handle_t triacTimer[3];
const int HALF_PERIOD = 10000;

// ======================================================
// HTML PAGE
// ======================================================
String htmlPage() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>SyncSpace Setup</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{ font-family:Arial; background:#f2f2f2; padding:20px; }
.box{ background:white; max-width:400px; margin:auto; padding:20px; border-radius:12px; }
select,input,button{ width:100%; padding:12px; margin-top:12px; box-sizing:border-box; }
button{ background:#007bff; color:white; border:none; cursor:pointer; }
label{ font-size:14px; color:#555; display:block; margin-top:10px; font-weight:bold; }
</style>
<script>
function checkCustom() {
  var sel = document.getElementById('ssidSelect');
  var txt = document.getElementById('customSSID');
  if(sel.value !== "") {
    txt.value = "";
    txt.style.display = "none";
  } else {
    txt.style.display = "block";
  }
}
</script>
</head>
<body onload="checkCustom()">
<div class="box">
<h2>SyncSpace WiFi Setup</h2>
<form action="/save" method="POST">
<label>Select Network:</label>
<select name="ssid" id="ssidSelect" onchange="checkCustom()">
<option value="">-- Enter Manually Below --</option>
)rawliteral";

  page += ssidList;

  page += R"rawliteral(
</select>
<input type="text" name="custom_ssid" id="customSSID" placeholder="Enter Hidden WiFi Name">
<label>WiFi Password:</label>
<input type="password" name="password" placeholder="WiFi Password">
<label>MQTT Server:</label>
<input type="text" name="mqtt" placeholder="MQTT Server IP">
<button type="submit">Connect</button>
</form>
</div>
</body>
</html>
)rawliteral";

  return page;
}

// ======================================================
// SCAN WIFI
// ======================================================
void scanWiFi() {
  ssidList = "";
  Serial.println("Scanning WiFi...");
  int n = WiFi.scanNetworks(false, true);
  if (n <= 0) {
    ssidList += "<option disabled>No Networks Found</option>";
    return;
  }
  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;
    ssidList += "<option value='" + s + "'>" + s + "</option>";
  }
  WiFi.scanDelete();
}

// ======================================================
// RESET WIFI
// ======================================================
void resetWiFiCredentials() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  Serial.println("WiFi credentials erased");
}

// ======================================================
// RESET BUTTON HANDLER
// ======================================================
void handleResetButton() {
  static unsigned long pressStart = 0;
  static bool pressed = false;

  if (digitalRead(RESET_BUTTON) == LOW) {
    if (!pressed) {
      pressed = true;
      pressStart = millis();
      Serial.println("RESET BUTTON PRESSED");
    }
    if (millis() - pressStart > 5000) {
      Serial.println("RESETTING WIFI");
      resetWiFiCredentials();
      delay(1000);
      ESP.restart();
    }
  } else {
    pressed = false;
  }
}

// ======================================================
// START AP MODE
// ======================================================
void startAPMode() {
  setupMode = true;
  Serial.println("\n===== STARTING AP MODE =====");

  WiFi.disconnect();
  delay(500);

  WiFi.mode(WIFI_AP_STA);
  bool result = WiFi.softAP(ap_ssid, ap_password, 6);

  if (!result) {
    Serial.println("AP FAILED");
    return;
  }

  Serial.println("AP STARTED");
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(apIP);

  dnsServer.start(DNS_PORT, "*", apIP);
  scanWiFi();

  server.on("/", []() {
    server.send(200, "text/html", htmlPage());
  });

  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String custom_ssid = server.arg("custom_ssid");
    String password = server.arg("password");
    String mqtt = server.arg("mqtt");

    if (custom_ssid.length() > 0) {
      ssid = custom_ssid;
    }

    ssid.trim();
    password.trim();
    mqtt.trim();

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("mqtt", mqtt);
    preferences.end();

    server.send(200, "text/html", "<h2>Saved.<br>Restarting...</h2>");
    delay(3000);
    ESP.restart();
  });

  server.on("/generate_204", []() { server.sendHeader("Location", "http://192.168.4.1", true); server.send(302, "text/plain", ""); });
  server.on("/hotspot-detect.html", []() { server.sendHeader("Location", "http://192.168.4.1", true); server.send(302, "text/plain", ""); });

  server.onNotFound([]() {
    if (!server.hostHeader().equals("192.168.4.1")) {
      server.sendHeader("Location", "http://192.168.4.1", true);
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  server.begin();
  Serial.println("WEB SERVER STARTED");
}

// ======================================================
// CONNECT WIFI
// ======================================================
bool connectWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  mqttIP = preferences.getString("mqtt", "");
  preferences.end();

  if (ssid == "") return false;

  Serial.println("\nCONNECTING WIFI");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Target MQTT Broker IP: ");
  Serial.println(mqttIP);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWIFI CONNECTED");
    Serial.print("ESP32's OWN IP Address: ");
    Serial.println(WiFi.localIP());

    if (mqtt_server_ip.fromString(mqttIP)) {
      client.setServer(mqtt_server_ip, 1883);
    } else {
      Serial.println("ERROR: Invalid MQTT Broker IP format parsed!");
    }
    setupMode = false;
    return true;
  }
  return false;
}

// ======================================================
// MQTT CALLBACK
// ======================================================
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  String t = String(topic);

  Serial.print("MQTT Message Arrived [");
  Serial.print(t);
  Serial.print("]: ");
  Serial.println(msg);

  if (t == "syncspace/relay/0/cmd") {
    if (msg == "ON") {
      relayState = true;
      digitalWrite(relayPin, HIGH);
      client.publish("syncspace/relay/0/state", "ON");
      Serial.println("-> Relay (Pin 27) turned ON");
    } else {
      relayState = false;
      digitalWrite(relayPin, LOW);
      client.publish("syncspace/relay/0/state", "OFF");
      Serial.println("-> Relay (Pin 27) turned OFF");
    }
  }

  for (int i = 0; i < 3; i++) {
    if (t == "syncspace/fan/" + String(i) + "/cmd") {
      // Allow Ghost Mode ONLY on the 3rd Triac (HALOGEN_CH = 2)
      if ((msg == "GHOST" || msg == "FLICKER") && i == HALOGEN_CH) {
        ghostMode = true;
        ghostStartTime = millis();
        lastFlickerUpdate = millis();

        // Guaranteed immediate first strike on the Halogen channel
        activeSpeed[HALOGEN_CH] = 85; 

        client.publish("syncspace/fan/2/state", "GHOST");
        Serial.println("-> Fan 2 (Halogen) set to GHOST FLICKER MODE!");
      } else {
        if (i == HALOGEN_CH) {
          ghostMode = false;
        }

        fanSpeed[i] = constrain(msg.toInt(), 0, 100);
        activeSpeed[i] = fanSpeed[i]; // Apply normal speed

        client.publish(("syncspace/fan/" + String(i) + "/state").c_str(), String(fanSpeed[i]).c_str());
        Serial.print("-> Fan ");
        Serial.print(i);
        Serial.print(" speed set to: ");
        Serial.println(fanSpeed[i]);
      }
    }
  }
}

// ======================================================
// MQTT RECONNECT
// ======================================================
void reconnect() {
  if (client.connected()) return;

  String clientID = "MainSync-";
  clientID += String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.print("Attempting MQTT connection to broker...");
  if (client.connect(clientID.c_str())) {
    Serial.println(" Connected!");
    client.subscribe("syncspace/relay/0/cmd");
    for (int i = 0; i < 3; i++) {
      client.subscribe(("syncspace/fan/" + String(i) + "/cmd").c_str());
    }
  } else {
    Serial.print(" Failed, rc=");
    Serial.print(client.state());
    Serial.println(" (Check broker IP, firewall, or port 1883)");
  }
}

// ======================================================
// TRIAC TIMER CALLBACK (NON-BLOCKING)
// ======================================================
void IRAM_ATTR triacFireCallback(void* arg) {
  int ch = (int)(intptr_t)arg;
  digitalWrite(triacPins[ch], HIGH);
  delayMicroseconds(50);
  digitalWrite(triacPins[ch], LOW);
}

// ======================================================
// ZCD ISR
// ======================================================
void IRAM_ATTR zeroCrossISR() {
  zeroCrossTime = micros();
  for (int i = 0; i < 3; i++) {
    int speed = activeSpeed[i]; 
    if (speed <= 0) {
      digitalWrite(triacPins[i], LOW);
    } else if (speed >= 99) {
      digitalWrite(triacPins[i], HIGH);
    } else {
      int delayTime = map(speed, 1, 98, HALF_PERIOD - 500, 200);
      esp_timer_start_once(triacTimer[i], delayTime);
    }
  }
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  pinMode(RESET_BUTTON, INPUT_PULLUP);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  for (int i = 0; i < 3; i++) {
    pinMode(triacPins[i], OUTPUT);
    digitalWrite(triacPins[i], LOW);

    esp_timer_create_args_t timer_args = {};
    timer_args.callback = &triacFireCallback;
    timer_args.arg = (void*)(intptr_t)i;
    timer_args.name = "triac";
    esp_timer_create(&timer_args, &triacTimer[i]);
  }

  pinMode(zcdPin, INPUT_PULLUP);

  if (!connectWiFi()) {
    startAPMode();
  }

  client.setCallback(callback);
  attachInterrupt(digitalPinToInterrupt(zcdPin), zeroCrossISR, FALLING);
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  handleResetButton();

  if (setupMode) {
    dnsServer.processNextRequest();
  }

  server.handleClient();

  if (!setupMode && WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
    if (!client.connected()) reconnect();
    client.loop();
  }

  unsigned long now = millis();

  // --- SAFETY WATCHDOG: Automatically cuts ghost mode off after 2.0s ---
  if (ghostMode && (now - ghostStartTime > 2000)) {
    ghostMode = false;
    activeSpeed[HALOGEN_CH] = 0;
    Serial.println("-> Ghost mode safety auto-off triggered.");
  }

  // --- FLICKER EFFECT GENERATOR (50ms Cadence) ---
  if (now - lastFlickerUpdate > 50) { 
    lastFlickerUpdate = now;

    if (ghostMode) {
      // 25% chance of bright flash, 75% dim/off
      if (random(100) > 75) {
        activeSpeed[HALOGEN_CH] = random(70, 100); // Bright flash
      } else {
        activeSpeed[HALOGEN_CH] = random(0, 25);   // Dim / off
      }
    }
  }

  if (!setupMode) {
    static unsigned long lastHB = 0;
    if (now - lastHB > 5000) {
      client.publish("syncspace/esp32/status", "ONLINE");
      lastHB = now;
    }
  }

  delay(2);
}
