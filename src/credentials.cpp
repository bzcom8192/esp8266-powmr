#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "credentials.h"

// #define __DEBUG_WIFI

IPAddress local_IP(192, 168, 1, 209);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);

void connectToWifi(void) {
	if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
		#ifdef __DEBUG_WIFI
		Serial.println("STA Failed to configure");
		#endif
	}
	WiFi.begin(ssid, password);
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);
	#ifdef __DEBUG_WIFI
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	#endif
}