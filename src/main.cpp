#include <Arduino.h>
#include <M5Stack.h>

#include <Wifi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <DNSServer.h>

// for websocket
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#ifndef ENABLE_WIFI_AP
// #include "WiFiManager.h" //https://github.com/tzapu/WiFiManager
#include "ESPAsyncWiFiManager.h" //https://github.com/alanswx/ESPAsyncWiFiManager
#endif

#include "SPIFFS.h"

#include "utils/Debug.h"
#include "Roomba.h"
#include "Status.h"
#include "Action.h"


// Project config
#include "def.h"

uint32_t updateTimer;

Debug debug(&Serial, DEBUG_LEVEL);

Roomba roomba(&Serial2, ROOMBA_BRC);
Status status;
Action action;

const char *SSID = "rb";
IPAddress apIP(ACCESSPOINT_IP1, ACCESSPOINT_IP2, ACCESSPOINT_IP3, ACCESSPOINT_IP4);

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool ledState = false;


void notifyClients()
{
    ws.textAll(String(ledState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        for(int i = 0; i < len; i++)
        {
            debug.print((char)data[i], DEBUG_CONTROL);
        }
        debug.println("", DEBUG_CONTROL);


        if (strcmp((char *)data, "toggle") == 0)
        { 
            ledState = !ledState;
            notifyClients();
            if(ledState)
                debug.println("on", DEBUG_CONTROL);
            else
                debug.println("off", DEBUG_CONTROL);
        }

        else if (strcmp((char *)data, "f") == 0)
        {
            debug.println("forward", DEBUG_CONTROL);
            action.forward();
        }
        else if (strcmp((char *)data, "b") == 0)
        {
            debug.println("back", DEBUG_CONTROL);
            action.back();
        }
        else if (strcmp((char *)data, "r") == 0)
        {
            debug.println("right", DEBUG_CONTROL);
            action.turnRight();
        }
        else if (strcmp((char *)data, "l") == 0)
        {
            debug.println("left", DEBUG_CONTROL);
            action.turnLeft();
        }
        else if (strcmp((char *)data, "c") == 0)
        {
            debug.println("toggle motor", DEBUG_CONTROL);
            action.toggleCleaningMotors();
        }
        else if (strcmp((char *)data, "s") == 0)
        {
            debug.println("stop", DEBUG_CONTROL);
            action.stop();
        }
        else
        {
            char cstrX[len], cstrY[len];
            bool foundSeparator = false;
            uint8_t cstrYindex = 0;
            for (int i = 0; i < len; i++)
            {
                char currentChar = (char)data[i];
                if (!foundSeparator)
                {
                    //if separator was found, mark the location
                    if (currentChar == ',')
                    {
                        foundSeparator = true;
                    }
                    // loop for loading first, X value
                    else
                    {
                        cstrX[i] = currentChar;
                    }
                }
                // reading loop for second, Y value
                else
                {
                    cstrY[cstrYindex++] = currentChar;
                }
            }
            int16_t coordinateX = atoi(cstrX);
            int16_t coordinateY = atoi(cstrY);
            debug.print(coordinateX, DEBUG_CONTROL);
            debug.print(",", DEBUG_CONTROL);
            debug.println(coordinateY, DEBUG_CONTROL);
            if (coordinateX < SPEED_LIMIT && coordinateX > -SPEED_LIMIT &&
                coordinateY < SPEED_LIMIT && coordinateY > -SPEED_LIMIT)
                action.setXY2Speed(coordinateX, coordinateY);
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

String processor(const String &var)
{
    debug.println(var, DEBUG_CONTROL);
    if (var == "STATE")
    {
        if (ledState)
        {
            return "ON";
        }
        else
        {
            return "OFF";
        }
    }
    return String();
}

/////////////////////// Coding Part /////////////////////////////////////////
char *strToChar(String str)
{
	int len = str.length() + 1;
	char *buf = new char[len];
	strcpy(buf, str.c_str());
	return buf;
}

String TFReadFile(String path)
{
	String buf = "";
	File file = SD.open(strToChar(path));
	if (file)
	{
		while (file.available())
		{
			buf += (char)file.read();
		}
		file.close();
	}
	return buf;
}

void showQR(){
    M5.Lcd.clear();
//   M5.Lcd.qrcode(const char *string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);
    // M5.Lcd.qrcode("http://192.168.4.1");
    String ipAddress = "http://";
    #ifdef ENABLE_WIFI_AP
    ipAddress += WiFi.softAPIP().toString();
    #else
    ipAddress += WiFi.localIP().toString();
#endif
    M5.Lcd.qrcode(ipAddress);
    debug.println(ipAddress, DEBUG_WIFI);
}

String openPage(String page)
{
	debug.print(F("Opening Page: "), DEBUG_CONTROL);
	debug.println(page, DEBUG_CONTROL);
	String content = "";

	// load page event
	if (page == "/")
	{
		content = TFReadFile("/index.html");
	}
	
	if (content != "")  return content;
	else return "# 404 NOT FOUND #\n"; // if not found
}

int cntChrs(String str, char chr)
{
	int cnt = 0;
	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == chr)
			cnt++;
	}
	return cnt;
}

// void configModeCallback(WiFiManager *myWiFiManager)
// {
//     debug.println("Entered config mode", DEBUG_WIFI);
//     debug.printf(DEBUG_WIFI, "%s\n", WiFi.softAPIP());
// }

void setupWiFi(String myName)
{
    // on wifi ap mode
#ifdef ENABLE_WIFI_AP
    WiFi.disconnect();   //added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_OFF); //added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(myName.c_str());
#else
    // WiFiManager wifiManager;
    AsyncWiFiManager wifiManager(&server, &dnsServer);
    // //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    // wifiManager.setAPCallback(configModeCallback);
    // wifiManager.setBreakAfterConfig(true); // Without this saveConfigCallback does not get fired
    // wifiManager.setEnableConfigPortal(true);
    wifiManager.autoConnect("WIFISETUP");

    debug.println("wifi setup", DEBUG_WIFI);

    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

#endif
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
#ifdef ENABLE_DNS
    dnsServer.start(DNS_PORT, "*", apIP);
#endif
    // server.begin();

}

void setup() {
    updateTimer = millis();

    Serial.begin(MONITOR_BAUD);
    // Serial2.begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert)
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    debug.println(F("M5-Roomba"), DEBUG_GENERAL);

    M5.begin();
    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.setBrightness(0x80);

    if(!SPIFFS.begin(true))
    {
        debug.println("error on mounting SPIFFS", DEBUG_SPIFFS);
        return;
    }

#ifndef DEBUG_ONLY_WEB
    roomba.roboInitSequence();
#endif

    String deviceName = SSID;
    setupWiFi(deviceName);

    showQR();

    initWebSocket();

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", String(), false, processor); });

    // Start server
    server.begin();

    debug.println(F("Init done"), DEBUG_GENERAL);
    M5.Lcd.setRotation(0);
    M5.Lcd.println("Init done");

    M5.Lcd.println(WiFi.softAPIP().toString());
    M5.Lcd.setRotation(1);
}

void loop()
{
#ifdef ENABLE_DNS
    dnsServer.processNextRequest();
#endif
    ws.cleanupClients();
    // if(millis() - updateTimer > UPDATE_RATE_ms){
    //     dnsServer.processNextRequest();
    //     WiFiClient client;
        SensorData data;

    //     client = server.available();   // listen for incoming clients

        // Check machine state
        data = status.read();
    //     updateTimer = millis();
    //     // if you get a client request, responce web page, parse the request and return robo action
    //     if (client) processWebRequest(&client, &action);

        if(action.updated){
            debug.println("updated", DEBUG_ACTION);
            roomba.driveDirect(action.motorR, action.motorL);

            roomba.toggleCleaning(action.cleaning);
            action.updated = false;
        }

        if(data.buttonA_pressed){
            showQR();
            debug.ledOn();
            roomba.playBeep(APP_BEEP);
            debug.println(F("QR"), DEBUG_GENERAL);
            
            action.stop();
            roomba.driveDirect(0, 0);

        }
        if (data.buttonB_pressed){
            debug.ledOff();
            M5.Lcd.clear();
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("Cleaning Start");
            roomba.toggleCleaning(true);
            // action.stop();
            roomba.driveDirect(0, 0);
        }
        if (data.buttonC_pressed){
            M5.Lcd.clear();
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println("Cleaning Stop");
            // action.stop();
            roomba.toggleCleaning(false);
            roomba.driveDirect(0, 0);
        }
    //     else{
    //     }
    // }
    delay(1);
}