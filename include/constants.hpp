#ifndef CONSTANTS_HPP_INCLUDED
#define CONSTANTS_HPP_INCLUDED

#include <stdint.h>

// Pinout for LCD to display codes/status messages
const int RS = 13;
const int ENABLE = 14;
const int D4 = 25;
const int D5 = 26;
const int D6 = 18;
const int D7 = 19;

// Pinout for pushbutton to enter load mode
const int LOAD_BTN = 21;

// Period for re-syncing time with NTP server (seconds)
const unsigned long TIME_SYNC_INTERVAL_SEC = 86400; // 1 day

// Other constants
const int BAUD_RATE = 115200;
const int TOTP_KEY_MAX = 65;

const uint32_t LABEL_READ_TIME = 2000; // 2 seconds
const uint64_t POLL_NS = 1000000;      // 1 second

#endif
