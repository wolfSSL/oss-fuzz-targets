#include <wolfssh/ssh.h>
#include <fuzzers/shared.h>

WOLFSSH_CTX* ctx = NULL;

FUZZER_INITIALIZE_HEADER
{
    if (wolfSSH_Init() != WS_SUCCESS) {
        abort();
    }

    ctx = wolfSSH_CTX_new(WOLFSSH_ENDPOINT_CLIENT, NULL);
    if ( ctx == NULL ) {
        abort();
    }

    /* noret */ wolfSSH_SetIORecv(ctx, fuzzer_recv);
    /* noret */ wolfSSH_SetIOSend(ctx, fuzzer_send);
}
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    WOLFSSH* ssh = NULL;
    int ret;

    fuzzer_set_data(data, size);
    ssh = wolfSSH_new(ctx);
    if ( ssh == NULL ) return 0;
    if ( (ret = wolfSSH_SetUsername(ssh, "U")) != WS_SUCCESS ) {
        goto end;
    }

    if ( (ret = wolfSSH_connect(ssh)) != WS_SUCCESS ) {
        goto end;
    }

end:
    ret = wolfSSH_shutdown(ssh);
    wolfSSH_free(ssh);
    fuzzer_unset_data();
}
FUZZER_RUN_FOOTER
