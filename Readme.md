# will\_crypto
_Will Maclean_

My own little crypto library, mainly to get a better understanding of the
different components required in a crypto library. Includes, in various states
of functionality:
- Big integer support (working)
- RNG (ideally, cryptographically secure) (working)
- Prime number generation (working)
- RSA keygen (in progress)
- RSA encryption/decryption, including padding scheme (todo)

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

## Usage
Tests: `bin/tests`

Sample output:
```
----STARTING TESTS----

In one sec, for 32-word numbers, generated 402936 nums
Prime number gen took 2358.586914 milliseconds
Generated prime:

0x0b9b491314cc00362bbd2cd8283e6bc3a4f17e709ab965f4dcef69c4b604e52ad946fae931aa25d0508059b6ffc44e4f3eae05219973c1df89e87e755fb74ec1c51463582abce876266c8c38c694f8ebf05b6ec1d3dab1a100c126db51f777e2a25411429c0c321973d91807592da0edd486c04353a3c68e8edfc23027248d53

----ENDING TESTS----
Tests: 148. Passes: 148. Failures: 0

```
