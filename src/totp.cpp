#include "totp.hpp"

#include <stdio.h>

#include <stddef.h>
#include <stdint.h>

#include "mbedtls/md.h"

namespace totp
{
static mbedtls_md_context_t ctx;
static bool initialized = false;

void init()
{
    if (initialized)
        return;

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1);
    initialized = true;
}

bool generate(unsigned char* key, size_t key_len, unsigned long time, char* out)
{
    if (!initialized)
        return false;

    uint64_t steps = time / 30;
    uint8_t msg[8];
    for (int i = 7; i >= 0; i--)
    {
        msg[i] = steps & 0xFF;
        steps >>= 8;
    }

    mbedtls_md_hmac_starts(&ctx, key, key_len);
    mbedtls_md_hmac_update(&ctx, msg, 8);
    uint8_t hash[20];
    mbedtls_md_hmac_finish(&ctx, hash);

    uint8_t offset = hash[19] & 0x0F;
    uint32_t bin_code =
        ((hash[offset] & 0x7F) << 24) | ((hash[offset + 1] & 0xFF) << 16) |
        ((hash[offset + 2] & 0xFF) << 8) | (hash[offset + 3] & 0xFF);
    bin_code %= 1000000;
    snprintf(out, 7, "%06lu", bin_code);
    return true;
}
} // namespace totp
