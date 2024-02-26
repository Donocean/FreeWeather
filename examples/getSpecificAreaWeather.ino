/**
 * @file getSpecificAreaWeather.ino
 * @author Donocean
 *
 * @copyright (c) 2024, TreeBot Development Team
 *
 * SPDX-License-Identifier: GNU-GPL v3.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include "FreeWeather.h"

FreeWeather w = FreeWeather();

#define SSID "Your Wifi SSID"
#define PASSWORD "Your Wifi password"

void setup()
{
    Serial.begin(115200);

    WiFi.begin(SSID, PASSWORD);
    int t = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        t++;
        Serial.print(".");
        if (t == 5)
            ESP.restart();
    }

    // All of the provinces and citys can be found in the CityData.h
    w.begin("上海", "松江");
    if (w.getWeatherInfo())
    {
        Serial.println("city: " + w.getCurrentCity());
        Serial.println("temperature: " + String(w.info.temperature));
        Serial.println("weather: " + w.info.weather);
        Serial.println("air: " + w.info.airQuality);

        Serial.println("wind direct: " + w.info.windDirection);
        Serial.println("wind power: " + w.info.windPower);
    }

    w.setCityPosition("浙江", "杭州");
    if (w.getWeatherInfo())
    {
        Serial.println("city: " + w.getCurrentCity());
        Serial.println("temperature: " + String(w.info.temperature));
        Serial.println("weather: " + w.info.weather);
        Serial.println("air: " + w.info.airQuality);

        Serial.println("wind direct: " + w.info.windDirection);
        Serial.println("wind power: " + w.info.windPower);
    }

}

void loop()
{
}
