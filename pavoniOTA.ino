#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#include "pavoni.h"
#include "credentials.h"

#define ADC_PIN A0
#define RELAY_PIN 14
#define UPDATE_DELAY_MS 100
#define SEND_DELAY_MS 1000


void wifi_init(const char* ssid, const char* password);

void ota_init(uint16_t port, const char* name);
void ota_loop(void);

void mqtt_init(const char* server);
void mqtt_loop(void);
void mqtt_reconnect(void);
void mqtt_callback(String topic, byte* message, unsigned int length);

void send_pressure(float pressure);


Pavoni pavoni;

WiFiClient espClient;
PubSubClient client(espClient);

int nextUpdate;
int nextSend;


void setup()
{
    wifi_init(MY_SSID, MY_PW);
    ota_init(8266, "esp-pavoni");
    mqtt_init(MQTT_SERVER);

    pavoni.Init(ADC_PIN, RELAY_PIN, 1.5f);

    nextUpdate = millis() + UPDATE_DELAY_MS;
    nextSend = millis() + SEND_DELAY_MS;
}


void loop()
{
    ota_loop();
    mqtt_loop();

    int now = millis();

    if (nextUpdate < now)
    {  
        nextUpdate = now + UPDATE_DELAY_MS;
        pavoni.Update();
    }

    if (nextSend < now)
    {
        nextSend = now + SEND_DELAY_MS;
        send_pressure(pavoni.GetPressure());
    }
}


void wifi_init(const char* ssid, const char* password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    } 
}

void ota_init(uint16_t port, const char* name)
{
    ArduinoOTA.setPort(port);
    ArduinoOTA.setHostname(name);

    ArduinoOTA.onStart([]()
        {
            Serial.println("Start");
        });

    ArduinoOTA.onEnd([]()
        {
            Serial.println("\nEnd");
        });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
        {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });

    ArduinoOTA.onError([](ota_error_t error)
        {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });


    ArduinoOTA.begin();  
}

void ota_loop(void)
{
    ArduinoOTA.handle();
}

void mqtt_init(const char* server)
{
    client.setServer(server, 1883);
    client.setCallback(mqtt_callback);
}

void mqtt_loop(void)
{
    if (!client.connected())
    {
        mqtt_reconnect();
    }
    if (!client.loop())
    {
        client.connect("ESP8266Client");
    }
}

void mqtt_reconnect(void)
{
    while (!client.connected())
    {
        if (client.connect("ESP8266Client"))
        {
            client.subscribe("home/pavoni");
            client.subscribe("home/pavoni/maxpressure");
        }
        else
        {
            delay(5000);
        }
    }
}

void mqtt_callback(String topic, byte* message, unsigned int length)
{
    String messageTemp;

    for (unsigned int i = 0; i < length; i++)
    {
        messageTemp += (char)message[i];
    }

    if (topic == "home/pavoni")
    {
        if (messageTemp == "on")
        {
            pavoni.On();
        }
        else if (messageTemp == "off")
        {
            pavoni.Off();
        }
    }
    else if (topic == "home/pavoni/maxpressure")
    {
        float temp = messageTemp.toFloat();
        if (temp >= 1.0f && temp <= 2.0f)
        {
            pavoni.SetMaxPressure(temp);
        }
    } 
}

void send_pressure(float pressure)
{
    char msg[10];
    sprintf(msg, "%.2f", pressure);

    client.publish("home/pavoni/pressure", msg, strlen(msg));
}
