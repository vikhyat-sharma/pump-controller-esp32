#ifndef TELEGRAMMANAGER_H
#define TELEGRAMMANAGER_H

#include <HTTPClient.h>
#include <Preferences.h>
#include "Config.h"

class TelegramManager {
public:
  void sendMessage(String message, String chatId = TELEGRAM_CHAT_ID);
  void handleCommand(const String& fromChatID, const String& messageText);
  bool removeApprovedChatID(const String& chatID);
  void sendToApproved(String message);
  void saveApprovedChatID(const String& chatID);
  bool isChatIDApproved(const String& chatID);
  String getApprovedChatIDs();

private:
  Preferences preferences;
};

extern TelegramManager telegramManager;

#endif
