#ifndef DomoticzUtils_h
#define DomoticzUtils_h

#include <Arduino.h>
#include <Ethernet.h>


class Domoticz
{
private:
    EthernetClient client;
    const char* server_ip;

public:
    // Constructor
    Domoticz(const char* server_ip) : server_ip(server_ip)
    {
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
        if (client.connect(server_ip, 8080))
        {
            client.print("GET /json.htm?");
            client.print(data);
            client.println(" HTTP/1.1");
            client.print("Host: ");
            client.println(server_ip);
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
};

#endif // DomoticzUtils_h