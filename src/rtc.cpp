#include <Arduino.h>

#include "constants.hpp"
#include "rtc.hpp"
#include "storage.hpp"

#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

namespace rtc
{
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP);

static uint64_t start_ts;
static uint64_t last_sync_microsec;

bool sync(struct storage::WiFiDetails* wifi, bool print_errors)
{
    // note that the wifi param is explicitly a pointer to allow for nulls; in
    // the future, this is to support other external scenarios outside of WiFi
    // for synchronizing the RTC

    uint64_t now = esp_timer_get_time();

    // Restart the ESP32 if the millis has overflowed; easier to do it this way
    // than to account for it in timing code
    if (now < last_sync_microsec)
        ESP.restart();

    // Should skip processing if already set at least once, and
    // TIME_SYNC_INTERVAL has not been reached yet
    if (start_ts && (now - last_sync_microsec) / 1000 < TIME_SYNC_INTERVAL_SEC)
    {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi->ssid, wifi->ppk);
    while (WiFi.status() != WL_CONNECTED)
    {
        if (WiFi.status() == WL_NO_SSID_AVAIL ||
            WiFi.status() == WL_CONNECT_FAILED)
        {
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);

            if (print_errors)
                Serial.println("ERROR: failed to connect to WiFi with provided "
                               "credentials");
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    timeClient.begin();
    timeClient.update();

    last_sync_microsec = esp_timer_get_time();
    start_ts = (uint64_t)timeClient.getEpochTime() * 1000000;

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return true;
}

bool ready() { return start_ts != 0; }

unsigned long get()
{
    // Account for the number of seconds since the last sync
    return (start_ts + (esp_timer_get_time() - last_sync_microsec)) / 1000000;
}
} // namespace rtc
