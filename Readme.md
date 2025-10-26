# will\_crypto
_Will Maclean_

My own little crypto library, mainly to get a better understanding of the
different components required in a crypto library. Includes:
- Big integer support (unsigned and signed)
- RNG (ChaCha 16)
- Prime number generation (Probabilistic Miller-Rabin with optimisations)
- RSA keygen (512, 1024, 2048, 4096 bit keys)
- RSA encryption/decryption (no padding)
- CLI

Note - this is a hobby project, and as such will probably contain bugs. Please
don't use this in production systems!

## Installation
```bash
git clone https://github.com/will-maclean/will_crypto.git
cd will_crypto
mkdir build
cd build
cmake ..
make
# binaries are now available in will_crypto/bin
```

## Usage: `will_crypto demo`
Shows a demonstration of RSA keygen, encryption, and decryption.

Run: `./bin/will_crypto demo`
Sample output:
```
Running RSA keygen, encryption, decryption demo

----------
Generated public key, where 
        n=0x6371a916b8ab9209a23c086e762561ecb8c531530016ade3a0b1f75ab92b0923be077ab992ad33e645f3cac11b7b70a30fd61e1d86f55948f0d1943b9b2c8c01

        e=0x00010001
----------

----------
Generated private key, where
        d=0x02dddfaf3b3f1cec60ac24dd1454530c992dfc1445b421aaa67302ebaf5b557dd4337afa7222657851af82298220d99ddd5b57639bea607e44684120303c4cdd
----------

Encrypting message=0x12345678

Encrypted cyphertext=0x4f2769275f60825801b0989f5de4ba022216223489bf27634a8bdf2a6075df976adf1d1db92eb4be42bcee17aa10a070ae00271e047159102bbd6b51569d3a54

Decrypted message=0x12345678
```

## Usage: `will_crypto gen_keys`
Generates RSA keys and saves to file.

Run: `./bin/will_crypto gen_keys [--output_dir '.'] [--public_key_filename will_rsa.pub] [--private_key_filename will_rsa.priv] [--seed number] [--rsa_mode will_rsa_512]`

Note: rsa_mode types are [will_rsa_512, will_rsa_1024, will_rsa_2048, will_rsa_4096].

Note: if `seed` is not set, default is to use system random seed.

Sample output:
```
Generating keys for will_rsa
Saved public key to ./will_rsa.pub
Saved private key to ./will_rsa.priv
```
And file contents like:
```
# will_rsa.pub
3bf179584bddddaa276e4ec354eb5c423daa330d635f77dadef3d236c4970bc2178f3ac59dfcfa9e42036d17d28e2bd3ca761c3961f24b79a418db3948c0e2721b55593ec72362c31a24773069f43eaee8e1b429f11bee319c1bf556296654110850bcc454aa0ff43fefa14dcdfa7f7737b0afca2b96c9eae2ec6e55a4705e6d
00010001
will_rsa(public)
```
```
# will_rsa.priv
3bf179584bddddaa276e4ec354eb5c423daa330d635f77dadef3d236c4970bc2178f3ac59dfcfa9e42036d17d28e2bd3ca761c3961f24b79a418db3948c0e2721b55593ec72362c31a24773069f43eaee8e1b429f11bee319c1bf556296654110850bcc454aa0ff43fefa14dcdfa7f7737b0afca2b96c9eae2ec6e55a4705e6d
00000dc5ef5a1023
will_rsa(private)
```

## Usage: `will_crypto gen_prime`
Generates and prints a prime.

Run: `./bin/will_crypto gen_prime [--words 32] [--seed number]`

Note: if `seed` is not set, default is to use system random seed.

Sample output:
```
0x1a688433470372be645f517505de63ab6acf56a597cd22d97424afe85770966539995b70baf88311fc2fcfc7dbdb8b8c89afc3378835c7b1db8e805b5ca6a592d9a7a86ab5867f62e69036d414b1d0587d359b1387663bcc79e97f63ae1cdd02c48a9a41f8c227eb6b5eb93c71f84ae242edb0a82bb1b54e5accdefcfe1c20d3
```

## Testing
Testing is done with CUnit, managed in CTest. This requires CUnit to be installed.

Build tests:
`cmake --build build --target tests`

Run tests:
`ctest --test-dir build -V`

Sample output:
```
1: Suite: BIGINT_Suite
1:   Test: bi_add ...passed
1:   Test: bi_sub ...passed
1:   Test: bi_shift_right ...passed
1:   Test: bi_shift_left ...passed
1:   Test: bi_mul ...passed
1:   Test: bi_pow ...passed
1:   Test: bi_mod ...passed
1:   Test: bi_mod_exp ...passed
1:   Test: bi_gcd ...passed
1:   Test: bi_lcm ...passed
1:   Test: ext_euc ...passed
1:   Test: test_bi_mul_inv_mod ...passed
1: Suite: BIGINT_signed_suite
1:   Test: test_signed_add ...passed
1:   Test: test_signed_sub ...passed
1:   Test: signed_eucl_div ...passed
1: Suite: CRYPTO_CORE_Suite
1:   Test: primality ...passed
1: Suite: RNG_Suite
1:   Test: rng ...In one sec, for 32-word numbers, generated 650465 nums
1: passed
1:   Test: chacha ...passed
1: Suite: RSA_Suite
1:   Test: rsa_keygen ...Generated public and private keys for RSA (512 bit key)
1: n:
1: 0x6371a916b8ab9209a23c086e762561ecb8c531530016ade3a0b1f75ab92b0923be077ab992ad33e645f3cac11b7b70a30fd61e1d86f55948f0d1943b9b2c8c01
1: e:
1: 0x00010001
1: d:
1: 0x02dddfaf3b3f1cec60ac24dd1454530c992dfc1445b421aaa67302ebaf5b557dd4337afa7222657851af82298220d99ddd5b57639bea607e44684120303c4cdd
1: passed
1:   Test: calc_lambda_n_d ...passed
1:   Test: rsa_end_to_end ...passed
1:   Test: test_rsa_encrypt_decrypt_cases ...passed
1: 
1: Run Summary:    Type  Total    Ran Passed Failed Inactive
1:               suites      5      5    n/a      0        0
1:                tests     22     22     22      0        0
1:              asserts    321    321    321      0      n/a
1: 
1: Elapsed time =    3.650 seconds
1/1 Test #1: tests ............................   Passed    4.80 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =  4.80 sec
```

## Benchmarking
Build benchmarking target:
`cmake --build build --target benchmarking`

Run benchmarking:
`./bin/benchmarking`

Current benchmarking summary results are:
```
----BENCHMARK RESULTS----

        Function: bi_add
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.000550

        Function: bi_add
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.000510

        Function: bi_add
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 0.000570

        Function: bi_add
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 0.000560

        Function: bi_add
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.000690

        Function: bi_add
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.000840

        Function: bi_add
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.001160

        Function: bi_sub
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.000540

        Function: bi_sub
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.000550

        Function: bi_sub
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 0.000600

        Function: bi_sub
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 0.000510

        Function: bi_sub
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.000520

        Function: bi_sub
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.000490

        Function: bi_sub
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.000680

        Function: bi_mul
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.000500

        Function: bi_mul
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.000500

        Function: bi_mul
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 0.000520

        Function: bi_mul
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 0.000580

        Function: bi_mul
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.001640

        Function: bi_mul
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.004920

        Function: bi_mul
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.018890

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.000520

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.000650

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 0.000770

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 0.000680

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.000700

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.000770

        Function: bi_eucl_div
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.000890

        Function: bi_mod
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.000550

        Function: bi_mod
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.000680

        Function: bi_mod
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 0.000810

        Function: bi_mod
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 0.000590

        Function: bi_mod
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.000780

        Function: bi_mod
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.000890

        Function: bi_mod
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.000990

        Function: rng
        N trials: 100
        N words for inputs: 32
        Avg exe ms: 0.000690

        Function: rng
        N trials: 100
        N words for inputs: 64
        Avg exe ms: 0.000900

        Function: rng
        N trials: 100
        N words for inputs: 128
        Avg exe ms: 0.001320

        Function: gen_prime
        N trials: 100
        N words for inputs: 1
        Avg exe ms: 0.198150

        Function: gen_prime
        N trials: 100
        N words for inputs: 2
        Avg exe ms: 0.793440

        Function: gen_prime
        N trials: 100
        N words for inputs: 4
        Avg exe ms: 1.858940

        Function: gen_prime
        N trials: 100
        N words for inputs: 8
        Avg exe ms: 8.877950

        Function: gen_prime
        N trials: 10
        N words for inputs: 16
        Avg exe ms: 59.938599

        Function: gen_prime
        N trials: 10
        N words for inputs: 32
        Avg exe ms: 785.991089

        Function: gen_prime
        N trials: 10
        N words for inputs: 64
        Avg exe ms: 3150.173828

        Function: gen_prime
        N trials: 1
        N words for inputs: 128
        Avg exe ms: 74548.179688


----END BENCHMARK RESULTS----
```
