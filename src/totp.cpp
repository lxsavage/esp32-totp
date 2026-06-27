#include "totp.hpp"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <Base32-Decode.h>

#include "storage.hpp"

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

bool decode_totpauth(const char* uri, struct storage::OTPCode& out)
{
    const char* prefix = "otpauth://totp/";
    const char* keyprefix = "?secret=";
    const int prefix_len = 15;
    const int keyprefix_len = 8;

    size_t uri_i = 0;
    size_t uri_len = strlen(uri);

    out.label[0] = '\0';
    out.label_len = 0;
    if (strncmp(uri, prefix, prefix_len) == 0)
    {
        uri_i = prefix_len;

        size_t label_i = 0;
        while (uri_i < uri_len)
        {
            if (uri[uri_i] == '?' || label_i >= sizeof(out.label) - 1)
                break;

            out.label[label_i++] = uri[uri_i++];
        }
        out.label[label_i] = '\0';
        out.label_len = label_i;

        if (strncmp(&uri[uri_i], keyprefix, keyprefix_len) != 0)
            return false;
        uri_i += keyprefix_len;
    }

    char key_buf[256];
    key_buf[0] = '\0';
    size_t key_i = 0;
    while (uri_i < uri_len)
    {
        if (uri[uri_i] == '&')
            break;

        if (key_i >= 255)
            return false;

        key_buf[key_i++] = uri[uri_i++];
    }
    key_buf[key_i] = '\0';
    out.key_len = base32decode(key_buf, out.key, TOTP_KEY_MAX);
    return true;
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

    uint8_t hash[20];
    mbedtls_md_hmac_starts(&ctx, key, key_len);
    mbedtls_md_hmac_update(&ctx, msg, 8);
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
