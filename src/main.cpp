#include <Arduino.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <HTTPClient.h>
#include <map>
#include <functional>
#include "DomoticzDevicesLists.h"
#include "DomoticzUtils.h"
#include "DomoticzAnalogOutput.h"
#include "httpUtils.h"

const int TIMER_PRESCALER = 80;
const unsigned long SENSOR_UPDATE_INTERVAL = 10; // in milliseconds
const uint32_t TIMER_ALARM_PERIOD = 5000000;     // 5 seconds
const int SERIAL_BAUD_RATE = 9600;
const int ETHERNET_CS_PIN = 5;
const int ETHERNET_PORT = 80;
const char domoticz_server[] = "192.168.0.164";

String process_relay_action(const char *domoticzIp, const String &actionRequest)
{
  int noIndex = actionRequest.indexOf("no=");
  int actionIndex = actionRequest.indexOf("action=");

  if (noIndex != -1 && actionIndex != -1)
  {
    int relayNo = actionRequest.substring(noIndex + 3, actionRequest.indexOf('&')).toInt();
    String action = actionRequest.substring(actionIndex + 7);

    for (auto &output : outputRelays)
    {
      if (output.getRelayNumber() == relayNo && output.processAction(action))
      {
        Serial.println("Action: " + actionRequest + " processed.");
        break;
      }
    }
  }
  return "OK";
}
std::map<String, std::function<String(const char *, const String &)>> endpointMap = {
    {"", [](const char *domoticzIp, const String &action)
     { return create_welcome_page(domoticzIp); }},
    {"relay", process_relay_action},
    // Add other endpoints and their corresponding functions here
};

EthernetServer server = EthernetServer(ETHERNET_PORT);
Domoticz domoticz = Domoticz(domoticz_server);

hw_timer_t *timer = NULL;

byte mac[] = {
    0x56, 0x99, 0x52, 0x31, 0xD6, 0x53};

// IPAddress ip(192, 168, 0, 24);

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
  domoticz.sendLogMessage(message, 2);
  delay(100);
  domoticz.receiveDomoticzData(true);
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

      domoticz.sendTempData(temperature, sensor.idx);
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
      String requestHeader = server_client.readStringUntil('\0');
      String action, request;
      analyzeIncomingRequestHeader(requestHeader, action, request);

      // Find the corresponding function for the given action in the map
      auto entry = endpointMap.find(action);

      // If the function is found, call it and send the response
      if (entry != endpointMap.end())
      {
        String responseContent = entry->second(domoticz_server, request);
        Serial.println(responseContent);
        String httpResponse = send_http_response(responseContent);
        Serial.println(httpResponse);
        server_client.print(httpResponse);
      }
    }
  }
  yield();              // Give the web browser time to receive the data
  server_client.stop(); // close the connection:
}