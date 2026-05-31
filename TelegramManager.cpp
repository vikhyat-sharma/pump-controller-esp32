#include <Arduino.h>
#include "TelegramManager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

TelegramManager telegramManager;

unsigned long telegramLastUpdateId = 0;

void TelegramManager::sendMessage(String message, String chatId) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    message.replace("\n", "%0A");
    Serial.print("Sending Telegram message to chat id: ");
    Serial.println(chatId);
    String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/sendMessage?chat_id=" + chatId + "&text=" + message;

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void TelegramManager::pollUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/getUpdates?offset=" + String(telegramLastUpdateId + 1);

  if (https.begin(client, url)) {
    int httpCode = https.GET();
    if (httpCode == 200) {
      String payload = https.getString();

      const size_t capacity = 30 * JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3) + 2048;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc["ok"]) {
        JsonArray result = doc["result"].as<JsonArray>();

        for (JsonVariant update : result) {
          JsonObject obj = update.as<JsonObject>();
          telegramLastUpdateId = obj["update_id"].as<unsigned long>();

          if (obj.containsKey("message")) {
            String chatId = obj["message"]["chat"]["id"].as<String>();
            String text = obj["message"]["text"].as<String>();
            Serial.println("Received: " + text + " from " + chatId);
            handleCommand(chatId, text);
          }
        }
      }
    }
    https.end();
  }
}

void TelegramManager::saveApprovedChatID(const String& chatID) {
  preferences.begin("telegram", false);
  String existing = preferences.getString("approved_ids", "");
  if (existing.indexOf(chatID) == -1) {
    if (existing.length() > 0) existing += ",";
    existing += chatID;
    preferences.putString("approved_ids", existing);
  }
  preferences.end();
}

String TelegramManager::getApprovedChatIDs() {
  preferences.begin("telegram", true);
  String ids = preferences.getString("approved_ids", "");
  preferences.end();
  return ids;
}

bool TelegramManager::isChatIDApproved(const String& chatID) {
  String approved = getApprovedChatIDs();
  return approved.indexOf(chatID) != -1;
}

void TelegramManager::sendToApproved(String message) {
  String approved = getApprovedChatIDs();
  int start = 0;
  int end = approved.indexOf(',');
  while (end != -1) {
    String chatID = approved.substring(start, end);
    sendMessage(message, chatID);
    start = end + 1;
    end = approved.indexOf(',', start);
  }
  // Send to last chat ID
  if (start < approved.length()) {
    String chatID = approved.substring(start);
    sendMessage(message, chatID);
  }
}

void TelegramManager::handleCommand(const String& fromChatID, const String& messageText) {
  if (messageText == "/start") {
    if (isChatIDApproved(fromChatID)) {
      sendMessage("You're already approved to receive notifications.", fromChatID);
    } else {
      sendMessage("Thanks for requesting notifications. Awaiting admin approval.", fromChatID);
      sendMessage("New request from chat ID: " + fromChatID + "\nSend /approve " + fromChatID + " to approve.", TELEGRAM_CHAT_ID);
    }
  } 
  else if (messageText.startsWith("/approve ") && fromChatID == TELEGRAM_CHAT_ID) {
    String newChatID = messageText.substring(9);
    if (isChatIDApproved(newChatID)) {
      sendMessage("You've already approved to this chat ID.", TELEGRAM_CHAT_ID);
    } else {
      saveApprovedChatID(newChatID);
      sendMessage("Chat ID " + newChatID + " approved.", TELEGRAM_CHAT_ID);
      sendMessage("Your request has been approved. You'll now receive notifications.", newChatID);
    }
  }
  else if (messageText.startsWith("/unapprove ") && fromChatID == TELEGRAM_CHAT_ID) {
    String removeChatID = messageText.substring(11);
    removeApprovedChatID(removeChatID);
    sendMessage("Unapproved ❌", removeChatID);
  }
  else if (fromChatID == TELEGRAM_CHAT_ID && messageText == "/listapproved") {
    preferences.begin("telegram", false);
    String approved = preferences.getString("approved_ids", "");
    preferences.end();
    
    if (approved.length() == 0) {
      sendMessage("No approved chat IDs.", TELEGRAM_CHAT_ID);
    } else {
      sendMessage("Approved Chat IDs:\n" + approved, TELEGRAM_CHAT_ID);
    }
  }
}

bool TelegramManager::removeApprovedChatID(const String& chatID) {
  preferences.begin("telegram", false);
  String existing = preferences.getString("approved_ids", "");

  int start = 0;
  int end = existing.indexOf(',');
  String result = "";
  bool removed = false;

  while (end != -1) {
    String id = existing.substring(start, end);
    if (id != chatID) {
      if (result.length() > 0) result += ",";
      result += id;
    } else {
      removed = true;
    }
    start = end + 1;
    end = existing.indexOf(',', start);
  }

  // Handle last or single element
  String lastID = existing.substring(start);
  if (lastID.length() > 0 && lastID != chatID) {
    if (result.length() > 0) result += ",";
    result += lastID;
  } else if (lastID == chatID) {
    removed = true;
  }

  if (removed) {
    preferences.putString("approved_ids", result);
  }

  preferences.end();
  return removed;
}
