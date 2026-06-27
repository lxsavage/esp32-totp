#ifndef TOTP_H_INCLUDED
#define TOTP_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "constants.hpp"

#include "storage.hpp"

namespace totp
{
// Initialize crypto libraries needed; run this again if the private key
// changes
void init();

// Get a TOTP URI and attempt to decode it; looking for URI in the form
// otpauth://totp/<label>?secret=<base32 key>. If not an otpauth:// URI, just
// does a normal base32 decode and leaves label empty.
//
// NOTE: this doesn't decode correctly due to a bug if secret is not the first
// URI parameter!
bool decode_totpauth(const char* uri, struct storage::OTPCode& out);

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
