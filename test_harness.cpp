#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_harness.h"
unsigned char uncompressed[CODING_BUFFER_SIZE];
unsigned char compressed[UNCOMPRESSED_BUFFER_SIZE];
unsigned char roundtrip[sizeof(uncompressed)];

void populate_random_data() {
    FILE * fp = fopen("/dev/urandom", "r");
    fread(uncompressed, 1, sizeof(uncompressed), fp);
    fclose(fp);
}
void populate_ascii_data() {
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        uncompressed[i] &= 127;
    }
}

void populate_zero_data() {
    memset(uncompressed, 0, sizeof(uncompressed));
}

void populate_small_data() {
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        uncompressed[i] %= 3;
        if (uncompressed[i] == 2) { // 0 1  or 3 allowed
            ++uncompressed[i];
        }
    }
}

void populate_even_data() {
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (i & 1) {
            uncompressed[i] &= 1;
            uncompressed[i] *= 2;
            uncompressed[i] += 1;
        } else {
            uncompressed[i] = 0;
        }
    }
}

void populate_shifting_even_data() {
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (i < sizeof(uncompressed) / 2) {
            if (i & 1) {
                uncompressed[i] &= 1;
                uncompressed[i] *= 2;
                uncompressed[i] += 1;
            } else {
                uncompressed[i] = 0;
            }
        } else {
            if (i & 1) {
                uncompressed[i] &= 1;
                uncompressed[i] *= 2;
            } else {
                uncompressed[i] &= 1;
                uncompressed[i] *= 2;
                uncompressed[i] += 1;
            }
        }
    }
}

void populate_rel_prime_with_2_3_5_and_7_data() {
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (i & 1) {
            uncompressed[i] = 3;
            if (i % 3 == 0 || i % 5 == 0 || i % 7 == 0) {
                uncompressed[i] = 1;
            }
        } else {
            uncompressed[i] = 0;
        }
    }
}


bool is_prime(size_t val) {
    if (val <= 2) return true;
    if (val & 1) return false;
    for (size_t i = 0; i < val / 2; ++i) { // should sqrt, but oh well
        for (size_t j = 0; j < val / 2; ++j) {
            if (i * j == val) {
                return false;
            }
        }
    }
    return true;
}

void populate_prime_data() {
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (i & 1) {
            uncompressed[i] = 1;
            if (is_prime(i)) {
                uncompressed[i] = 3;
            }
        } else {
            uncompressed[i] = 0;
        }
    }
}


void exercise_mostly_one_data() { // lesson 0 test
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (!is_prime(uncompressed[i])) {
            uncompressed[i] = 0xff;
        }
    }
}
void exercise_increasing_data() { // lesson 1 test
    populate_random_data();
    bool started = 0;
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (started) {
            uncompressed[i] = uncompressed[i-1] + 1;
        } else if (uncompressed[i] < 0x10) {
            started = true;
        }
    }
}
void exercise_single_bit_flip() { // lesson 2 test
    populate_random_data();
    for (size_t i = 1; i < sizeof(uncompressed); ++i) {
        if (uncompressed[i] / 8 == 0) { // flip fairly rarely
            unsigned int which_bit = uncompressed[i] & 7;
            uncompressed[i] = uncompressed[i-1] ^ (1 << which_bit);
        }
    }
}
void exercise_zero_through_11ish_exp_skew() { //lesson 3 test
    populate_random_data();
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        if (uncompressed[i] % 13 != 0) {
            if (uncompressed[i] < 40) {
                uncompressed[i] = 0;
            } else if (uncompressed[i] < 64) {
                uncompressed[i] = 1;
            } else if (uncompressed[i] < 96) {
                uncompressed[i] = 2;
            } else if (uncompressed[i] < 128) {
                uncompressed[i] = 3;
            } else if (uncompressed[i] < 160) {
                uncompressed[i] = 4;
            } else if (uncompressed[i] < 180) {
                uncompressed[i] = 5;
            } else if (uncompressed[i] < 190) {
                uncompressed[i] = 6;
            } else {
                uncompressed[i] %= 11;
            }

        }
    }
}
void exercise_random_walk() { //lesson 4 test
    populate_random_data();
    for (size_t i = 1; i < sizeof(uncompressed); ++i) {
        if (i % 11 != 0) {
            uncompressed[i] = uncompressed[i-1] + 2 * (uncompressed[i] & 1) - 1;
        }
    }
}
