// ============================================================================
// PUMP CONTROLLER - Simplified Version
// ============================================================================
// A beginner-friendly ESP32 pump controller with local button, Blynk,
// Telegram, and ESP-NOW support.
//
// QUICK SETUP:
// 1. Copy Config.example.h to Config.h and fill in your credentials
// 2. Upload to ESP32
// 3. Use button, Blynk app, or Telegram to control the pump
// ============================================================================

// ============================================================================
// MAIN INCLUDE - All necessary libraries
// ============================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <esp_now.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <BlynkSimpleEsp32.h>
#include "Config.h"

// -------------------------------------------------------------------------
// SECTION 1: GLOBAL VARIABLES - All state in one place
// -------------------------------------------------------------------------

// Pump state
bool pumpRunning = false;
bool manualOverride = false;
unsigned long pumpStartTime = 0;
unsigned long manualOverrideStart = 0;

// Tank levels (from ESP-NOW sensor devices)
int tank1Level = 0;  // Source tank (0-100%)
int tank2Level = 0;  // Destination tank (0-100%)

// Flow sensor
volatile int flowPulseCount = 0;
int lastFlowPulses = 0;

// Timing
unsigned long lastStatusUpdate = 0;
unsigned long lastTelegramPoll = 0;
unsigned long lastFlowCheck = 0;
unsigned long lastAP_check = 0;

// Telegram tracking
unsigned long lastUpdateId = 0;

// AP / DNS servers
WebServer webServer(80);
DNSServer dnsServer;

// -------------------------------------------------------------------------
// SECTION 2: PIN CONFIGURATION & BASIC SETUP
// -------------------------------------------------------------------------

void setupPins() {
  // Relay pin - LOW turns pump ON (most relay modules are active-low)
  pinMode(PUMP_PIN, OUTPUT);
  turnPumpOff();  // Start with pump OFF

  // Manual button (pulled HIGH, pressed LOW)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Flow sensor (counts pulses from flow meter)
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulseISR, FALLING);

  // Status LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

IRAM_ATTR void flowPulseISR() {
  flowPulseCount++;
}

// -------------------------------------------------------------------------
// SECTION 3: WIFI CONNECTION
// -------------------------------------------------------------------------

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    blinkLED(3, 200);  // 3 blinks = success
  } else {
    Serial.println("\nWiFi failed. Starting hotspot mode...");
    startCaptivePortal();
  }
}

// Start a local hotspot when WiFi fails - lets users configure credentials
void startCaptivePortal() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PumpController-Setup", "12345678");

  Serial.print("Hotspot IP: ");
  Serial.println(WiFi.softAPIP());

  // Route all domains to our simple page
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Simple web pages
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html",
      "<h1>Pump Controller</h1>"
      "<p>WiFi not configured. Edit Config.h with your credentials.</p>");
  });

  webServer.on("/generate_204", HTTP_GET, []() {
    webServer.send(200, "text/html", "<h1>Please configure WiFi</h1>");
  });

  webServer.onNotFound([]() {
    webServer.send(200, "text/html", "<h1>Pump Controller Setup</h1>");
  });

  webServer.begin();
  Serial.println("Captive portal running");
}

// -------------------------------------------------------------------------
// SECTION 4: PUMP CONTROL FUNCTIONS
// -------------------------------------------------------------------------

void startPump() {
  // Cooldown check - prevent rapid on/off
  unsigned long now = millis();
  if (now - pumpStartTime < 30000 && pumpStartTime != 0) {
    Serial.println("Cooldown active - wait 30s before restart");
    return;
  }

  turnPumpOn();
  pumpRunning = true;
  pumpStartTime = now;

  Blynk.virtualWrite(V3, 1);  // Update Blynk status
  Serial.println(">>> PUMP ON <<<");
}

void stopPump() {
  turnPumpOff();
  pumpRunning = false;

  Blynk.virtualWrite(V3, 0);  // Update Blynk status
  Serial.println(">>> PUMP OFF <<<");
}

void togglePump() {
  if (pumpRunning) stopPump();
  else startPump();
}

// Safety timer - stop pump if running too long
void checkPumpSafety() {
  if (!pumpRunning) return;

  unsigned long runtime = millis() - pumpStartTime;

  // Configurable max runtime (default: 30 minutes)
  #ifdef MAX_PUMP_ON_DURATION_MS
  if (runtime >= MAX_PUMP_ON_DURATION_MS) {
    stopPump();
    Serial.println("SAFETY: Pump stopped - max runtime reached");

    // Send notification if Blynk event is configured
    #ifdef BLYNK_TEMPLATE_ID
    Blynk.logEvent("safety", "Pump auto-stopped: safety timeout");
    #endif
  }
  #endif
}

// Check button press (with simple debounce)
void checkButton() {
  static unsigned long lastPress = 0;
  static int lastState = HIGH;

  int current = digitalRead(BUTTON_PIN);

  // Button pressed (LOW)
  if (current == LOW && lastState == HIGH) {
    if (millis() - lastPress > 50) {  // Simple 50ms debounce
      togglePump();
      manualOverride = true;
      manualOverrideStart = millis();
      Serial.println("Manual button pressed");
    }
    lastPress = millis();
  }

  lastState = current;

  // Auto disable manual override after 10 minutes
  if (manualOverride && millis() - manualOverrideStart > 600000) {
    manualOverride = false;
    Serial.println("Manual override expired");
  }
}

// -------------------------------------------------------------------------
// SECTION 5: AUTO-PUMP BASED ON TANK LEVELS
// -------------------------------------------------------------------------

void checkAutoPump() {
  #ifdef AUTO_PUMP_ENABLED
  if (manualOverride) return;

  if (!pumpRunning &&
      tank1Level >= TANK1_LOW_THRESHOLD &&
      tank2Level < TANK2_HIGH_THRESHOLD) {
    Serial.println("Auto: Starting pump (tank1 OK, tank2 needs fill)");
    startPump();
  }
  else if (pumpRunning &&
           (tank2Level >= TANK2_HIGH_THRESHOLD ||
            tank1Level < TANK1_LOW_THRESHOLD)) {
    Serial.println("Auto: Stopping pump (tank2 full or tank1 low)");
    stopPump();
  }
  #endif
}

// -------------------------------------------------------------------------
// SECTION 6: FLOW SENSOR
// -------------------------------------------------------------------------

void updateFlowSensor() {
  if (millis() - lastFlowCheck >= 1000) {
    lastFlowCheck = millis();

    if (lastFlowPulses > 0) {
      Blynk.virtualWrite(V4, lastFlowPulses);  // Send to Blynk
      Serial.print("Flow: ");
      Serial.println(lastFlowPulses);
    }

    lastFlowPulses = flowPulseCount;
    flowPulseCount = 0;
  }
}

// -------------------------------------------------------------------------
// SECTION 7: ESP-NOW COMMUNICATION (Sensor data from other ESP32)
// -------------------------------------------------------------------------

// Special packet structure for ESP-NOW messages
struct MessagePacket {
  char type[16];     // "LEVEL", "CMD", etc.
  char payload[128]; // Data
};

void onEspNowReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != sizeof(MessagePacket)) return;

  MessagePacket msg;
  memcpy(&msg, data, sizeof(msg));
  msg.type[15] = '\0';
  msg.payload[127] = '\0';

  Serial.print("ESP-NOW: ");
  Serial.print(msg.type);
  Serial.print(" -> ");
  Serial.println(msg.payload);

  // Parse tank levels: "30,80" means tank1=30%, tank2=80%
  if (strcmp(msg.type, "LEVEL") == 0) {
    sscanf(msg.payload, "%d,%d", &tank1Level, &tank2Level);
    tank1Level = constrain(tank1Level, 0, 100);
    tank2Level = constrain(tank2Level, 0, 100);

    Blynk.virtualWrite(V0, tank1Level);
    Blynk.virtualWrite(V1, tank2Level);
  }
  // Remote pump commands (requires 3 commands within 30 seconds)
  else if (strcmp(msg.type, "CMD") == 0) {
    if (manualOverride) {
      Serial.println("Ignored: manual override active");
      return;
    }

    static int onCount = 0, offCount = 0;
    static unsigned long firstOnTime = 0, firstOffTime = 0;

    if (strcmp(msg.payload, "ON") == 0) {
      if (millis() - firstOnTime > 30000) { onCount = 0; firstOnTime = millis(); }
      onCount++;
      if (onCount >= 3) { startPump(); onCount = 0; }
    }
    else if (strcmp(msg.payload, "OFF") == 0) {
      if (millis() - firstOffTime > 30000) { offCount = 0; firstOffTime = millis(); }
      offCount++;
      if (offCount >= 3) { stopPump(); offCount = 0; }
    }
  }
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onEspNowReceive);
  Serial.println("ESP-NOW ready");
}

// -------------------------------------------------------------------------
// SECTION 8: TELEGRAM BOT
// -------------------------------------------------------------------------

void sendTelegram(String msg) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  msg.replace("\n", "%0A");
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) +
               "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + msg;

  http.begin(url);
  int code = http.GET();
  if (code > 0) Serial.println("Telegram: " + String(code));
  http.end();
}

void pollTelegram() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastTelegramPoll < 5000) return;  // Poll every 5 seconds
  lastTelegramPoll = millis();

  HTTPClient https;
  telegramClient.setInsecure();
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) +
               "/getUpdates?offset=" + String(lastUpdateId + 1);

  if (!https.begin(telegramClient, url)) return;

  int code = https.GET();
  if (code != 200) { https.end(); return; }

  String payload = https.getString();
  https.end();

  // Parse JSON response
  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, payload);

  if (err || !doc["ok"]) return;

  for (JsonObject update : doc["result"].as<JsonArray>()) {
    lastUpdateId = update["update_id"].as<unsigned long>();

    if (update.containsKey("message")) {
      String chatId = update["message"]["chat"]["id"].as<String>();
      String text = update["message"]["text"].as<String>();

      Serial.print("Telegram from ");
      Serial.print(chatId);
      Serial.print(": ");
      Serial.println(text);

      // Process commands
      if (text == "/start") {
        sendTelegram("Pump Controller ready! Commands: /on /off /status /help");
      }
      else if (text == "/on" && chatId == TELEGRAM_CHAT_ID) {
        startPump();
        sendTelegram("Pump started via Telegram");
      }
      else if (text == "/off" && chatId == TELEGRAM_CHAT_ID) {
        stopPump();
        sendTelegram("Pump stopped via Telegram");
      }
      else if (text == "/status") {
        String status = "Pump: " + String(pumpRunning ? "ON" : "OFF") +
                       "\nTank 1: " + String(tank1Level) + "%" +
                       "\nTank 2: " + String(tank2Level) + "%" +
                       "\nManual: " + String(manualOverride ? "YES" : "NO");
        sendTelegram(status);
      }
      else if (text == "/help") {
        sendTelegram("Commands:\n/on - Start pump\n/off - Stop pump\n/status - View status");
      }
    }
  }
}

// -------------------------------------------------------------------------
// SECTION 9: BLYNK APP SUPPORT
// -------------------------------------------------------------------------

// This runs when Blynk app sends commands
BLYNK_WRITE(V2) {
  // V2 is the pump ON/OFF switch in Blynk
  int value = param.asInt();
  if (value == 1) startPump();
  else stopPump();
}

// -------------------------------------------------------------------------
// SECTION 10: MAIN SETUP & LOOP
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PUMP CONTROLLER STARTING ===");

  // 1. Setup all pins
  setupPins();

  // 2. Connect to WiFi (or start AP)
  connectWiFi();

  // 3. Setup Blynk (cloud control)
  #ifdef BLYNK_AUTH_TOKEN
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  #endif

  // 4. Setup OTA updates (wireless firmware upload)
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  // 5. Setup ESP-NOW (receive tank levels)
  setupEspNow();

  // 6. Telegram startup message
  sendTelegram("Pump Controller online! IP: " + WiFi.localIP().toString());

  Serial.println("=== READY ===");
}

void loop() {
  // Run everything repeatedly

  Blynk.run();           // Blynk cloud connection
  ArduinoOTA.handle();   // Check for OTA updates

  // If WiFi disconnected, run captive portal
  if (WiFi.status() != WL_CONNECTED) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }

  // Core pump logic
  checkButton();        // Manual button
  checkPumpSafety();    // Safety timer
  checkAutoPump();      // Auto based on tank levels
  updateFlowSensor();   // Flow meter readings

  // External connections
  pollTelegram();       // Check Telegram commands

  // Blink LED when pump is running
  if (pumpRunning) {
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Reconnect WiFi if dropped
  if (WiFi.status() != WL_CONNECTED && millis() - lastAP_check > 60000) {
    lastAP_check = millis();
    Serial.println("WiFi lost - reconnecting...");
    connectWiFi();
  }
}