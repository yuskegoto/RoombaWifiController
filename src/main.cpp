#include <Arduino.h>
#include <M5Stack.h>

#include <Wifi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <DNSServer.h>

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
// const char *password = "";
const byte DNS_PORT = 53;
IPAddress apIP(ACCESSPOINT_IP1, ACCESSPOINT_IP2, ACCESSPOINT_IP3, ACCESSPOINT_IP4);
DNSServer dnsServer;
WiFiServer server(80);

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

// like a split JS
String parseString(int idSeparator, char separator, String str)
{
	String output = "";
	int separatorCout = 0;
	for (int i = 0; i < str.length(); i++)
	{
		if ((char)str[i] == separator)
		{
			separatorCout++;
		}
		else
		{
			if (separatorCout == idSeparator)
			{
				output += (char)str[i];
			}
			else if (separatorCout > idSeparator)
			{
				break;
			}
		}
	}
	return output;
}

void showQR(){
    M5.Lcd.clear();
  // M5.Lcd.qrcode(const char *string, uint16_t x = 50, uint16_t y = 10, uint8_t width = 220, uint8_t version = 6);
    M5.Lcd.qrcode("http://192.168.4.1");
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

void processWebRequest(WiFiClient* client, Action* act){
    debug.println(F("New Client."), DEBUG_CONTROL);           // print a message out the serial port
    debug.ledOn();

    String currentLine = "";                // make a String to hold incoming data from the client
    while (client->connected()) {            // loop while the client's connected
        if (client->available()) {             // if there's bytes to read from the client,
            char c = client->read();             // read a byte, then
            debug.print(c, DEBUG_CONTROL);                    // print it out the serial monitor
            if (c == '\n') {                    // if the byte is a newline character

                // if the current line is blank, you got two newline characters in a row.
                // that's the end of the client HTTP request, so send a response:
                if (currentLine.length() == 0) {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    String content = openPage("/");
                    client->println("HTTP/1.1 200 OK");
                    client->println("Content-type:text/html");
                    client->println();

                    // // the content of the HTTP response follows the header:

                    if (content != "")
                    {
                        for (int i = 0; i < cntChrs(content, '\n'); i++)
                        {
                            String oneHTMLLine = parseString(i, '\n', content);
                            // debug.println(oneHTMLLine, DEBUG_CONTROL);
                            client->print(oneHTMLLine);
                        }
                    }
                    // // The HTTP response ends with another blank line:
                    client->println();

                    break;
                }
                else {    // if you got a newline, then clear currentLine:
                currentLine = "";
                }
            }
            else if (c != '\r') {  // if you got anything else but a carriage return character,
                currentLine += c;      // add it to the end of the currentLine
            }

            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /F")) act->forward();
            if (currentLine.endsWith("GET /R")) act->turnRight();
            if (currentLine.endsWith("GET /L")) act->turnLeft();
            if (currentLine.endsWith("GET /S")) act->stop();
            if (currentLine.endsWith("GET /B")) act->back();
            if (currentLine.endsWith("GET /C")) act->toggleCleaningMotors();
        }
    }
    debug.ledOff();
    // close the connection:
    client->stop();
    debug.println(F("Client Disconnected."), DEBUG_CONTROL);
    // return action;
}

void setupWiFi(String myName)
{
// on wifi ap mode
    WiFi.disconnect();   //added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_OFF); //added to start with the wifi off, avoid crashing
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(myName.c_str());
    // WiFi.softAP(ssid);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", apIP);
    server.begin();

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
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,0);

    roomba.roboInitSequence();

    // WiFi.softAP(ssid, password);
    // server.begin();
    String deviceName = SSID;
    setupWiFi(deviceName);

    if (!SD.begin()) M5.Lcd.println("Insert SD and tap power button plz.");

    debug.println(F("Init done"), DEBUG_GENERAL);
    M5.Lcd.println("Init done");
}


void loop() {
    if(millis() - updateTimer > UPDATE_RATE_ms){
        dnsServer.processNextRequest();
        WiFiClient client;
        SensorData data;

        client = server.available();   // listen for incoming clients

        // Check machine state
        data = status.read();
        updateTimer = millis();
        // if you get a client request, responce web page, parse the request and return robo action
        if (client) processWebRequest(&client, &action);

        if(action.updated){
            debug.println("updated", DEBUG_CONTROL);
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
        else{
        }
    }
    delay(1);
}