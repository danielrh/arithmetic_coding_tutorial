#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "test_harness.h"



//////////////////
////  Lesson 0 - Basic Arithmetic Coding
//////////////////
/*
  This lesson shows how an arithmetic coder works:
  An arithmetic coder simply lets you skew how much disk space you spend for a 0 or a 1 bit
  Respectively.  If you are sure that the next bit will be zero, you want to set the probability
  Of a zero very high, so that the weight will favor zeros over ones.

  Note that the algorithm will always produce correct results:
  The ZERO data with incorrect probability example shows the calamity that happens
  When the wrong probability is guessed for a stream. The result is simpy 10x bigger
  The data is still recoverable, just extra disk space was wasted

  Likewise you can see when the probability is guessed exactly, the compression factor is 100:1 here

  The EXERCISE is as follows: given an array filled with mostly 1 bits, what should the probability
  of a zero be passed in as.  The current value, 255, is not beneficial and results in a 6.5x
  bloat to the original file

 */

void encode_with_fixed_probability(uint8_t prob){
    vpx_writer wri ={0};
    vpx_start_encode(&wri, compressed); //setup the arithmetic coder
    for (size_t i = 0; i < sizeof(uncompressed); ++i) { // go through each byte in the buffer
        for(int bit = 1; bit < 256; bit <<= 1) { // for each bit in each byte
            vpx_write(&wri, !!(uncompressed[i] & bit), prob); // serialize out the bit
                                                              // use the specified prob
                                                              // to skew the cost of 1 or 0
        }
    }
    vpx_stop_encode(&wri);
    printf("Buffer encoded with prob(0) = %.2f results in %d size (%.2f%%)\n",
           prob / 255.,
           wri.pos,
           100 * wri.pos / float(sizeof(uncompressed)));
    vpx_reader rea={0};
    vpx_reader_init(&rea,
                    wri.buffer,
                    wri.pos); // setup the decoder
    memset(roundtrip, 0, sizeof(roundtrip)); // zero out the roundtrip array

    for (size_t i = 0; i < sizeof(roundtrip); ++i) { // go through each byte in the output
        for(int bit = 1; bit < 256; bit <<= 1) { // for each bit in the output
            if (vpx_read(&rea, prob)) { // read the bit in the input, weighted by prob
                roundtrip[i] |= bit;    // if it's true, set the bit in the output
            }
        }
    }
    assert(vpx_reader_has_error(&rea) == 0); //make sure the reader has no error
    assert(memcmp(uncompressed, roundtrip, sizeof(uncompressed)) == 0); // make sure input = output
}

int main () {
    printf("Random data\n");
    populate_random_data();
    encode_with_fixed_probability(127);
    printf("ASCII data\n");
    populate_ascii_data();
    encode_with_fixed_probability(145);
    printf("ZERO data\n");
    populate_zero_data();
    encode_with_fixed_probability(255);
    printf("ZERO data with incorrect probability\n");
    populate_zero_data();
    encode_with_fixed_probability(0);

    printf("----------------\nEXERCISE\n");
    unsigned int prob = 255; // <-- FILL ME IN WITH A BETTER VALUE
    exercise_mostly_one_data();
    encode_with_fixed_probability(prob);

    return roundtrip[0];
}
