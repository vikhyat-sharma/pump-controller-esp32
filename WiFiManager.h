#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include "Config.h"
#include <DNSServer.h>

class WiFiManager {
public:
  void connectWiFi();
  String getIPAddress();
  bool isConnected();
  void startAccessPoint();
  void handleDNS();
};

extern WiFiManager wifiManager;
extern DNSServer dnsServer;

#endif
