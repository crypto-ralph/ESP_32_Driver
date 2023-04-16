#include <Arduino.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <HTTPClient.h>
#include "DomoticzTempSensor.h"
#include "DomoticzDevicesLists.h"
#include "DomoticzOutput.h"

const int TIMER_PRESCALER = 80;
const unsigned long SENSOR_UPDATE_INTERVAL = 10; // in milliseconds
const uint32_t TIMER_ALARM_PERIOD = 5000000; // 5 seconds
const int SERIAL_BAUD_RATE = 9600;
const int ETHERNET_CS_PIN = 5;
const int ETHERNET_PORT = 80;

// Outputs config
DomoticzOutput outputRelays[] = {
    DomoticzOutput(22, "relay1on", "relay1off"),
};

EthernetClient client;
EthernetServer server = EthernetServer(ETHERNET_PORT);

hw_timer_t *timer = NULL;

byte mac[] = {
    0x56, 0x99, 0x52, 0x31, 0xD6, 0x53};

// IPAddress ip(192, 168, 0, 24);
char domoticz_server[] = "192.168.0.164";

bool sendLogMessage(String msg, int lvl);
bool sendTempData(float temp, String dev_idx);
void receiveDomoticzData(bool serialPrint = false);
bool sendDomoticzData(String data);
String analyzeIncomingRequestHeader(String &request);

volatile bool shouldSendTemp = false;
static size_t currentSensorIndex = 0;

// Timer interrupt for sending temperature flag.
void IRAM_ATTR onTimer()
{
  shouldSendTemp = true;
}

void setup()
{
  for (auto &sensor : temperatureSensors)
  {
    sensor.begin();
  }

  Ethernet.init(ETHERNET_CS_PIN);

  Serial.begin(SERIAL_BAUD_RATE);
  Ethernet.begin(mac);
  server.begin();

  Serial.println(__FILE__);
  Serial.print("DS18B20 Library version: ");
  Serial.println(DS18B20_LIB_VERSION);

  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }
  Serial.print("This device ip: ");
  Serial.println(Ethernet.localIP());

  // initialize timer and attach interrupt to it
  timer = timerBegin(0, TIMER_PRESCALER, true); // timer 0, prescaler 80, count up
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, TIMER_ALARM_PERIOD, true);
  timerAlarmEnable(timer);

  String message = "Device ESP_MASTER booted.";
  sendLogMessage(message, 2);
  delay(100);
  receiveDomoticzData(true);
}

void send_standard_http_response(EthernetClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close"); // the connection will be closed after completion of the response
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>ok</html>");
}

void loop()
{
  static unsigned long lastSensorUpdate = 0;

  if (shouldSendTemp)
  {
    static unsigned long lastSensorUpdate = 0;

    if (millis() - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL)
    {
      auto &sensor = temperatureSensors[currentSensorIndex];
      float temperature = sensor.getTemperature();
      Serial.print("Temperature Sensor ");
      Serial.print(sensor.idx);
      Serial.print(": ");
      Serial.print(temperature);
      Serial.println("C");

      sendTempData(temperature, sensor.idx);
      lastSensorUpdate = millis(); // Update the last sensor update timestamp

      // Move to the next sensor
      currentSensorIndex++;
      if (currentSensorIndex >= (sizeof(temperatureSensors) / sizeof(temperatureSensors[0])))
      {
        currentSensorIndex = 0;
        shouldSendTemp = false;
        Serial.println("temp sent to domoticz");
      }
    }
  }

  EthernetClient server_client = server.available();
  if (server_client)
  {
    if (server_client.available())
    {
      String request = server_client.readStringUntil('\0');
      String action = analyzeIncomingRequestHeader(request);

      send_standard_http_response(server_client);
      Serial.println(action);
      for (auto &output : outputRelays)
      {
        if (output.processAction(action))
        {
          Serial.println("Action: " + action + " processed.");
          break;
        }
      }
    }
    yield();              // Give the web browser time to receive the data
    server_client.stop(); // close the connection:
  }
}

String analyzeIncomingRequestHeader(String &request)
{
  int spaceIndex = request.indexOf(' ');
  int httpIndex = request.indexOf(" HTTP/");
  String address = request.substring(spaceIndex + 1, httpIndex);
  int index = address.lastIndexOf('/') + 1;

  if (index != -1)
  {
    // if "/" is found
    return address.substring(index); // return substring after "/"
  }
  return "";
}

bool sendLogMessage(String msg, int lvl)
{
  msg.replace(" ", "%20");
  String data = "type=command&param=addlogmessage&message=" + msg + "&level=" + String(lvl);
  return sendDomoticzData(data);
}

bool sendTempData(float temp, String dev_idx)
{
  String data = "type=command&param=udevice&idx=" + dev_idx + "&nvalue=0&svalue=" + String(temp);
  return sendDomoticzData(data);
}

bool sendDomoticzData(String data)
{
  // Serial.println(data);
  if (client.connect(domoticz_server, 8080))
  {
    client.print("GET /json.htm?");
    client.print(data);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(domoticz_server);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    // if you didn't get a connection to the server:
    Serial.println("Connection failed");
    return false;
  }
  return true;
}

void receiveDomoticzData(bool serialPrint)
{
  int len = client.available();
  if (len > 0)
  {
    byte buffer[80];
    if (len > 80)
      len = 80;
    client.read(buffer, len);
    if (serialPrint)
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
  }

  // if the server's disconnected, stop the client:
  if (!client.connected())
  {
    client.stop();
  }
  client.stop();
}
