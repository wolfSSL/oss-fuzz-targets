#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/asn.h>
#include <wolfssl/internal.h>
#include <fuzzers/shared.h>

FUZZER_INITIALIZE_HEADER
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

static void DecodedCert_memory_test(const DecodedCert* cert) {
    memory_test(cert->publicKey, cert->pubKeySize);
    memory_test(&cert->pubKeyStored, sizeof(cert->pubKeyStored));
    memory_test(cert->subjectCN, cert->subjectCNLen);
    memory_test(cert->extensions, cert->extensionsSz);
    memory_test(cert->extAuthInfo, cert->extAuthInfoSz);
    memory_test(cert->extCrlInfo, cert->extCrlInfoSz);
}

FUZZER_RUN_HEADER
{
    {
        DecodedCert cert;
        InitDecodedCert(&cert, data, size, 0);
        if ( ParseCert(&cert, CERT_TYPE, NO_VERIFY, NULL) == 0 ) {
            DecodedCert_memory_test(&cert);

            {
                WOLFSSL_X509 x509;
                InitX509(&x509, 0, NULL);
                CopyDecodedToX509(&x509, &cert);
                FreeX509(&x509);
            }
            {
                OcspRequest req;
                if ( InitOcspRequest(&req, &cert, 0, NULL) == 0 ) {
                    int encodedSz = EncodeOcspRequest(&req, NULL, 0);
                    if ( encodedSz > 0 ) {
                        byte* out = malloc(encodedSz);
                        if ( EncodeOcspRequest(&req, out, encodedSz) > 0 ) {
                            memory_test(out, encodedSz);
                        }
                        free(out);
                    }
                    FreeOcspRequest(&req);
                }
            }
        }
        FreeDecodedCert(&cert);
    }

    {
        DecodedCert cert;
        InitDecodedCert(&cert, data, size, 0);
        DecodeToKey(&cert, 0);
        FreeDecodedCert(&cert);
    }

    {
        int outSz = wc_DerToPem(data, size, NULL, 0, CERT_TYPE);
        if ( outSz > 0 ) {
            unsigned char* out = malloc(outSz);
            wc_DerToPem(data, size, out, outSz, CERT_TYPE);
            /* TODO memory test on out */
            free(out);
        }
    }
}
FUZZER_RUN_FOOTER
