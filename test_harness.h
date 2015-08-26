enum {
    CODING_BUFFER_SIZE=1024,
    UNCOMPRESSED_BUFFER_SIZE=CODING_BUFFER_SIZE * 16
};
extern unsigned char uncompressed[CODING_BUFFER_SIZE];
extern unsigned char compressed[UNCOMPRESSED_BUFFER_SIZE];
extern unsigned char roundtrip[sizeof(uncompressed)];

// populate the uncompressed data with random data from 0 to 255 per byte
void populate_random_data();
// populate the uncompressed data with random data from 0 to 127 per byte
void populate_ascii_data();
// populate the data with zero
void populate_zero_data();
// populate each byte of the uncompressed data with random assortments of values 0, 1 or 3
void populate_small_data();
// populates each byte of uncompressed data with 0 if even and 1 or 3 if odd
void populate_even_data();
// populates each byte of uncompressed data with 0 if even and 1 or 3 if odd early on and 0 or 2 if odd later and 1 if even
void populate_shifting_even_data();
// populates each byte of uncompressed data with 0 if even and 1 if odd and 3 if prime
void populate_rel_prime_with_2_3_5_and_7_data();



void exercise_mostly_one_data(); // lesson 0 test
void exercise_increasing_data(); // lesson 1 test
void exercise_single_bit_flip(); // lesson 2 test
void exercise_zero_through_11ish_exp_skew(); //lesson 3 test
void exercise_random_walk(); //lesson 4 test
