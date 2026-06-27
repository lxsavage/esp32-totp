#include "storage.hpp"

#include <EEPROM.h>

namespace storage
{
bool initialized = false;
void init()
{
    if (initialized)
        return;

    EEPROM.begin(sizeof(struct OTPCode) + sizeof(struct WiFiDetails));
    initialized = true;
}

void load_privatekey(struct OTPCode& out) { EEPROM.get(0, out); }

void load_wifi(struct WiFiDetails& out)
{
    EEPROM.get(sizeof(struct OTPCode), out);
}

void write_secret(struct OTPCode& in) { EEPROM.put(0, in); }

void write_secret(struct WiFiDetails& in)
{
    EEPROM.put(sizeof(struct OTPCode), in);
}

bool commit_writes() { return EEPROM.commit(); }
} // namespace storage
