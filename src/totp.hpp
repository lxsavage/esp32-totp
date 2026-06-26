#ifndef TOTP_H_INCLUDED
#define TOTP_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

namespace totp
{
// Initialize crypto libraries needed; run this again if the private key
// changes
void init();

// Calculate a TOTP code for the given key and time
// - key/key_len: the decoded private key and its character count.
// - time: the current unix timestamp, which is canonically a
//         32-bit integer
// - out: a buffer of at least 7 bytes to store the output/terminator in
//
// Returns true if successful, false otherwise.
bool generate(unsigned char* key, size_t key_len, unsigned long time,
              char* out);
} // namespace totp

#endif
