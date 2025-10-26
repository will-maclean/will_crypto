#pragma once

#include <stdbool.h>
#include <stdint.h>

void parse_arg_string(int argc, char *argv[], char *argname, char result[256],
                      char *default_, bool fail_hard);

void parse_arg_int32(int argc, char *argv[], char *argname, int32_t *result,
                     int32_t *default_, bool fail_hard);

void parse_arg_uint64(int argc, char *argv[], char *argname, uint64_t *result,
                      uint64_t *default_, bool fail_hard);
