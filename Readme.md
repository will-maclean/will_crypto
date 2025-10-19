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


## Usage: `tests`
Tests: `./bin/tests`

Sample output:
```
----STARTING TESTS----

In one sec, for 32-word numbers, generated 47766 nums
Prime number gen took 6565.018066 milliseconds
Generated prime:

0x0b9b491314cc00362bbd2cd8283e6bc3a4f17e709ab965f4dcef69c4b604e52ad946fae931aa25d0508059b6ffc44e4f3eae05219973c1df89e87e755fb74ec1c51463582abce876266c8c38c694f8ebf05b6ec1d3dab1a100
c126db51f777e2a25411429c0c321973d91807592da0edd486c04353a3c68e8edfc23027248d53

----ENDING TESTS----
Tests: 187. Passes: 187. Failures: 0

----Starting RSA test-----
Generated public and private keys for RSA (512 bit key)
n:
0x2ce77453cb02f474b69dc850de3d624f5d8a4576308198de49c7e284d88b1de72c4d5ad3d21bc85a330b19abc186198440cf4bb42c84270f2606eaa1c810d77d
e:
0x00010001
d:
0x000006a1965f321b

----Finishing RSA test-----
```
