/*

pin
A0  =   AI_SW1		PULL HIGH		Button
D0  =   None
D1  =   SCL							OLED
D2  =   SDA							OLED
D3  =   None
D4  =   None
D5  =   RX_232						RS232
D6  =   TX_232						RS232
D7  =   None
D8  =   None

Project:    esp8266-powmr
Board:      NodeMCU ESP8266

History
Version			Date			Descriptions

*/


#include "Arduino.h"
#include "Wire.h"
#include "AsyncElegantOTA.h"
#include "WiFiUdp.h"
#include "SSD1306Wire.h"
#include "ArduinoJson.h"
#include "EEPROM.h"
#include "ModbusMaster.h"
#include "SoftwareSerial.h"
#include "TaskScheduler.h"
#include "credentials.h"
#include "version.h"
#include "html.h"
#include "time.h"
#include "variable.h"

// #define __DEBUG

#define			AIN_SW1  	    			A0
#define			BAUD_RATE					2400			// 1 stop bit, no parity control
#define			SLAVE_ID					5				// PowMr device slave id is 5.
#define         RX_232                      D5
#define         TX_232                      D6
#define			SCREEN_WIDTH				128
#define			SCREEN_HEIGHT				64


uint8_t			u8_Result					= 0;
uint8_t			u8_CountRead				= 0;
uint8_t			u8_Page						= 1;
uint32_t		u32_LastActivity			= 0;
uint32_t		u32_Uptime					= 0;
uint32_t		u32_WhTime					= 0;
double			doubleOutWh					= 0.0;
double			doubleInWh					= 0.0;
bool			bitReadSuccess  			= false;
bool			bitButton       			= false;
bool			bitFlagButton   			= false;
bool			bitFlagReset				= false;

SSD1306Wire display(0x3C, D2, D1);
WiFiUDP Udp;
AsyncWebServer server(80);
ModbusMaster node;
SoftwareSerial serial232(RX_232, TX_232);
Scheduler runner;
IPAddress remoteIP(192, 168, 1, 4);		// For local server (Orange Pi with Node-RED installed)

bool getResultMsg(ModbusMaster *node, uint8_t result);
void readModBus(void);
uint16_t wordSwapBytes(uint16_t value);
bool sendUdp(String str);
void sendStatus(void);
void controlUpdate(void);
void displayUpdate(void);
void countTime(void);
void page0(void);
void page1(void);
void page2(void);
void page3(void);
void calculateWh(void);
String numToTimeStr(uint32_t n);
void clearData(void);

Task tk_readModBus(1000, TASK_FOREVER, &readModBus);
Task tk_sendStatus(10000, TASK_FOREVER, &sendStatus);
Task tk_controlUpdate(50, TASK_FOREVER, &controlUpdate);
Task tk_displayUpdate(50, TASK_FOREVER, &displayUpdate);
Task tk_countTime(1000, TASK_FOREVER, &countTime);
Task tk_calculateWh(1000, TASK_FOREVER, &calculateWh);

void setup() {
	#ifdef __DEBUG
    Serial.begin(115200);
    Serial.println("Booting");
	#endif

	EEPROM.begin(EEPROM_SIZE);
	EEPROM.get(ADDR_InWh, doubleInWh);
	EEPROM.get(ADDR_OutWh, doubleOutWh);

	serial232.begin(BAUD_RATE);
	node.begin(SLAVE_ID, serial232);
    display.init();
    display.flipScreenVertically();
    display.clear();
    display.setBrightness(64);
    display.drawString(0, 0, "Inverter");
    display.drawString(0, 12, version);
    display.display();
	delay(1000);

	connectToWifi();
	while (WiFi.status() != WL_CONNECTED) {
		delay(100);
	}

    /*		--- ElegantOTA ---		*/
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/html", html);
	});
    AsyncElegantOTA.begin(&server);
    server.begin();

	runner.addTask(tk_readModBus);
	runner.addTask(tk_sendStatus);
	runner.addTask(tk_countTime);
	runner.addTask(tk_controlUpdate);
	runner.addTask(tk_displayUpdate);
	runner.addTask(tk_calculateWh);
	tk_readModBus.enable();
	tk_sendStatus.enable();
	tk_countTime.enable();
	tk_controlUpdate.enable();
	tk_displayUpdate.enable();
	tk_calculateWh.enable();
	u32_LastActivity = millis();
	u32_Uptime = millis()/1000;
	u32_WhTime = millis();
}

void loop() {
	runner.execute();
}

void sendStatus(void) {
	EEPROM.put(ADDR_InWh, doubleInWh);
	EEPROM.put(ADDR_OutWh, doubleOutWh);
	EEPROM.commit();
	if (WiFi.status() != WL_CONNECTED) return;
	else {
		DynamicJsonDocument docInfo(1024);
		String tData = "";
		docInfo["rssi"]				= String(WiFi.RSSI());
		docInfo["uptime"]			= String(u32_Uptime);
		for (uint8_t i = 0; i < 38; i++) {
			docInfo["data"][i]		= String(modbus_read.u16[i]);
		}
		docInfo["data"][38]			= String(doubleInWh, 3);
		docInfo["data"][39]			= String(doubleOutWh, 3);
		serializeJson(docInfo, tData);
		sendUdp(tData);
		docInfo.clear();
	}
}

bool getResultMsg(ModbusMaster *node, uint8_t result) {
	String tmpstr2 = "\r\n";
	switch (result) {
		case node->ku8MBSuccess:
			#ifdef __DEBUG
			Serial.println("***ku8MBSuccess***");
			#endif
			return true;
		break;
		case node->ku8MBIllegalFunction:		tmpstr2 += "Illegal Function";
												break;
		case node->ku8MBIllegalDataAddress:		tmpstr2 += "Illegal Data Address";
												break;
		case node->ku8MBIllegalDataValue:		tmpstr2 += "Illegal Data Value";
												break;
		case node->ku8MBSlaveDeviceFailure:		tmpstr2 += "Slave Device Failure";
												break;
		case node->ku8MBInvalidSlaveID:			tmpstr2 += "Invalid Slave ID";
												break;
		case node->ku8MBInvalidFunction:		tmpstr2 += "Invalid Function";
												break;
		case node->ku8MBResponseTimedOut:		tmpstr2 += "Response Timed Out";
												break;
		case node->ku8MBInvalidCRC:				tmpstr2 += "Invalid CRC";
												break;
		default:								tmpstr2 += "Unknown error: " + String(result);
												break;
	}
	#ifdef __DEBUG
	DynamicJsonDocument docInfo(256);
	String tData = "";
	docInfo["debug"][0] = tmpstr2;
	serializeJson(docInfo, tData);
	sendUdp(tData);
	docInfo.clear();
	Serial.println(tmpstr2);
	#endif
	return false;
}

void readModBus(void) {
	switch (u8_CountRead) {
		case 0: {
			uint16_t startAddr = 4501;
			u8_Result = node.readHoldingRegisters(startAddr, 8);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 0;
				for (uint8_t j = 0; j<8; j++) {
					uint16_t res = node.getResponseBuffer(j);
					modbus_read.u16[startIndex+j] = wordSwapBytes(res);
				}
				u8_CountRead += 1;
			}
			break;
		}
		case 1: {
			uint16_t startAddr = 4509;
			u8_Result = node.readHoldingRegisters(startAddr, 8);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 8;
				for (uint8_t j = 0; j<8; j++) {
					uint16_t res = node.getResponseBuffer(j);
					modbus_read.u16[startIndex+j] = wordSwapBytes(res);
				}
				u8_CountRead += 1;
			}
			break;
		}
		case 2: {
			uint16_t startAddr = 4530;
			u8_Result = node.readHoldingRegisters(startAddr, 1);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 16;
				uint16_t res = node.getResponseBuffer(0);
				modbus_read.u16[startIndex] = wordSwapBytes(res);
				u8_CountRead += 1;
			}
			break;
		}
		case 3: {
			uint16_t startAddr = 4535;
			u8_Result = node.readHoldingRegisters(startAddr, 4);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 17;
				for (uint8_t j = 0; j<4; j++) {
					uint16_t res = node.getResponseBuffer(j);
					uint16_t nowAddr = startAddr + j;
					if (nowAddr == 4535) {
						modbus_read.u16[startIndex+j] = res;
					} else {
						modbus_read.u16[startIndex+j] = wordSwapBytes(res);
					}
				}
				u8_CountRead += 1;
			}
			break;
		}
		case 4: {
			uint16_t startAddr = 4540;
			u8_Result = node.readHoldingRegisters(startAddr, 8);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 21;
				for (uint8_t j = 0; j<8; j++) {
					uint16_t res = node.getResponseBuffer(j);
					modbus_read.u16[startIndex+j] = wordSwapBytes(res);
				}
				u8_CountRead += 1;
			}
			break;
		}
		case 5: {
			uint16_t startAddr = 4548;
			u8_Result = node.readHoldingRegisters(startAddr, 8);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 29;
				for (uint8_t j = 0; j<8; j++) {
					uint16_t res = node.getResponseBuffer(j);
					uint16_t nowAddr = startAddr + j;
					if (nowAddr == 4553 || nowAddr == 4554) {
						modbus_read.u16[startIndex+j] = res;
					} else {
						modbus_read.u16[startIndex+j] = wordSwapBytes(res);
					}
				}
				u8_CountRead += 1;
			}
			break;
		}
		case 6: {
			uint16_t startAddr = 4556;
			u8_Result = node.readHoldingRegisters(startAddr, 2);
			yield();
			bitReadSuccess = getResultMsg(&node, u8_Result);
			if (bitReadSuccess) {
				uint8_t startIndex = 37;
				for (uint8_t j = 0; j<2; j++) {
					uint16_t res = node.getResponseBuffer(j);
					modbus_read.u16[startIndex+j] = wordSwapBytes(res);
				}
				u8_CountRead += 1;
			}
			break;
		}
		default:
			u8_CountRead = 0;
			break;
	}
}

uint16_t wordSwapBytes(uint16_t value) {
	return (value << 8) | (value >> 8);
}

bool sendUdp(String str) {
	Udp.beginPacket(remoteIP, REMOTE_UDP_PORT);
	Udp.write(str.c_str());
	return Udp.endPacket();
}

void controlUpdate(void) {
	bitButton = (analogRead(AIN_SW1) < 8);

	if (bitButton && !bitFlagButton) {
		bitFlagButton = true;
		u32_LastActivity = millis();
		u8_Page += 1;
		if (u8_Page > 3) u8_Page = 1;
	}

	/* Prevent reset if cannot read ModBus in first time */
	if (bitButton && millis() - u32_LastActivity > 15000 && !bitFlagReset && bitReadSuccess) {
		bitFlagReset = true;
		clearData();
	}

	if (!bitButton) {
		bitFlagButton = false;
		bitFlagReset = false;
	}

	if (millis() - u32_LastActivity > 120000 && u8_Page != 0) {
		u8_Page = 0;
	}
}

void displayUpdate(void) {
	switch (u8_Page) {
		case 1:
			page1();
			break;
		case 2:
			page2();
			break;
		case 3:
			page3();
			break;
		default:
			page0();
			break;
	}
}

void page0(void) {
	display.clear();
	display.display();
}

void page1(void) {
	display.clear();
	display.drawString(2, 0, "Battery");
	display.drawString(2, 10, "SoC: " + String(u16_Batt_SoC) + " %  /  Volt: " + String(u16_Batt_Volt/10.0, 1) + " V");
	display.drawString(2, 20, "Out: " + String(u16_Load_Volt/10.0, 1) + " V  /  " + String(u16_Discharge_Cur) + " A");
	display.drawString(2, 30, "In: " + String(u16_Charge_Cur) + " A");
	display.drawString(2, 53, numToTimeStr(u32_Uptime));
	display.display();
}

void page2(void) {
	String t = "";
	switch (u16_Charge_Status) {
		case 0:
			t = "INACTIVE";
			break;
		case 1:
			t = "IDLE";
			break;
		case 2:
			t = "ACTIVE";
			break;
		default:
			t = "";
			break;
	}
	display.clear();
	display.drawString(2, 0, "PV");
	display.drawString(2, 10, t);
	display.drawString(2, 20, "Volt: " + String(u16_PV_Volt/10.0,1) + " V");
	display.drawString(2, 30, "In: " + String(u16_Charge_Cur) + " A");
	display.drawString(2, 40, "In: " + String(doubleInWh, 3) + " kWh");
	display.drawString(2, 53, numToTimeStr(u32_Uptime));
	display.display();
}

void page3(void) {
	display.clear();
	display.drawString(2, 0, "Load");
	display.drawString(2, 10, "Volt: " + String(u16_Load_Volt/10.0,1) + " V  /  " + String(u16_Load_Freq/10.0,1) + " Hz");
	display.drawString(2, 20, "Out: " + String(u16_Discharge_Cur) + " A  /  " + String(u16_Load_Power) + " W");
	display.drawString(2, 30, "Out: " + String(u16_Load_Per1) + " %");
	display.drawString(2, 40, "Out: " + String(doubleOutWh, 3) + " kWh");
	display.drawString(2, 53, numToTimeStr(u32_Uptime));
	display.display();
}

void countTime(void) {
	u32_Uptime += 1;
}

void calculateWh(void) {
	doubleInWh	+= ((u16_Batt_Volt / 10.0) * u16_Charge_Cur) * (millis() - u32_WhTime) / 3600000000.0;	// From battery power
	// doubleOutWh	+= ((u16_Batt_Volt / 10.0) * u16_Discharge_Cur) * (millis() - u32_WhTime) / 3600000000.0;	// From battery power
	doubleOutWh	+= u16_Load_Power * (millis() - u32_WhTime) / 3600000000.0;		// From load power
	u32_WhTime = millis();
}

String numToTimeStr(uint32_t n) {
	char t[32];
	sprintf(t, "%d Days, %d%d:%d%d:%d%d", n/86400, ((n/3600)%24)/10, ((n/3600)%24)%10, ((n/60)%60)/10, ((n/60)%60)%10, (n%60)/10, (n%60)%10);
	return t;
}

void clearData(void) {
	doubleInWh	= 0.0;
	doubleOutWh	= 0.0;
	EEPROM.put(ADDR_InWh, doubleInWh);
	EEPROM.put(ADDR_OutWh, doubleOutWh);
	EEPROM.commit();
}