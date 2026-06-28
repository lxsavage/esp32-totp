#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>

#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h>

#include <Base32-Decode.h>

#include "constants.hpp"
#include "rtc.hpp"
#include "storage.hpp"
#include "totp.hpp"

static struct storage::OTPCode decoded_key;
static struct storage::WiFiDetails network;

static hd44780_pinIO* lcd = nullptr;

void load_mode();

void loop()
{
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

    if (rtc::sync(&network, false))
        Serial.println("Synchronized RTC clock!");

    esp_light_sleep_start();
}

void setup()
{
    // initArduino();

    esp_sleep_enable_timer_wakeup(TOTP_POLL_NS);
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
    }
    else
    {
        lcd->write("TOTP for");
        lcd->setCursor(0, 1);
        lcd->write(decoded_key.label);
    }

    // Delay at least LABEL_READ_TIME, but skip the delay if it took longer than
    // that to do a RTC sync
    int64_t sync_start_ts = esp_timer_get_time() / 1000;
    bool last_sync_successful = rtc::sync(&network, true);

    if (!rtc::ready() && !last_sync_successful)
    {
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

    // Early return for no label; don't need to worry about reading a label!
    if (decoded_key.label_len == 0)
        return;

    int64_t sync_end_ts = esp_timer_get_time() / 1000;
    if (sync_end_ts - sync_start_ts > LABEL_READ_TIME)
        return;

    delay(LABEL_READ_TIME - sync_end_ts - sync_start_ts);

    // Print a static TOTP text mask to prevent redrawing every letter every
    // loop iteration to reduce text dimming from unnecessary updates
    lcd->clear();
    lcd->setCursor(0, 1);
    lcd->write("Expires in   s");

    // Max-priority task for the main update loop
    // xTaskCreatePinnedToCore(task_totp_sync, "main_loop", 1000, NULL,
    //                         configMAX_PRIORITIES - 1, NULL, tskNO_AFFINITY);
}

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
