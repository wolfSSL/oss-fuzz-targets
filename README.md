# Fuzzers for wolfSSL and wolfSSH

## Building

From the ```oss-fuzz``` directory, run:

```sh
infra/helper.py build_fuzzers wolf-ssl-ssh
```

This builds the 64 bit AddressSanitizer based fuzzers.

If you want UndefinedBehaviorSanitizer, append ```--sanitizer=undefined```.

If you want MemorySanitizer, append ```--sanitizer=memory```.

If you want 32 bit + AddressSanitizer, append ```--architecture=i386```.

For each fuzzer target, three binaries are generated: One that is primarily guided by code coverage, one by intensity (finds slow inputs) and one by peak allocations (finds inputs that consume a lot of memory).

## Build output

After building, files will be in ```oss-fuzz/build/out/wolf-ssl-ssh/```.

Files include:

- A binary optimized for code coverage guided fuzzing: ```fuzzer-XXX```
- A binary optimized for intensity guided fuzzing: ```fuzzer-XXX-intensity```
- A binary optimized for allocation guided fuzzing: ```fuzzer-XXX-allocation```
- A corpus for the fuzzers: ```corp-XXX/```

where ```XXX``` is any of the fuzzer targets.

## Running

### UndefinedBehaviorSanitizer

Note that when running UBSan fuzzers, typically a lot of messages like

```
wolfcrypt/src/sha256.c:728:44: runtime error: unsigned integer overflow: 4219830777 + 550611398 cannot be represented in type 'unsigned int'
```

will be displayed initially. These are not actually undefined behavior, and can be safely ignored.

When the fuzzer detects real undefined behavior (eg. overshifting a signed variable), the fuzzer will crash and write the offending input to disk.

### Intensity guided fuzzers

Append ```-intensity_guided=1``` to the fuzzer command. Optionally, also append ```-no_coverage_guided=1``` to discard the code coverage signal, and focus entirely on the intensity signal. Additionally, you may append ```-timeout=1``` to enforce a crash whenever an input takes longer than 1 second to process. The default timeout value is 1200 seconds.

Example:

```
build/out/wolf-ssl-ssh/fuzzer-wolfssl-misc-intensity -intensity_guided=1 build/out/wolf-ssl-ssh/corp-wolfssl-misc/
```

### Allocation guided fuzzers

Append ```-custom_guided=1``` to the fuzzer command. Optionally, also append ```-no_coverage_guided=1``` to discard the code coverage signal, and focus entirely on the allocation signal.

Example:

```
build/out/wolf-ssl-ssh/fuzzer-wolfssl-misc-allocation -custom_guided=1 build/out/wolf-ssl-ssh/corp-wolfssl-misc/
```

### Resource randomization

#### Normal operation

If no arguments are given, then:

- Every request for a memory allocation by wolfSSL/wolfSSH succeeds
- Every request to receive data succeeds, unless the fuzzer is out of data
- Every request to send data succeeds

#### Allocation randomization

When ```--randomize-allocation``` is passed on the command line, then

- Some requests for a memory allocation by wolfSSL/wolfSSH fail
- Every request to receive data succeeds, unless the fuzzer is out of data
- Every request to send data succeeds

#### IO randomization

When ```--randomize-io``` is passed on the command line, then

- Every request for a memory allocation by wolfSSL/wolfSSH succeeds
- Some requests to receive data fail, some requests only receive 1..N for N bytes requested
- Some requests to send data fail, some requests only return 1..N bytes sent for N requested

#### Notes

The corpora for the various possible options:

- normal operation
- allocation randomization
- IO randomization
- allocation randomization + IO randomization

are not interchangeable; each option needs a separate corpus.

## Options

### Memory tests

Undefine ```MEMORY_TESTS``` in ```oss-fuzz/projects/wolf-ssl-ssh/fuzzers/include/fuzzers/shared.h``` for more speed, but fewer memory tests on output data. Please see the comments in that file for more information.

## License

The [oss-fuzz/LICENSE](oss-fuzz/LICENSE) file (Apache 2) applies to all files in the `oss-fuzz/` tree, EXCEPT for the files in the `oss-fuzz/projects/wolf-ssl-ssh/` tree, to which the [GPLv2](LICENSE) license applies.
