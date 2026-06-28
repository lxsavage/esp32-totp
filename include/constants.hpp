#ifndef CONSTANTS_HPP_INCLUDED
#define CONSTANTS_HPP_INCLUDED

#include <stdint.h>

// Pinout for LCD to display codes/status messages
const int RS = 13;
const int ENABLE = 14;
const int D4 = 26;
const int D5 = 25;
const int D6 = 18;
const int D7 = 19;

// Pinout for pushbutton to enter load mode
const int LOAD_BTN = 21;

// Period for re-syncing time with NTP server (seconds); uses deep sleep between
// resynchronization events, so this is not an exact timing
const int64_t TIME_SYNC_INTERVAL_SEC = 86400; // 1 day

// Minimum amount of time to display label before code (if applicable)
const int64_t LABEL_READ_TIME = 1500; // 1.5 seconds

// Other constants
const int BAUD_RATE = 115200;
const int TOTP_KEY_MAX = 65;
const int64_t TOTP_POLL_NS = 1000000; // 1 second

#endif
