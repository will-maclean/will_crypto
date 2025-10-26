#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <rng/rng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "argparse.h"

void print_help_and_exit(int exit_code) {
    if (exit_code != 0) {
        printf("Error, exiting.\n");
    }
    char *help_message =
        "\nwill_crypto\n"
        "\nUsage: will_crypto [function]\n"
        "Functions (followed by arguments, optional in square brackets):\n"
        "\n\thelp: prints help and usage message\n"
        "\n\tgen_keys: generates and saves keys. [--output_dir '.'] "
        "[--public_key_filename will_rsa.pub] [--private_key_filename "
        "will_rsa.priv] [--seed number] [--rsa_mode will_rsa_512]\n"
        "\n\tgen_prime: generates and prints a prime [--words 32]\n"
        "\n\tdemo: demo of rsa keygen, encryption, decryption\n"
        "\n\nNote: rsa_mode types are [will_rsa_512, will_rsa_1024, "
        "will_rsa_2048, will_rsa_4096]\n"
        "\nExamples:\n"
        "\nGenerate a 2048 bit prime. 2048 / 32 = 64, so 64 words.\n"
        "will_crypto gen_prime 64\n"
        "\nGenerate RSA keys, with all default parameters\n"
        "will_crypto gen_keys\n"
        "\nGenerate RSA keys, setting paths, rsa mode, and random seed\n"
        "will_crypto gen_keys --output_dir my/dir --public_key_filename "
        "my_key.pub --rsa_mode will_rsa_2048 --seed 1234\n";

    printf("%s", help_message);
    exit(exit_code);
}

typedef enum program_function_t {
    gen_keys,
    gen_prime_fn,
    demo,
} program_function_t;

typedef struct {
    char public_key_filename[256];
    char private_key_filename[256];
    char output_dir[256];

    uint64_t seed;
    rsa_mode_t rsa_mode;
} gen_keys_args_t;

typedef struct {
    uint32_t words;
    uint64_t seed;
} gen_prime_args_t;

typedef struct {
    program_function_t function;

    union {
        gen_keys_args_t gen_keys_args;
        gen_prime_args_t gen_prime_args;
    } args;
} program_inputs_t;

program_inputs_t parse_args(int argc, char *argv[]) {
    if (argc < 2) {
        print_help_and_exit(1);
    }

    program_inputs_t res;

    char *fn_name = argv[1];
    if (!strcmp(fn_name, "gen_keys")) {
        // parse the gen_key args
        res.function = gen_keys;

        parse_arg_string(argc - 2, argv + 2, "output_dir",
                         res.args.gen_keys_args.output_dir, ".", true);
        parse_arg_string(argc - 2, argv + 2, "public_key_filename",
                         res.args.gen_keys_args.public_key_filename,
                         "will_rsa.pub", true);
        parse_arg_string(argc - 2, argv + 2, "private_key_filename",
                         res.args.gen_keys_args.private_key_filename,
                         "will_rsa.priv", true);
        uint64_t default_seed = time(NULL);
        parse_arg_uint64(argc - 2, argv + 2, "seed",
                         &res.args.gen_keys_args.seed, &default_seed, true);
        char rsa_mode_name[256];
        parse_arg_string(argc - 2, argv + 2, "rsa_mode", rsa_mode_name,
                         "will_rsa_512", true);
        if (rsa_mode_from_str(rsa_mode_name,
                              &res.args.gen_keys_args.rsa_mode)) {
            printf("Failed parsing RSA mode from %s\n", rsa_mode_name);
            exit(1);
        }

    } else if (!strcmp(fn_name, "gen_prime")) {
        res.function = gen_prime_fn;

        uint32_t default_words = 32;
        parse_arg_uint32(argc - 2, argv + 2, "words",
                         &res.args.gen_prime_args.words, &default_words, true);
        uint64_t default_seed = time(NULL);
        parse_arg_uint64(argc - 2, argv + 2, "seed",
                         &res.args.gen_prime_args.seed, &default_seed, true);

        printf("genning a prime with %d words\n\n",
               res.args.gen_prime_args.words);

    } else if (!strcmp(fn_name, "demo")) {
        res.function = demo;

    } else if (!strcmp(fn_name, "help")) {
        print_help_and_exit(0);
        exit(0);
    } else {
        printf("Unrecognised function: %s\n", fn_name);
        print_help_and_exit(1);
        exit(1);
    }

    return res;
}

void join_dir_and_filename(char *dir, char *file, char *res) {
    // assumes unix file joins i.e. "/"
    strcpy(res, dir);
    strcat(res, "/");
    strcat(res, file);
}

void cmdline_gen_keys(gen_keys_args_t args) {
    rsa_public_token_t pub;
    rsa_private_token_t priv;

    printf("Generating keys for will_rsa\n");

    will_rng_init(args.seed);

    gen_pub_priv_keys(args.seed, &pub, &priv, args.rsa_mode);

    char full_public_filepath[256];
    char full_private_filepath[256];
    join_dir_and_filename(args.output_dir, args.public_key_filename,
                          full_public_filepath);
    join_dir_and_filename(args.output_dir, args.private_key_filename,
                          full_private_filepath);

    pub_key_to_file(&pub, full_public_filepath, args.rsa_mode, true);
    priv_key_to_file(&priv, full_private_filepath, args.rsa_mode, true);

    printf("Saved public key to %s\n", full_public_filepath);
    printf("Saved private key to %s\n", full_private_filepath);

    // cleanup
    bi_free(pub.e);
    bi_free(pub.n);
    bi_free(priv.n);
    bi_free(priv.d);
}

void run_demo(void) {
    printf("Running RSA keygen, encryption, decryption demo\n\n");

    rsa_public_token_t pub;
    rsa_private_token_t priv;
    gen_pub_priv_keys(1234, &pub, &priv, RSA_MODE_512);

    printf("----------\nGenerated public key, where \n\tn=");
    bi_print(pub.n);
    printf("\n\n\te=");
    bi_print(pub.e);
    printf("\n----------\n\n----------\nGenerated private key, where\n\td=");
    bi_print(priv.d);

    MPI msg = bi_init(1);
    bi_set(msg, 0x12345678);

    MPI c = will_rsa_encrypt_num(msg, &pub);
    MPI msg_ = will_rsa_decrypt_num(c, &priv);

    printf("\n----------\n\nEncrypting message=");
    bi_print(msg);
    printf("\n\nEncrypted cyphertext=");
    bi_print(c);
    printf("\n\nDecrypted message=");
    bi_print(msg_);
    printf("\n");

    bi_free(pub.n);
    bi_free(priv.n);
    bi_free(pub.e);
    bi_free(priv.d);
    bi_free(msg);
    bi_free(c);
    bi_free(msg_);
}

int main(int argc, char *argv[]) {
    program_inputs_t args = parse_args(argc, argv);

    MPI prime;
    switch (args.function) {
    case gen_keys:
        cmdline_gen_keys(args.args.gen_keys_args);
        break;
    case gen_prime_fn:
        will_rng_init(args.args.gen_prime_args.seed);
        prime = gen_prime(args.args.gen_prime_args.words);
        bi_print(prime);
        bi_free(prime);
        break;
    case demo:
        run_demo();
    }

    return 0;
}
