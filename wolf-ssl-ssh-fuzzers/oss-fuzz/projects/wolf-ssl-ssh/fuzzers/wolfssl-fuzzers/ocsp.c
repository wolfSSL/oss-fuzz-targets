#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/asn.h>
#include <fuzzers/shared.h>

FUZZER_INITIALIZE_HEADER
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    OcspResponse resp;
    OcspEntry single;
    CertStatus status;
    InitOcspResponse(&resp, &single, &status, data, size, NULL);
    OcspResponseDecode(&resp, NULL, NULL, 1, 1);
    FreeOcspResponse(&resp);

    InitOcspResponse(&resp, &single, &status, data, size, NULL);
    OcspResponseDecode(&resp, NULL, NULL, 1, 0);
    FreeOcspResponse(&resp);
}
FUZZER_RUN_FOOTER
