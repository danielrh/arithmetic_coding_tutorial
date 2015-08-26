#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "test_harness.h"
#include "dyn_prob.h"


/////////////////
//// Lesson 4 - Priors
/////////////////
/*
  This lesson illustrates how priors may be used to help describe correlations in a system
  In the sce4narios below, the index of the stream helps to decide what happens.
  Even values must be 0 and odd values may be 1 or 3.
  Given this foreknowledge, we know that the index of the stream is an excellent prior
  That can help the arithmetic coding system encode the bitstream efficiently.

  the EXERCISE is finding priors for a truly random walk.

  You could simply subtract the previous value from the next, and store things in a prior buffer

  To do this exercise well, you may need to adjust CODING_BUFFER_SIZE to 1000000 in test_harness.h
  to allow for a longer stream to for the coder to learn on.
  The prior number in its entirety is usually a good prior since there are only 2 possible outcomes
  for each previous value
  But even each corresponding bit as a prior could be sufficient for fairly good performance
 */


void encode_with_adaptive_probability() {
    DynProb encode[2][256];// one probability per number encoded so far and per index mod 2
    vpx_writer wri ={0};
    vpx_start_encode(&wri, compressed);
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        uint8_t encoded_so_far = 0; // <-- this is the number encoded so far (in reverse order), as a prior for the rest
        for(int bit = 0; bit < 8; ++bit) {
            bool cur_bit = !!(uncompressed[i] & (1 << (7 - bit)));
            vpx_write(&wri, cur_bit, encode[i%2][encoded_so_far + (1 << bit)].prob);
            encode[i%2][encoded_so_far + (1 << bit)].record_bit(cur_bit);
            if (cur_bit) {
                encoded_so_far |= (1 << bit); // <-- this is the number encoded so far, as a prior for the rest
            }
        }
    }
    vpx_stop_encode(&wri);
    printf("Buffer encoded dynamically results in %d size (%.2f%%)\n",
           wri.pos,
           100 * wri.pos / float(sizeof(uncompressed)));
    DynProb decode[2][256];
    vpx_reader rea={0};
    vpx_reader_init(&rea,
                    wri.buffer,
                    wri.pos);
    memset(roundtrip, 0, sizeof(roundtrip));
    for (size_t i = 0; i < sizeof(roundtrip); ++i) {
        uint8_t decoded_so_far = 0; // <-- this is the number decoded so far, as a prior for the rest
        for(int bit = 0; bit < 8; ++bit) {
            if (vpx_read(&rea, decode[i%2][decoded_so_far + (1 << bit)].prob)) {
                roundtrip[i] |= (1 << (7 - bit));
                decode[i%2][decoded_so_far + (1 << bit)].record_bit(true);
                decoded_so_far |= (1 << bit); // <-- this is the number decoded so far, as a prior for the rest
            } else {
                decode[i%2][decoded_so_far + (1 << bit)].record_bit(false);
            }
        }
    }
    assert(vpx_reader_has_error(&rea) == 0);
    assert(memcmp(uncompressed, roundtrip, sizeof(uncompressed)) == 0);
}

int main () {
    printf("Random data\n");
    populate_random_data();
    encode_with_adaptive_probability();
    printf("ASCII data\n");
    populate_ascii_data();
    encode_with_adaptive_probability();
    printf("ZERO data\n");
    populate_zero_data();
    encode_with_adaptive_probability();
    printf("0, 1, or 3\n");
    populate_small_data();
    encode_with_adaptive_probability();
    printf("even = 0, odd is 1, or 3\n");
    populate_even_data();
    encode_with_adaptive_probability();
    printf("even = 0, odd is 1, or 3 followed by even = 0 or 2, odd is 1 or 3\n");
    populate_shifting_even_data();
    encode_with_adaptive_probability();
    printf("multiple of 3,5,7 is 3, even = 0, 1 otherwise\n");
    populate_rel_prime_with_2_3_5_and_7_data();
    encode_with_adaptive_probability();

    printf("----------------\nEXERCISE\n");
    // can you come up with priors that help to define the idea that usually there will
    // be a bitflip towards the end and occasionally there will be carry bits going on
    exercise_random_walk();
    encode_with_adaptive_probability();

    return roundtrip[0];
}
