#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#include <stddef.h>

#include "constants.hpp"

namespace storage
{
struct PrivateKey
{
    size_t len;
    unsigned char key[TOTP_KEY_MAX];
};

struct NetworkData
{
    char ppk[64];
    char ssid[32];
};

// Initialize storage for reading/writing
void init();

// Get the TOTP secret from storage
void load_privatekey(struct PrivateKey& out);

// Get the WiFi credentials from storage
void load_wifi(struct NetworkData& out);

// Write a new secret to storage
void write_privatekey(struct PrivateKey& in);

// Write new WiFi credentials to storage
void write_wifi(struct NetworkData& in);

// Apply any pending write calls to storage
bool commit_writes();
} // namespace storage

#endif
