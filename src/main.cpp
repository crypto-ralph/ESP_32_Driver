#include <Arduino.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <HTTPClient.h>

#define ONE_WIRE_BUS1 25
#define ONE_WIRE_BUS2 26

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

DS18B20 sensor1(&oneWire1);
DS18B20 sensor2(&oneWire2);

EthernetClient client;
EthernetServer server = EthernetServer(80);

hw_timer_t *timer = NULL;

byte mac[] = {
    0x56, 0x99, 0x52, 0x31, 0xD6, 0x53};

// IPAddress ip(192, 168, 0, 24);
char domoticz_server[] = "192.168.0.164";


bool sendLogMessage(String msg, int lvl);
bool sendTempData(float temp, int dev_idx);
void receiveDomoticzData(bool serialPrint = false);
bool sendDomoticzData(String data);
String analyzeIncomingRequestHeader(String & request);

volatile bool shouldSendTemp = false;

// Timer interrupt for sending temperature flag.
void IRAM_ATTR onTimer()
{
  shouldSendTemp = true;
}

void setup()
{
  Ethernet.init(5);
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("DS18B20 Library version: ");
  Serial.println(DS18B20_LIB_VERSION);
  sensor1.begin();
  sensor2.begin();

  Ethernet.begin(mac);
  server.begin();

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
  //String message = "Device MASTER_ESP booted with IP address:";
  String message = "Device ESP_MASTER booted.";
  sendLogMessage(message, 2);
  delay(100);
  receiveDomoticzData(true);

  timer = timerBegin(0, 80, true); // timer 0, prescaler 80, count up
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5000000, true);
  timerAlarmEnable(timer);
}

void send_standard_http_response(EthernetClient & client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>ok</html>");
}

void loop()
{
  if(shouldSendTemp)
  {
    float temp1 = 0.0;
    float temp2 = 0.0;
    bool result = true;

    // print the temperature when available and request a new reading
    if (sensor1.isConversionComplete())
    {
      temp1 = sensor1.getTempC();
      sensor1.requestTemperatures();
    }
    if (sensor2.isConversionComplete())
    {
      temp2 = sensor2.getTempC();
      sensor2.requestTemperatures();
    }

    sendTempData(temp1, 1);
    sendTempData(temp2, 2);

    Serial.println("temp sent to domoticz");
    delay(10);
    // receiveDomoticzData(true);
    shouldSendTemp = false;
  }

  EthernetClient server_client = server.available();
  if (server_client) 
  {
    if (server_client.available()) 
    {
      String request = server_client.readStringUntil('\0');
      String device = analyzeIncomingRequestHeader(request);
      Serial.print("Device is: ");
      Serial.println(device);
      send_standard_http_response(server_client);
    }
    delay(1); // give the web browser time to receive the data
    server_client.stop(); // close the connection:
  }
}

String analyzeIncomingRequestHeader(String & request)
{
  int spaceIndex = request.indexOf(' ');
  int httpIndex = request.indexOf(" HTTP/");
  String address = request.substring(spaceIndex+1, httpIndex);
  int index = address.lastIndexOf('/') + 1; // find index of first "/"

  if (index != -1)
  { 
    // if "/" is found
    String output = address.substring(index); // extract substring after "/"
    return output;
  }
  return "";
}


bool sendLogMessage(String msg, int lvl)
{
  msg.replace(" ", "%20");
  String data = "type=command&param=addlogmessage&message=" + msg + "&level=" + String(lvl);
  return sendDomoticzData(data);
}

bool sendTempData(float temp, int dev_idx)
{
  String data = "type=command&param=udevice&idx=" + String(dev_idx) + "&nvalue=0&svalue=" + String(temp);
  return sendDomoticzData(data);
}


bool sendDomoticzData(String data)
{
  // Serial.println(data);
  if (client.connect(domoticz_server, 8080)) {
    client.print("GET /json.htm?");
    client.print(data);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(domoticz_server);
    client.println("Connection: close");
    client.println();
  } else {
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
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (serialPrint) Serial.write(buffer, len); // show in the serial monitor (slows some boards)
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) 
  {
    client.stop();
  }
}
