#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>

#include "esp_sleep.h"
#include "esp_system.h"

#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h>

#include <Base32-Decode.h>

#include "constants.hpp"
#include "storage.hpp"
#include "time.hpp"
#include "totp.hpp"

static bool first_time;

static struct storage::OTPCode decoded_key;
static struct storage::WiFiDetails network;

static hd44780_pinIO* lcd = nullptr;

void load_mode()
{
    struct storage::OTPCode code;
    struct storage::WiFiDetails network = {.ppk = {'\0'}, .ssid = {'\0'}};

    size_t key_len;
    storage::init();

    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->write("LOAD MODE  > ...");
    lcd->setCursor(0, 1);
    lcd->write("Waiting for key");
    Serial.println("Waiting for key...");

    unsigned long debounce_helper = millis();
    while (!Serial.available())
    {
        delay(10);
        if (millis() - debounce_helper > 2000 && digitalRead(LOAD_BTN) == HIGH)
        {
            Serial.println("Skipped new key loading");
            goto load_mode_network;
        }
    }
    char key_buf[256];
    key_len = Serial.readBytesUntil('\n', key_buf, 255);
    key_buf[key_len] = '\0';

    totp::decode_totpauth(key_buf, code);

    Serial.print("label [");
    Serial.print(code.label_len);
    Serial.print("]: ");
    Serial.println(code.label);

    Serial.print("key [");
    Serial.print(code.key_len);
    Serial.print("]: ");
    Serial.println((char*)code.key);

    storage::write_secret(code);
    if (!storage::commit_writes())
    {
        Serial.println("Saving key failed");
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->write("ERROR");
        lcd->setCursor(0, 1);
        lcd->write("Key not saved");
        delay(2000);
        return;
    }
    Serial.println("Key commit successful");

    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->write("LOAD MODE");
    lcd->setCursor(0, 1);
    lcd->write("Saved key!");
    delay(2000);

load_mode_network:
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->write("LOAD MODE    ...");
    lcd->setCursor(0, 1);
    lcd->write("Waiting for SSID");
    Serial.println("Waiting for SSID...");

    while (!Serial.available())
    {
        delay(10);
    }

    key_len = Serial.readBytesUntil('\n', network.ssid, 32);
    network.ppk[key_len] = '\0';

    Serial.print("SSID: ");
    Serial.println(network.ssid);

    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->write("LOAD MODE    ...");
    lcd->setCursor(0, 1);
    lcd->write("Waiting for PPK");
    Serial.println("Waiting for PPK...");

    while (!Serial.available())
    {
        delay(10);
    }

    key_len = Serial.readBytesUntil('\n', network.ppk, 64);
    network.ppk[key_len] = '\0';

    Serial.print("PPK: ");
    Serial.println(network.ppk);

    storage::write_secret(network);
    if (!storage::commit_writes())
    {
        Serial.println("Saving network failed");
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->write("ERROR");
        lcd->setCursor(0, 1);
        lcd->write("Network not saved");
        delay(2000);
        return;
    }

    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->write("LOAD MODE");
    lcd->setCursor(0, 1);
    lcd->write("Saved network!");
    delay(2000);
}

void setup()
{
    esp_sleep_enable_timer_wakeup(POLL_NS);
    Serial.begin(BAUD_RATE);

    lcd = new hd44780_pinIO(RS, ENABLE, D4, D5, D6, D7);
    lcd->begin(16, 2);

    pinMode(LOAD_BTN, INPUT);
    if (digitalRead(LOAD_BTN) == HIGH)
    {
        load_mode();
        ESP.restart();
    }

    storage::init();
    storage::load_wifi(network);
    storage::load_privatekey(decoded_key);

    Serial.print("Syncing time using WiFi network \"");
    Serial.print(network.ssid);
    Serial.println("\"");

    totp::init();

    lcd->clear();
    lcd->setCursor(0, 0);

    if (decoded_key.label_len == 0)
    {
        lcd->write("Waiting for");
        lcd->setCursor(0, 1);
        lcd->write("NTP sync...");
        first_time = rtc::sync(network);
        return;
    }

    lcd->write("TOTP for");
    lcd->setCursor(0, 1);
    lcd->write(decoded_key.label);

    // Sync WiFi, and otherwise delay for at least 2000 milliseconds so that the
    // user can see the label
    unsigned long sync_start_ts = millis();
    first_time = rtc::sync(network);
    unsigned long sync_end_ts = millis();

    if (sync_end_ts - sync_start_ts > LABEL_READ_TIME)
        return;

    delay(LABEL_READ_TIME - sync_end_ts - sync_start_ts);
}

void loop()
{
    if (!rtc::ready())
    {
        if (first_time)
        {
            esp_light_sleep_start();
            return;
        }

        // rtc::sync and rtc::ready both returning false indicates connection
        // failure
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->write("ERROR");
        lcd->setCursor(0, 1);
        lcd->write("Reconfigure WiFi");

        // Unrecoverable: lock until manual reset
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        delay(100);
        esp_deep_sleep_start();
    }

    // Print a static text mask to prevent redrawing every letter every update
    if (first_time)
    {
        lcd->clear();
        lcd->setCursor(0, 1);
        lcd->write("Expires in   s");
    }

    unsigned long now = rtc::get();

    static char code_buf[7];
    if (!totp::generate(decoded_key.key, decoded_key.key_len, now, code_buf))
    {
        Serial.println("Failed to generate TOTP code");
        esp_light_sleep_start();
        return;
    }

    lcd->setCursor(0, 0);
    for (int i = 0; i < 6; i++)
    {
        if (i == 3)
            lcd->write(' ');
        lcd->write(code_buf[i]);
    }

    static char exp_buf[3];
    snprintf(exp_buf, 3, "%02u", 30 - (now % 30));
    lcd->setCursor(11, 1);
    lcd->write(exp_buf);

    esp_light_sleep_start();

    // Poll to see if the clock needs to sync and sync if necessary
    first_time = rtc::sync(network);
}
