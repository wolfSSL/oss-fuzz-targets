#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/asn.h>
#include <wolfssl/wolfcrypt/curve25519.h>
#include <wolfssl/wolfcrypt/curve448.h>
#include <wolfssl/wolfcrypt/ed448.h>
#include <wolfssl/wolfcrypt/ed25519.h>
#include <wolfssl/wolfcrypt/pkcs7.h>
#include <fuzzers/shared.h>

FUZZER_INITIALIZE_HEADER
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    if ( size < 1 ) {
        return 0;
    }

    const uint8_t choice = *data;
    data++; size--;

    switch ( choice ) {
        case    0:
            {
                uint32_t outSz;

                if ( size < sizeof(outSz) ) {
                    return 0;
                }

                memcpy(&outSz, data, sizeof(outSz));
                data += sizeof(outSz); size -= sizeof(outSz);

                outSz &= 0xFFFFFF;

                uint8_t* out = malloc(outSz);
                if ( Base64_Decode(data, size, out, &outSz) == 0 ) {
                    memory_test(out, outSz);
                }
                free(out);
            }
            break;
        case    1:
            {
                uint32_t outSz;
                uint8_t mode;

                if ( size < sizeof(outSz) ) {
                    return 0;
                }

                memcpy(&outSz, data, sizeof(outSz));
                data += sizeof(outSz); size -= sizeof(outSz);

                outSz &= 0xFFFFFF;

                if ( size < sizeof(mode) ) {
                    return 0;
                }

                memcpy(&mode, data, sizeof(mode));
                data += sizeof(mode); size -= sizeof(mode);

                uint8_t* out = malloc(outSz);
                int ret = -1;
                switch ( mode % 3 ) {
                    case    0:
                        ret = Base64_Encode(data, size, out, &outSz);
                        break;
                    case    1:
                        ret = Base64_EncodeEsc(data, size, out, &outSz);
                        break;
                    case    2:
                        ret = Base64_Encode_NoNl(data, size, out, &outSz);
                        break;
                }
                if ( ret == 0 ) {
                    memory_test(out, outSz);
                }
                free(out);
            }
            break;
        case    2:
            {
                uint32_t outSz;

                if ( size < sizeof(outSz) ) {
                    return 0;
                }

                memcpy(&outSz, data, sizeof(outSz));
                data += sizeof(outSz); size -= sizeof(outSz);

                outSz &= 0xFFFFFF;

                uint8_t* out = malloc(outSz);
                if ( Base16_Decode(data, size, out, &outSz) == 0 ) {
                    memory_test(out, outSz);
                }
                free(out);
            }
            break;
        case    3:
            {
                uint32_t outSz;

                if ( size < sizeof(outSz) ) {
                    return 0;
                }

                memcpy(&outSz, data, sizeof(outSz));
                data += sizeof(outSz); size -= sizeof(outSz);

                outSz &= 0xFFFFFF;

                uint8_t* out = malloc(outSz);
                if ( Base16_Encode(data, size, out, &outSz) == 0 ) {
                    memory_test(out, outSz);
                }
                free(out);
            }
            break;
        case    4:
            {
                uint32_t outSz;

                if ( size < sizeof(outSz) ) {
                    return 0;
                }

                memcpy(&outSz, data, sizeof(outSz));
                data += sizeof(outSz); size -= sizeof(outSz);

                outSz &= 0xFFFFF;

                uint8_t* out = malloc(outSz);

                if ( wc_BerToDer(data, size, out, &outSz) == 0 ) {
                    memory_test(out, outSz);
                }

                free(out);
            }
            break;
        case    5:
            {
                wc_curve25519_check_public(data, size, EC25519_LITTLE_ENDIAN);
                wc_curve25519_check_public(data, size, EC25519_BIG_ENDIAN);
            }
            break;
        case    6:
            {
                wc_curve448_check_public(data, size, EC448_LITTLE_ENDIAN);
                wc_curve448_check_public(data, size, EC448_BIG_ENDIAN);
            }
            break;
        case    7:
            {
                    ed448_key key;
                    if ( wc_ed448_init(&key) == 0 ) {
                        if ( wc_ed448_import_public(data, size, &key) == 0 ) {
                            unsigned char out[ED448_PUB_KEY_SIZE];
                            wc_ed448_make_public(&key, out, ED448_PUB_KEY_SIZE);
                        }
                        wc_ed448_free(&key);
                    }
            }
            break;
        case    8:
            {
                    ed25519_key key;
                    if ( wc_ed25519_init(&key) == 0 ) {
                        if ( wc_ed25519_import_public(data, size, &key) == 0 ) {
                            unsigned char out[ED25519_PUB_KEY_SIZE];
                            wc_ed25519_make_public(&key, out, ED25519_PUB_KEY_SIZE);
                        }
                        wc_ed25519_free(&key);
                    }
            }
            break;
        case    9:
            {
                    curve25519_key key;
                    if ( wc_curve25519_init(&key) == 0 ) {
                        if ( wc_curve25519_import_public(data, size, &key) == 0 ) {
                            unsigned char out[CURVE25519_KEYSIZE];
                            word32 outLen = sizeof(out);
                            wc_curve25519_export_public(&key, out, &outLen);
                        }
                        wc_curve25519_free(&key);
                    }
            }
            break;
        case    10:
            {
                    curve448_key key;
                    if ( wc_curve448_init(&key) == 0 ) {
                        if ( wc_curve448_import_public(data, size, &key) == 0 ) {
                            unsigned char out[CURVE448_PUB_KEY_SIZE];
                            word32 outLen = sizeof(out);
                            wc_curve448_export_public(&key, out, &outLen);
                        }
                        wc_curve448_free(&key);
                    }
            }
            break;
        case    11:
            {
                    ed25519_key key;
                    if ( wc_ed25519_init(&key) == 0 ) {
                        wc_ed25519_import_private_only(data, size, &key);
                        wc_ed25519_free(&key);
                    }
            }
            break;
        case    12:
            {
                    ed448_key key;
                    if ( wc_ed448_init(&key) == 0 ) {
                        wc_ed448_import_private_only(data, size, &key);
                        wc_ed448_free(&key);
                    }
            }
            break;
        case    13:
            {
                    curve25519_key key;
                    if ( wc_curve25519_init(&key) == 0 ) {
                        wc_curve25519_import_private_raw(data, size, NULL, 0, &key);
                        wc_curve25519_free(&key);
                    }
            }
            break;
        case    14:
            {
                    curve448_key key;
                    if ( wc_curve448_init(&key) == 0 ) {
                        wc_curve448_import_private_raw(data, size, NULL, 0, &key);
                        wc_curve448_free(&key);
                    }
            }
            break;
        case    15:
            {
                ecc_key key;
                if ( wc_ecc_init(&key) == 0 ) {
                    wc_ecc_import_private_key(data, size, NULL, 0, &key);
                    wc_ecc_free(&key);
                }
            }
            break;
        case    16:
            {
                PKCS7* pkcs7 =  wc_PKCS7_New(NULL, -1);
                if ( pkcs7 != NULL ) {
                    if ( wc_PKCS7_InitWithCert(pkcs7, (unsigned char*)data, size) == 0 ) {
                        uint8_t output[1024];
                        if ( wc_PKCS7_EncodeData(pkcs7, output, sizeof(output)) > 0 ) {
                        }
                    }
                    wc_PKCS7_Free(pkcs7);
                }
            }
            break;
    }
}
FUZZER_RUN_FOOTER
