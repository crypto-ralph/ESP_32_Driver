#ifndef httpUtils_h
#define httpUtils_h

#include <Arduino.h>
#include <Ethernet.h>

const String HTTP_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

void analyzeIncomingRequestHeader(const String &header, String &actionOut, String &requestOut)
{
  int spaceIndex = header.indexOf(' ');
  int httpIndex = header.indexOf(" HTTP/");
  String address = header.substring(spaceIndex + 1, httpIndex);
  int index = address.lastIndexOf('/') + 1;

  // Find the position of the question mark '?' or the end of the address
  int questionMarkIndex = address.indexOf('?');
  if (questionMarkIndex == -1) {
    questionMarkIndex = address.length();
  }

  // Assign the action and request parts
  actionOut = address.substring(index, questionMarkIndex);
  requestOut = address.substring(index);
}

String send_http_response(const String &content)
{
  String response = HTTP_RESPONSE_HEADER;
  response += content;
  return response;
}

String create_welcome_page(const char *domoticzIp)
{
  String html = "<!DOCTYPE html><html><head><title>Welcome Page</title></head><body>";
  html += "<h1>Hello! This is your device's welcome page.</h1>";
  html += "<p>Domoticz IP: ";
  html += domoticzIp;
  html += "</p>";
  html += "<h2>Configured Sensors:</h2>";
  html += getSensorListHtml();
  html += "</body></html>";
  return html;
}

#endif // httpUtils_h