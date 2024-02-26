/**
 * @file FreeWeather.h
 * @author Donocean
 * @version 1.0.1
 *
 * @copyright (c) 2024, TreeBot Development Team
 *
 * SPDX-License-Identifier: GNU-GPL v3.0
 */
#ifndef __FREE_WEATHER_H_
#define __FREE_WEATHER_H_

#include <WiFi.h>
#include <Arduino.h>

/* comment this macro to get city code from http server */
#define STATIC_GET

typedef struct
{
    float temperature;
    float precipitation;  // unit: mm
    float humidity;       // unit: %

    String weather;
    String windPower;
    String windDirection;

    int aqi;
    String airQuality;
} weatherInfo_t;

class FreeWeather
{
public:
    FreeWeather(void);
    void begin(String province = "本地", String city = "-");

    bool getWeatherInfo(void);
    String getCurrentCity(void);
    String getCurrentProvince(void);
    void setCityPosition(String province = "本地", String city = "-");

    weatherInfo_t info;

private:
    void _getLocalInfo(String &province, String &city);
    void _parseWeatherInfo(WiFiClient &httpClient);
    bool request(String req, const char *host, WiFiClient &wifiClient);
    void staticGetCityCode(void);
    void dynamicGetCityCode(void);

    String _province;
    String _city;
    String _cityCode;

    static const char *_host; 
};

#endif /* __FREE_WEATHER_H_ */