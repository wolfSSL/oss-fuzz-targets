WOLFSSH_A_PATH = ../src/.libs/libwolfssh.a
WOLFSSL_A_PATH = ../../wolfssl/src/.libs/libwolfssl.a
WOLFSSL_A_TRACE_PC_GUARD_PATH = ../../wolfssl_trace_pc_guard/src/.libs/libwolfssl.a

FUZZERS_CFLAGS = $(CFLAGS) -DFUZZER_WOLFSSH -I .. -I $(FUZZERS_INCLUDE_PATH)
FUZZERS_LIBS = $(LIBFUZZER_A_PATH) $(WOLFSSH_A_PATH) $(WOLFSSL_A_PATH)
FUZZERS_LIBS_GV = $(LIBFUZZER_GV_A_PATH) $(WOLFSSH_A_PATH) $(WOLFSSL_A_TRACE_PC_GUARD_PATH)

all : fuzzer-client fuzzer-client-intensity fuzzer-client-allocation fuzzer-server fuzzer-server-intensity fuzzer-server-allocation

client.o : client.c
	$(CC) $(FUZZERS_CFLAGS) client.c -c -o client.o
fuzzer-client : client.o
	$(CXX) $(CXXFLAGS) client.o $(FUZZERS_LIBS) -o fuzzer-client
fuzzer-client-intensity : client.o
	$(CXX) $(CXXFLAGS) client.o $(FUZZERS_LIBS_GV) -o fuzzer-client-intensity
client-allocation.o : client.c
	$(CC) -DFUZZER_ALLOCATION_GUIDED $(FUZZERS_CFLAGS) client.c -c -o client-allocation.o
fuzzer-client-allocation : client-allocation.o
	$(CXX) $(CXXFLAGS) client-allocation.o $(FUZZERS_LIBS_GV) -o fuzzer-client-allocation

server.o : server.c
	$(CC) $(FUZZERS_CFLAGS) server.c -c -o server.o
fuzzer-server : server.o
	$(CXX) $(CXXFLAGS) server.o $(FUZZERS_LIBS) -o fuzzer-server
fuzzer-server-intensity : server.o
	$(CXX) $(CXXFLAGS) server.o $(FUZZERS_LIBS_GV) -o fuzzer-server-intensity
server-allocation.o : server.c
	$(CC) -DFUZZER_ALLOCATION_GUIDED $(FUZZERS_CFLAGS) server.c -c -o server-allocation.o
fuzzer-server-allocation : server-allocation.o
	$(CXX) $(CXXFLAGS) server-allocation.o $(FUZZERS_LIBS_GV) -o fuzzer-server-allocation
