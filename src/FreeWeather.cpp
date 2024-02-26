/**
 * @file FreeWeather.cpp
 * @author Donocean
 * @version 1.0.1
 *
 * @copyright (c) 2024, TreeBot Development Team
 *
 * SPDX-License-Identifier: GNU-GPL v3.0
 */
#include "FreeWeather.h"
#include <ArduinoJson.h>
#include <HardwareSerial.h>

#ifdef STATIC_GET
#include "CityData.h"
/* avoid error when compiling */
extern const char *provinceCode;
#elif
#include "ProvinceCode.h"
#endif

// server address
const char *FreeWeather::_host = "www.nmc.cn";

FreeWeather::FreeWeather(void) {}
String FreeWeather::getCurrentProvince(void) { return _province; }
String FreeWeather::getCurrentCity(void) { return _city; }

void FreeWeather::begin(String province, String city)
{
    setCityPosition(province, city);
}

void FreeWeather::_getLocalInfo(String &province, String &city)
{
    WiFiClient _wifiClient;
    String reqRes = "/";
    const char *ipHost = "myip.ipip.net";

    province = "";
    city = "";

    if (request(reqRes, ipHost, _wifiClient))
    {
        uint8_t spaceNum = 0;
        String info = _wifiClient.readString();

        for (uint16_t i = 0; i < info.length(); i++)
        {
            if (info[i] == ' ')
                spaceNum++;
            else if (spaceNum == 4)
                province += info[i];
            else if (spaceNum == 5)
                city += info[i];

            if (spaceNum == 6)
                break;
        }

        _wifiClient.stop();
    }
    else 
    {
        Serial.println("http conncet failed!");
        _wifiClient.stop();
    }
}

void FreeWeather::setCityPosition(String province, String city)
{
    if (province == "本地")
        _getLocalInfo(province, city);

    _province = province;

    if (city == "-")
        _city = _province;
    else
        _city = city;

    // if (province == "香港" || province == "澳门")
    //     _city = province;
    
    /* get city code */
#ifdef STATIC_GET
    staticGetCityCode();
#elif
    dynamicGetCityCode();
#endif
}

void FreeWeather::staticGetCityCode(void)
{
    JsonDocument doc;
    deserializeJson(doc, cityData);
    _cityCode = doc[_province][_city].as<String>();
}

void FreeWeather::dynamicGetCityCode(void)
{
    WiFiClient wifiClient;
    JsonDocument docProvince;
    String req = "/rest/province/";

    deserializeJson(docProvince, provinceCode);
    req += docProvince[_province]["code"].as<String>();

    if (request(req, _host, wifiClient))
    {
        String info;
        JsonDocument doc;

        /* skip msg length taht read from server */
        wifiClient.find('\r');

        /* read json format msg */
        info = wifiClient.readStringUntil('\r');

        deserializeJson(doc, info);
        JsonArray arr = doc.as<JsonArray>();

        for (const JsonObject &repo : arr)
        {
            String httpCity = repo["city"].as<String>();
            if (httpCity == _city)
            {
                _cityCode = repo["code"].as<String>();
                break;
            }
        }

        wifiClient.stop();
    }
    else
    {
        Serial.println("http conncet failed!");
        wifiClient.stop();
    }
}

bool FreeWeather::getWeatherInfo(void)
{
    WiFiClient _wifiClient;
    String reqRes = "/rest/weather?stationid=" + _cityCode;

    /* get weather info */
    if (request(reqRes, _host, _wifiClient))
    {
        _parseWeatherInfo(_wifiClient);
        _wifiClient.stop();
        return true;
    }
    else
    {
        Serial.println("http conncet failed!");
        _wifiClient.stop();
        return false;
    }
}


/**
 * @brief       send the http request to the host server
 * 
 * @param req           the request that need to be sent 
 * @param host          the host that user want to send to
 * @param wifiClient    store the informations that read from the host server if this function return true
 * @return true         connection to the host was successful
 * @return false        connection to the host was failed
 */
bool FreeWeather::request(String req, const char *host, WiFiClient &wifiClient)
{
    uint8_t attempts = 6;

    String httpRequest = String("GET ") + req + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n";

    /* connecting to the host */
    do
    {
        wifiClient.stop();
        wifiClient.connect(host, 80);
        attempts--;
    } while (!wifiClient.connected() && attempts);

    /* connect success */
    if (wifiClient.connected())
    {
        /* send the http request to the host server */
        wifiClient.print(httpRequest);

        /* waiting http respond */
        unsigned long wtime = millis();
        while (!wifiClient.available())
        {
            /* wait 20s then restart board */
            if ((millis() - wtime) > 20000)
                ESP.restart();
        }

        String _status_response = wifiClient.readStringUntil('\n');
        String _response_code = _status_response.substring(9, 12);
        /* error if http response code not equal 200*/
        if (_response_code != "200")
        {
            wifiClient.stop();
            return false;
        }

        /* skip http responding head that read from server */
        wifiClient.find("\r\n\r\n");

        return true;
    }
    else 
    {
        wifiClient.stop();
        return false;
    }
}

// parse all the lines of the reply from server
void FreeWeather::_parseWeatherInfo(WiFiClient &httpClient)
{
    String line;
    JsonDocument doc;

    /* skip msg length taht read from server */
    httpClient.find('\r');

    /* read json format msg */
    line = httpClient.readStringUntil('\r');
    // Serial.println(line);

    deserializeJson(doc, line);

    info.temperature = doc["data"]["real"]["weather"]["temperature"].as<float>();
    info.humidity = doc["data"]["real"]["weather"]["humidity"].as<float>();
    info.weather = doc["data"]["real"]["weather"]["info"].as<String>();

    info.precipitation = doc["data"]["real"]["weather"]["rain"].as<float>();
    info.windDirection = doc["data"]["real"]["wind"]["direct"].as<String>();
    info.windPower = doc["data"]["real"]["wind"]["power"].as<String>();

    /* air*/
    info.aqi = doc["data"]["air"]["aqi"].as<int>();
    info.airQuality = doc["data"]["air"]["text"].as<String>();
}
