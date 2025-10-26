// Expected argument forms:
// --long_name value

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool starts_with(char *src, char *match) {
    if (strlen(src) < strlen(match))
        return false;

    for (int i = 0; i < strlen(match); i++) {
        if (src[i] != match[i])
            return false;
    }

    return true;
}

void parse_arg_string(int argc, char *argv[], char *argname, char result[256],
                      char *default_, bool fail_hard) {
    for (int i = 0; i < argc - 1; i++) {
        if (starts_with(argv[i], "--")) {
            // extract the arg name
            char parsed_arg_name[64];
            strcpy(parsed_arg_name, argv[i] + 2);

            if (!strcmp(parsed_arg_name, argname)) {
                // the args are the same!
                strcpy(result, argv[i + 1]);
                return;
            }
        }
    }

    if (default_)
        strcpy(result, default_);
    else if (fail_hard) {
        printf("Required argument %s was not set or not able to be parsed",
               argname);
    }
}

void parse_arg_int32(int argc, char *argv[], char *argname, int32_t *result,
                     int32_t *default_, bool fail_hard) {
    for (int i = 0; i < argc - 1; i++) {
        if (starts_with(argv[i], "--")) {
            // extract the arg name
            char parsed_arg_name[64];
            strcpy(parsed_arg_name, argv[i] + 2);

            if (!strcmp(parsed_arg_name, argname)) {
                char *res;
                // TODO: check res for a successfuly parse check
                *result = strtol(argv[i + 1], &res, 10);
                return;
            }
        }
    }

    if (default_)
        *result = *default_;
    else if (fail_hard) {
        printf("Required argument %s was not set or not able to be parsed",
               argname);
    }
}

void parse_arg_uint32(int argc, char *argv[], char *argname, uint32_t *result,
                      uint32_t *default_, bool fail_hard) {
    for (int i = 0; i < argc - 1; i++) {
        if (starts_with(argv[i], "--")) {
            // extract the arg name
            char parsed_arg_name[64];
            strcpy(parsed_arg_name, argv[i] + 2);

            if (!strcmp(parsed_arg_name, argname)) {
                char *res;
                // TODO: check res for a successfuly parse check
                *result = (uint32_t)strtol(argv[i + 1], &res, 10);
                return;
            }
        }
    }

    if (default_)
        *result = *default_;
    else if (fail_hard) {
        printf("Required argument %s was not set or not able to be parsed",
               argname);
    }
}

void parse_arg_uint64(int argc, char *argv[], char *argname, uint64_t *result,
                      int64_t *default_, bool fail_hard) {
    for (int i = 0; i < argc - 1; i++) {
        if (starts_with(argv[i], "--")) {
            // extract the arg name
            char parsed_arg_name[64];
            strcpy(parsed_arg_name, argv[i] + 2);

            if (!strcmp(parsed_arg_name, argname)) {
                char *res;
                // TODO: check res for a successfuly parse check
                *result = strtoll(argv[i + 1], &res, 10);
                return;
            }
        }
    }

    if (default_)
        *result = *default_;
    else if (fail_hard) {
        printf("Required argument %s was not set or not able to be parsed",
               argname);
    }
}
