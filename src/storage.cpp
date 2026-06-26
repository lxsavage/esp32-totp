#include "storage.hpp"

#include <EEPROM.h>

namespace storage
{
bool initialized = false;
void init()
{
    if (initialized)
        return;

    EEPROM.begin(sizeof(struct PrivateKey) + sizeof(struct NetworkData));
    initialized = true;
}

void load_privatekey(struct PrivateKey& out) { EEPROM.get(0, out); }

void load_wifi(struct NetworkData& out)
{
    EEPROM.get(sizeof(struct PrivateKey), out);
}

void write_privatekey(struct PrivateKey& in) { EEPROM.put(0, in); }

void write_wifi(struct NetworkData& in)
{
    EEPROM.put(sizeof(struct PrivateKey), in);
}

bool commit_writes() { return EEPROM.commit(); }
} // namespace storage
