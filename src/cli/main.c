#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_help_and_exit(int exit_code) {
    if (exit_code != 0) {
        printf("Error, exiting.\n");
    }
    char *help_message = "\n\nwill_crypto\n"
                         "Usage: will_crypto [function]\n"
                         "Functions:\n"
                         "\thelp: prints help and usage message\n"
                         "\tgen_keys: generates and saves keys\n"
                         "\tgen_prime: generates and prints a prime"
                         "\tdemo: demo of rsa keygen, encryption, decryption";

    printf("%s", help_message);
    exit(exit_code);
}

typedef enum program_function_t {
    gen_keys,
    gen_prime_fn,
    demo,
} program_function_t;

typedef struct {
    char public_key_filename[256];  // set to NULL for will_rsa.pub
    char private_key_filename[256]; // set to NULL for will_rsa.priv
    char output_dir[256];           // set to NULL for curr dir

    uint64_t seed;       // default if not provided is random from system
    rsa_mode_t rsa_mode; // default is RSA_1024
} gen_keys_args_t;

typedef struct {
    uint32_t words;

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

        // load args with default values
        strcpy(res.args.gen_keys_args.output_dir, ".");
        strcpy(res.args.gen_keys_args.private_key_filename, "will_rsa.priv");
        strcpy(res.args.gen_keys_args.public_key_filename, "will_rsa.pub");
        res.args.gen_keys_args.seed = time(NULL);
        res.args.gen_keys_args.rsa_mode = RSA_MODE_1024;
    } else if (!strcmp(fn_name, "gen_prime")) {
        res.function = gen_prime_fn;

        res.args.gen_prime_args.words = 32;

    }else if (!strcmp(fn_name, "demo")) {
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

void run_demo(){
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
        prime = gen_prime(args.args.gen_prime_args.words);
        bi_print(prime);
        bi_free(prime);
        break;
    case demo:
        run_demo();
    }

    return 0;
}
