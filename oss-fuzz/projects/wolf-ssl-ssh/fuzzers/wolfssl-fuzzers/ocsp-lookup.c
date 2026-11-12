#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <fuzzers/shared.h>

FUZZER_INITIALIZE_HEADER
{
    printf(
            "Please ensure that EmbedOcspLookup does not result in any network IO.\n"
            "This requires making wolfIO_Recv, wolfIO_Send, wolfIO_TcpConnect\n"
            "and any other IO functions called by EmbedOcspLookup possible to replace\n"
            "with custom send/recv functions that draw data from the fuzzer input\n"
            "\n"
            "Once that is done, remove this warning. This program will now exit.\n"
          );
    exit(1);
}
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    char* url = NULL;
    unsigned char* ocspReq = NULL;
    unsigned char* ocspRespBuf = NULL;

    uint32_t urlSz, ocspReqSz;
    if ( size < sizeof(urlSz) + sizeof(ocspReqSz) ) {
        return 0;
    }

    memcpy(&urlSz, data, sizeof(urlSz)); data += sizeof(urlSz); size -= sizeof(urlSz);
    memcpy(&ocspReqSz, data, sizeof(ocspReqSz)); data += sizeof(ocspReqSz); size -= sizeof(ocspReqSz);

    if ( urlSz > size ) {
        return 0;
    }

    url = malloc(urlSz + 1);
    memcpy(url, data, urlSz); data += urlSz; size -= urlSz;
    url[urlSz] = 0;

    if ( ocspReqSz > size ) {
        goto end;
    }

    ocspReq = malloc(ocspReqSz);
    memcpy(ocspReq, data, ocspReqSz); data += ocspReqSz; size -= ocspReqSz;

    fuzzer_set_data(data, size);
    EmbedOcspLookup(NULL, url, urlSz, ocspReq, ocspReqSz, &ocspRespBuf);
    fuzzer_unset_data();

end:
    free(url);
    free(ocspReq);
    XFREE(ocspRespBuf, NULL, DYNAMIC_TYPE_OCSP);
}
FUZZER_RUN_FOOTER
