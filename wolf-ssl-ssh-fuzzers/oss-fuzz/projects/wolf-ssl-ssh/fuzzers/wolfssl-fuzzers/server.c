#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <fuzzers/shared.h>

#define NUM_METHODS 5

WOLFSSL_CTX* ctx[NUM_METHODS];
WOLFSSL_METHOD* method[NUM_METHODS];

static int fuzzer_ticket_cb(WOLFSSL* ssl,
        byte key_name[WOLFSSL_TICKET_NAME_SZ],
        byte iv[WOLFSSL_TICKET_IV_SZ],
        byte mac[WOLFSSL_TICKET_MAC_SZ],
        int enc, byte* ticket, int inLen, int* outLen,
        void* userCtx)
{
    (void)ssl;
    (void)userCtx;

    /* TODO memory test parameters */

    memset(ticket, 0, inLen);
    *outLen = inLen;

    return WOLFSSL_TICKET_RET_OK;
}

FUZZER_INITIALIZE_HEADER
{
    method[0] = wolfTLSv1_2_server_method();
    method[1] = wolfTLSv1_1_server_method();
    method[2] = wolfTLSv1_3_server_method();
    method[3] = wolfDTLSv1_2_server_method();
    method[4] = wolfDTLSv1_3_server_method();

    for (int i = 0; i < NUM_METHODS; i++) {
        if ( (ctx[i] = wolfSSL_CTX_new(method[i])) == NULL) {
            abort();
        }

        /* TODO retval */ wolfSSL_CTX_SetIORecv(ctx[i], fuzzer_recv);
        /* TODO retval */ wolfSSL_CTX_SetIOSend(ctx[i], fuzzer_send);

        /* Load server certs into ctx */
        if (wolfSSL_CTX_use_certificate_file(ctx[i], get_certs_path((*argv)[0], "server-cert.pem"),
                    SSL_FILETYPE_PEM) != SSL_SUCCESS) {
            abort();
        }

        /* Load server key into ctx */
        if (wolfSSL_CTX_use_PrivateKey_file(ctx[i], get_certs_path((*argv)[0], "server-key.pem"),
                    SSL_FILETYPE_PEM) != SSL_SUCCESS) {
            abort();
        }

        if ( wolfSSL_CTX_set_TicketEncCb(ctx[i], fuzzer_ticket_cb) != WOLFSSL_SUCCESS ) {
            abort();
        }
        if ( wolfSSL_CTX_set_cipher_list(ctx[i], "TLS13-AES128-GCM-SHA256:TLS13-AES256-GCM-SHA384:TLS13-CHACHA20-POLY1305-SHA256:TLS13-AES128-CCM-SHA256:TLS13-AES128-CCM-8-SHA256:TLS13-SHA256-SHA256:TLS13-SHA384-SHA384:RC4-SHA:RC4-MD5:DES-CBC3-SHA:AES128-SHA:AES256-SHA:NULL-MD5:NULL-SHA:NULL-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:DHE-PSK-AES256-GCM-SHA384:DHE-PSK-AES128-GCM-SHA256:PSK-AES256-GCM-SHA384:PSK-AES128-GCM-SHA256:DHE-PSK-AES256-CBC-SHA384:DHE-PSK-AES128-CBC-SHA256:PSK-AES256-CBC-SHA384:PSK-AES128-CBC-SHA256:PSK-AES128-CBC-SHA:PSK-AES256-CBC-SHA:DHE-PSK-AES128-CCM:DHE-PSK-AES256-CCM:PSK-AES128-CCM:PSK-AES256-CCM:PSK-AES128-CCM-8:PSK-AES256-CCM-8:DHE-PSK-NULL-SHA384:DHE-PSK-NULL-SHA256:PSK-NULL-SHA384:PSK-NULL-SHA256:PSK-NULL-SHA:HC128-MD5:HC128-SHA:HC128-B2B256:AES128-B2B256:AES256-B2B256:RABBIT-SHA:NTRU-RC4-SHA:NTRU-DES-CBC3-SHA:NTRU-AES128-SHA:NTRU-AES256-SHA:AES128-CCM-8:AES256-CCM-8:ECDHE-ECDSA-AES128-CCM:ECDHE-ECDSA-AES128-CCM-8:ECDHE-ECDSA-AES256-CCM-8:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-RC4-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-RC4-SHA:ECDHE-ECDSA-DES-CBC3-SHA:AES128-SHA256:AES256-SHA256:DHE-RSA-AES128-SHA256:DHE-RSA-AES256-SHA256:ECDH-RSA-AES128-SHA:ECDH-RSA-AES256-SHA:ECDH-ECDSA-AES128-SHA:ECDH-ECDSA-AES256-SHA:ECDH-RSA-RC4-SHA:ECDH-RSA-DES-CBC3-SHA:ECDH-ECDSA-RC4-SHA:ECDH-ECDSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDH-RSA-AES128-GCM-SHA256:ECDH-RSA-AES256-GCM-SHA384:ECDH-ECDSA-AES128-GCM-SHA256:ECDH-ECDSA-AES256-GCM-SHA384:CAMELLIA128-SHA:DHE-RSA-CAMELLIA128-SHA:CAMELLIA256-SHA:DHE-RSA-CAMELLIA256-SHA:CAMELLIA128-SHA256:DHE-RSA-CAMELLIA128-SHA256:CAMELLIA256-SHA256:DHE-RSA-CAMELLIA256-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDH-RSA-AES128-SHA256:ECDH-ECDSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDH-RSA-AES256-SHA384:ECDH-ECDSA-AES256-SHA384:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-CHACHA20-POLY1305:DHE-RSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305-OLD:ECDHE-ECDSA-CHACHA20-POLY1305-OLD:DHE-RSA-CHACHA20-POLY1305-OLD:ADH-AES128-SHA:ADH-AES256-GCM-SHA384:QSH:RENEGOTIATION-INFO:IDEA-CBC-SHA:ECDHE-ECDSA-NULL-SHA:ECDHE-PSK-NULL-SHA256:ECDHE-PSK-AES128-CBC-SHA256:PSK-CHACHA20-POLY1305:ECDHE-PSK-CHACHA20-POLY1305:DHE-PSK-CHACHA20-POLY1305:EDH-RSA-DES-CBC3-SHA:WDM-NULL-SHA256") != WOLFSSL_SUCCESS ) {
            abort();
        }
    }
}
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    WOLFSSL* ssl;

    for (int i = 0; i < NUM_METHODS; i++) {
        fuzzer_set_data(data, size);

        if ( (ssl = wolfSSL_new(ctx[i])) == NULL) {
            /* wolfSSL_new() may fail if allocation randomization is enabled */
            fuzzer_unset_data();
            goto end;
        }

        if (wolfSSL_accept(ssl) == SSL_SUCCESS) {
            int n;
            char buf[4096];
            if ( (n = wolfSSL_read(ssl, buf, (sizeof(buf) -1))) > 0) { }
            memset(buf, 0, sizeof(buf));
            if ( (n = wolfSSL_write(ssl, buf, sizeof(buf))) > 0 ) { }
        }

        /* noret */ wc_ClearErrorNodes();
        wolfSSL_free(ssl);

        fuzzer_unset_data();
    }
}
end:
FUZZER_RUN_FOOTER
