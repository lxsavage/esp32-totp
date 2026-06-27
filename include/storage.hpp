#ifndef STORAGE_H_INCLUDED
#define STORAGE_H_INCLUDED

#include <stddef.h>

#include "constants.hpp"

namespace storage
{
struct OTPCode
{
    size_t label_len;
    size_t key_len;
    char label[256];
    unsigned char key[65];
};

struct WiFiDetails
{
    char ppk[64];
    char ssid[32];
};

// Initialize storage for reading/writing
void init();

// Get the TOTP secret from storage
void load_privatekey(struct OTPCode& out);

// Get the WiFi credentials from storage
void load_wifi(struct WiFiDetails& out);

// Write a new secret to storage
void write_secret(struct OTPCode& in);

// Write new WiFi credentials to storage
void write_secret(struct WiFiDetails& in);

// Apply any pending write calls to storage
bool commit_writes();
} // namespace storage

#endif
