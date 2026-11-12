#!/bin/bash -eu

set -e

rm -rf $SRC/fuzzers/wolfssl/
mkdir -p $SRC/fuzzers/wolfssl/
cp -R $SRC/wolfssl/ $SRC/fuzzers/wolfssl/
cp -R $SRC/fuzzers/wolfssl-fuzzers $SRC/fuzzers/wolfssl/wolfssl/
if [[ "$OSS_FUZZ_BUILD" -eq "0" ]]; then
    cp -R $SRC/fuzzers/wolfssl/wolfssl/ $SRC/fuzzers/wolfssl/wolfssl_trace_pc_guard/
fi

export CFLAGS="$CFLAGS -DWOLFSSL_STATIC_PSK"
export WOLFSSL_CONFIGURE_PARAMS="$WOLFSSL_BASE_CONFIGURE_PARAMS --enable-aesccm --enable-aescfb --enable-aesctr --enable-aeseax --enable-aesgcm-stream --enable-aeskeywrap --enable-aesofb --enable-aessiv --enable-arc4 --enable-asn=original --enable-blake2 --enable-blake2s --enable-camellia --enable-certext --enable-cmac --enable-compkey --enable-crl --enable-cryptocb --enable-curve25519 --enable-curve448 --enable-des3 --enable-dsa --enable-dtls --enable-dtls13 --enable-dtlscid --enable-earlydata --enable-ecccustcurves --enable-ecccustcurves=all --enable-eccencrypt --enable-eccsi --enable-ed25519 --enable-ed25519-stream --enable-ed448 --enable-ed448-stream --enable-harden --enable-hkdf --enable-hrrcookie --enable-indef --enable-keygen --enable-md2 --enable-md4 --enable-nullcipher --enable-ocsp --enable-ocspstapling --enable-oldtls --enable-opensslall --enable-opensslextra --enable-postauth --enable-psk --enable-pwdbased --enable-ripemd --enable-scrypt --enable-secure-renegotiation --enable-session-ticket --enable-shake128 --enable-shake256 --enable-siphash --enable-smallstack --enable-sni --enable-srp --enable-srtp --enable-sslv3 --enable-tls13 --enable-tlsv10 --enable-tlsx --enable-x963kdf --enable-xchacha --enable-xts --with-eccminsz=0 --enable-hpke --enable-quic --enable-ocspstapling2 --enable-pkcs7"

# Build everything with -fsanitize=fuzzer-no-link (normal code coverage guided fuzzing)
    # Build wolfSSL
        cd $SRC/fuzzers/wolfssl/wolfssl/
        autoreconf -ivf
        ./configure $WOLFSSL_CONFIGURE_PARAMS
        make -j$(nproc)

    # Build wolfSSL fuzzers (code coverage guided)
        cd $SRC/fuzzers/wolfssl/wolfssl/wolfssl-fuzzers

        make -B fuzzer-client
        make -B fuzzer-server
        make -B fuzzer-misc
        make -B fuzzer-crl
        make -B fuzzer-ocsp
        make -B fuzzer-x509
        if [[ "$OSS_FUZZ_BUILD" -eq "0" ]]; then
            make -B fuzzer-ocsp-lookup
        fi
        make -B fuzzer-rsa
        make -B fuzzer-srp

        cp fuzzer-client $OUT/fuzzer-wolfssl-client
        cp fuzzer-server $OUT/fuzzer-wolfssl-server
        cp fuzzer-misc $OUT/fuzzer-wolfssl-misc
        cp fuzzer-crl $OUT/fuzzer-wolfssl-crl
        cp fuzzer-ocsp $OUT/fuzzer-wolfssl-ocsp
        cp fuzzer-x509 $OUT/fuzzer-wolfssl-x509
        if [[ "$OSS_FUZZ_BUILD" -eq "0" ]]; then
            cp fuzzer-ocsp-lookup $OUT/fuzzer-wolfssl-ocsp-lookup
        fi
        cp fuzzer-rsa $OUT/fuzzer-wolfssl-rsa
        cp fuzzer-srp $OUT/fuzzer-wolfssl-srp

        if [[ "$OSS_FUZZ_BUILD" -eq "1" ]]; then
            CFLAGS="$CFLAGS -DOSS_FUZZ_BUILD_RANDOMIZE" make -B fuzzer-client
            cp fuzzer-client $OUT/fuzzer-wolfssl-client-randomize
            CFLAGS="$CFLAGS -DOSS_FUZZ_BUILD_RANDOMIZE" make -B fuzzer-server
            cp fuzzer-server $OUT/fuzzer-wolfssl-server-randomize
        fi

        if [[ "$OSS_FUZZ_BUILD" -eq "1" ]]; then
            zip $OUT/fuzzer-wolfssl-client_seed_corpus.zip corp-client/* >/dev/null
            zip $OUT/fuzzer-wolfssl-client-randomize_seed_corpus.zip corp-client-rand/* >/dev/null
            zip $OUT/fuzzer-wolfssl-server_seed_corpus.zip corp-server/* >/dev/null
            zip $OUT/fuzzer-wolfssl-server-randomize_seed_corpus.zip corp-server-rand/* >/dev/null
            zip $OUT/fuzzer-wolfssl-misc_seed_corpus.zip corp-misc/* >/dev/null
            zip $OUT/fuzzer-wolfssl-crl_seed_corpus.zip corp-crl/* >/dev/null
            zip $OUT/fuzzer-wolfssl-ocsp_seed_corpus.zip corp-ocsp/* >/dev/null
            zip $OUT/fuzzer-wolfssl-x509_seed_corpus.zip corp-x509/* >/dev/null
            zip $OUT/fuzzer-wolfssl-rsa_seed_corpus.zip corp-rsa/* >/dev/null
            zip $OUT/fuzzer-wolfssl-srp_seed_corpus.zip corp-srp/* >/dev/null
        else
            cp -R corp-client/ $OUT/corp-wolfssl-client/
            cp -R corp-server/ $OUT/corp-wolfssl-server/
            cp -R corp-misc/ $OUT/corp-wolfssl-misc/
            cp -R corp-crl/ $OUT/corp-wolfssl-crl/
            cp -R corp-ocsp/ $OUT/corp-wolfssl-ocsp/
            cp -R corp-x509/ $OUT/corp-wolfssl-x509/
            cp -R corp-rsa/ $OUT/corp-wolfssl-rsa/
            cp -R corp-srp/ $OUT/corp-wolfssl-srp/
        fi

# Build everything with -fsanitize-coverage=trace-pc-guard (for intensity and allocation guided fuzzing)
if [[ "$OSS_FUZZ_BUILD" -eq "0" ]]; then
    if [[ $CFLAGS != *-m32* ]]
    then
        export CFLAGS=${CFLAGS/"-fsanitize=fuzzer-no-link"/"-fsanitize-coverage=trace-pc-guard"}
        # Build wolfSSL
            cd $SRC/fuzzers/wolfssl/wolfssl_trace_pc_guard/
            autoreconf -ivf
            ./configure $WOLFSSL_CONFIGURE_PARAMS
            make -j$(nproc)

        # Build wolfSSL fuzzers (intensity guided)
            cd $SRC/fuzzers/wolfssl/wolfssl_trace_pc_guard/wolfssl-fuzzers

            make -B fuzzer-client-intensity
            make -B fuzzer-server-intensity
            make -B fuzzer-misc-intensity
            make -B fuzzer-crl-intensity
            make -B fuzzer-ocsp-intensity
            make -B fuzzer-x509-intensity
            make -B fuzzer-ocsp-lookup-intensity

            cp fuzzer-client-intensity $OUT/fuzzer-wolfssl-client-intensity
            cp fuzzer-server-intensity $OUT/fuzzer-wolfssl-server-intensity
            cp fuzzer-misc-intensity $OUT/fuzzer-wolfssl-misc-intensity
            cp fuzzer-crl-intensity $OUT/fuzzer-wolfssl-crl-intensity
            cp fuzzer-ocsp-intensity $OUT/fuzzer-wolfssl-ocsp-intensity
            cp fuzzer-x509-intensity $OUT/fuzzer-wolfssl-x509-intensity
            cp fuzzer-ocsp-lookup-intensity $OUT/fuzzer-wolfssl-ocsp-lookup-intensity

        # Build wolfSSL fuzzers (allocation guided)
            cd $SRC/fuzzers/wolfssl/wolfssl_trace_pc_guard/wolfssl-fuzzers

            make -B fuzzer-client-allocation
            make -B fuzzer-server-allocation
            make -B fuzzer-misc-allocation
            make -B fuzzer-crl-allocation
            make -B fuzzer-ocsp-allocation
            make -B fuzzer-x509-allocation
            make -B fuzzer-ocsp-lookup-allocation

            cp fuzzer-client-allocation $OUT/fuzzer-wolfssl-client-allocation
            cp fuzzer-server-allocation $OUT/fuzzer-wolfssl-server-allocation
            cp fuzzer-misc-allocation $OUT/fuzzer-wolfssl-misc-allocation
            cp fuzzer-crl-allocation $OUT/fuzzer-wolfssl-crl-allocation
            cp fuzzer-ocsp-allocation $OUT/fuzzer-wolfssl-ocsp-allocation
            cp fuzzer-x509-allocation $OUT/fuzzer-wolfssl-x509-allocation
            cp fuzzer-ocsp-lookup-allocation $OUT/fuzzer-wolfssl-ocsp-lookup-allocation
    fi
fi

cp -R $SRC/fuzzers/wolfssl/wolfssl/certs/ $OUT/
