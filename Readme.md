# will\_crypto
_Will Maclean_

My own little crypto library, mainly to get a better understanding of the
different components required in a crypto library. Includes, in various states
of functionality:
- Big integer support (working)
- RNG (ideally, cryptographically secure) (working)
- Prime number generation (working)
- RSA keygen (working)
- RSA encryption/decryption, including padding scheme (todo)
- CLI (supports RSA keygen)

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

## Usage: `will_crypto`
RSA keygen: `./bin/will_crypto gen_keys`
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


## Testing
Testing is done with CUnit, managed in CTest. This requires CUnit to be installed.

Build tests:
`cmake --build build --target tests`

Run tests:
`ctest --test-dir build -V`

Sample output:
```
1: Suite: BIGINT_Suite
1:   Test: bi_shift_right ...passed
1:   Test: bi_shift_left ...passed
1:   Test: bi_mul ...passed
1:   Test: bi_pow ...passed
1:   Test: bi_mod ...passed
1:   Test: bi_mod_exp ...passed
1:   Test: knuth_d ...passed
1:   Test: bi_gcd ...passed
1:   Test: bi_lcm ...passed
1: Suite: CRYPTO_CORE_Suite
1:   Test: primality ...Prime number gen took 43648.183594 milliseconds
1: Generated prime:
1: 
1: 0x7fb20cb7902acfae4d00541aafff63afc435702d85b6c4e034f67ff489094dda38babc3d7be64ad8abd511b0de5490889e6cbf8d98e328bb39a500f09608daf16ce64173b296d052225dc17c09bbc84fa60460248a1fab6c4d575377f063ab291f23a78c9c650fd9d84f6191d7c68e9c29ab2da697183f729f9402e22a192b23passed
1: Suite: RNG_Suite
1:   Test: rng ...In one sec, for 32-word numbers, generated 399250 nums
1: passed
1:   Test: chacha ...passed
1: Suite: RSA_Suite
1:   Test: rsa ...Generated public and private keys for RSA (512 bit key)
1: n:
1: 0x2ce77453cb02f474b69dc850de3d624f5d8a4576308198de49c7e284d88b1de72c4d5ad3d21bc85a330b19abc186198440cf4bb42c84270f2606eaa1c810d77d
1: e:
1: 0x00010001
1: d:
1: 0x000006a1965f321b
1: passed
1:   Test: ext_euc ...passed
1: 
1: Run Summary:    Type  Total    Ran Passed Failed Inactive
1:               suites      4      4    n/a      0        0
1:                tests     14     14     14      0        0
1:              asserts    154    154    154      0      n/a
1: 
1: Elapsed time =   44.810 seconds
1/1 Test #1: tests ............................   Passed   49.35 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =  49.36 sec
```
