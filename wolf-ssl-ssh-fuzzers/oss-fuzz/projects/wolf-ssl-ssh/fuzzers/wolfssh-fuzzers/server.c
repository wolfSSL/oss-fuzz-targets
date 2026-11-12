#include <wolfssh/ssh.h>
#include <wolfssh/wolfscp.h>
#include <fuzzers/shared.h>

WOLFSSH_CTX* ctx = NULL;

static const char serverBanner[] = "wolfSSH Example Server\n";

static int wsUserAuth(byte authType,
                      WS_UserAuthData* authData,
                      void* ctx)
{
    /* TODO */
    return WOLFSSH_USERAUTH_SUCCESS;
    //return WOLFSSH_USERAUTH_FAILURE;
}

int fuzzerScpRecvCallback(WOLFSSH* ssh, int state, const char* basePath,
        const char* fileName, int fileMode, word64 mTime, word64 aTime,
        word32 totalFileSz, byte* buf, word32 bufSz, word32 fileOffset,
        void* ctx) {
    return WS_SCP_ABORT;
}

int fuzzerScpSendCallback(WOLFSSH* ssh, int state, const char* peerRequest,
        char* fileName, word32 fileNameSz, word64* mTime, word64* aTime,
        int* fileMode, word32 fileOffset, word32* totalFileSz, byte* buf,
        word32 bufSz, void* ctx) {
    return WS_SCP_ABORT;
}

FUZZER_INITIALIZE_HEADER
{
    if (wolfSSH_Init() != WS_SUCCESS) {
        abort();
    }

    ctx = wolfSSH_CTX_new(WOLFSSH_ENDPOINT_SERVER, NULL);
    if ( ctx == NULL ) {
        abort();
    }

    /* noret */ wolfSSH_SetUserAuth(ctx, wsUserAuth);

    if ( wolfSSH_CTX_SetBanner(ctx, serverBanner) != WS_SUCCESS ) {
        abort();
    }

    /* noret */ wolfSSH_SetIORecv(ctx, fuzzer_recv);
    /* noret */ wolfSSH_SetIOSend(ctx, fuzzer_send);

    /* noret */ wolfSSH_SetScpRecv(ctx, fuzzerScpRecvCallback);
    /* noret */ wolfSSH_SetScpSend(ctx, fuzzerScpSendCallback);
}
FUZZER_INITIALIZE_FOOTER_1
FUZZER_INITIALIZE_FOOTER_2

FUZZER_RUN_HEADER
{
    WOLFSSH* ssh = NULL;
    int ret;

    fuzzer_set_data(data, size);
    ssh = wolfSSH_new(ctx);

    if ( (ret = wolfSSH_accept(ssh)) != WS_SUCCESS ) {
        goto end;
    }


end:
    ret = wolfSSH_shutdown(ssh);
    wolfSSH_free(ssh);
    fuzzer_unset_data();
}
FUZZER_RUN_FOOTER
