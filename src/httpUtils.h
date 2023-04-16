#ifndef httpUtils_h
#define httpUtils_h

#include <Arduino.h>
#include <Ethernet.h>

const String HTTP_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

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

void send_standard_http_response(EthernetClient &client)
{
  client.print(HTTP_RESPONSE_HEADER);
  client.println("<!DOCTYPE HTML>");
  client.println("<html>ok</html>");
}

// void send_welcome_page_response(EthernetClient &client, const char* domoticz_ip)
// {
//     client.print(
//         "HTTP/1.1 200 OK\r\n"
//         "Content-Type: text/html\r\n"
//         "Connection: close\r\n"
//         "\r\n"
//         "<!DOCTYPE HTML>"
//         "<html>"
//         "<head><title>Welcome</title></head>"
//         "<body>"
//         "<h1>Hello World!</h1>"
//         "<p>Device IP: ");
//     client.print(Ethernet.localIP());
//     client.print("</p>"
//         "<p>Domoticz IP: ");
//     client.print(domoticz_ip); // Add the Domoticz IP address here
//     client.print("</p>"
//         "</body>"
//         "</html>"
//     );
// }
void send_welcome_page_response(EthernetClient &client, const char *domoticzIp)
// String send_welcome_page_response(EthernetClient &client, const IPAddress &deviceIp, const char *domoticzIp)
{
  String html = "<!DOCTYPE html><html><head><title>Welcome Page</title></head><body>";
  html += "<h1>Hello! This is your device's welcome page.</h1>";
  // html += "<p>Device IP: ";
  // html += deviceIp.toString();
  // html += "</p>";
  html += "<p>Domoticz IP: ";
  html += domoticzIp;
  html += "</p>";
  html += "<h2>Configured Sensors:</h2>";
  html += getSensorListHtml();
  html += "</body></html>";
  // return html;
  client.print(html);
}

#endif // httpUtils_h