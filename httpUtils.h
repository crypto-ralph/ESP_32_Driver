#ifndef httpUtils_h
#define httpUtils_h

#include <Arduino.h>
#include <Ethernet.h>

const String HTTP_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

void send_standard_http_response(EthernetClient &client)
{
  client.print(HTTP_RESPONSE_HEADER);
  client.println("<!DOCTYPE HTML>");
  client.println("<html>ok</html>");
}

void send_welcome_page(EthernetClient &client, IPAddress deviceIP)
{
  String html = HTTP_RESPONSE_HEADER +
                "<!DOCTYPE HTML>\r\n"
                "<html>\r\n"
                "<head>\r\n"
                "<title>Welcome</title>\r\n"
                "</head>\r\n"
                "<body>\r\n"
                "<h1>Welcome to ESP_MASTER!</h1>\r\n"
                "<p>Device IP: " + deviceIP.toString() + "</p>\r\n"
                "</body>\r\n"
                "</html>\r\n";

  client.print(html);
}

#endif // httpUtils_h