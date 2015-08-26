#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "test_harness.h"

/////////////////
//// Lesson 1 - Dynamic Probabilities
/////////////////
/*
  This lesson shows the important detail that is required to compress data using arithmetic coding
  Enumarting the probabilities in code is rather cumbersome,
  but probabilities can be learned *as* the file is observed.

  Training your arithmetic coder on the file at hand can help keep the description
  of the compression format crisp and simple, and it can make the system work on a variety of data.

  This encoder simply keeps a weighted average of the number of zeros seen in the stream.

  It works well for very simple data, but leaves something to be desired for more complex data

  However some data can be transformed to be closer to either all-ones or all-zeros
  The EXERCISE is as follows: can you make a reversible transformation that helps
  The test data be closer to either all 1's or all 0's so that this very simple compression
  system can better compress the exercise_increasing_data()?

 */


struct DynProb {
    uint8_t prob;
    int true_count;
    int false_count;
    DynProb() {
        prob = 127;
        true_count = 0;
        false_count = 0;
    }
    void record_bit(bool bit) { // this class keeps track of what has been recorded so far
        if (bit) {
            ++true_count; // if we just saw a true, update
        } else {
            ++false_count; // if we just saw a false, update
        }
        prob = 256 * (false_count + 1) / (true_count + 2 + false_count);
        // estimate the probability based on the false count / false count + true count
        // in a ~bayesian manner
    }
};
void nop(unsigned char*){}
void (*transform)(unsigned char* tmp) = nop;
void (*untransform)(unsigned char* tmp) = nop;

unsigned char tmp[sizeof(uncompressed)];
void encode_with_adaptive_probability() {
    memcpy(tmp, uncompressed, sizeof(uncompressed));
    (*transform)(tmp); // this currently is a no-op but it may be helpful for the EXERCISE
    DynProb encode;
    vpx_writer wri ={0};
    vpx_start_encode(&wri, tmp);
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        for(int bit = 1; bit < 256; bit <<= 1) {
            bool cur_bit = !!(tmp[i] & bit);
            vpx_write(&wri, cur_bit, encode.prob);
            encode.record_bit(cur_bit); // <-- this a new line for lesson1 that lets the encoder adapt to data
        }
    }
    vpx_stop_encode(&wri);
    printf("Buffer encoded with final prob(0) = %.2f results in %d size (%.2f%%)\n",
           encode.prob / 255.,
           wri.pos,
           100 * wri.pos / float(sizeof(uncompressed)));
    DynProb decode;
    vpx_reader rea={0};
    vpx_reader_init(&rea,
                    wri.buffer,
                    wri.pos);
    memset(roundtrip, 0, sizeof(roundtrip));
    for (size_t i = 0; i < sizeof(roundtrip); ++i) {
        for(int bit = 1; bit < 256; bit <<= 1) {
            if (vpx_read(&rea, decode.prob)) {
                roundtrip[i] |= bit;
                decode.record_bit(true); // <-- this a new line for lesson1
            } else {
                decode.record_bit(false); // <-- this a new line for lesson1
            }
        }
    }
    assert(vpx_reader_has_error(&rea) == 0);
    (*untransform)(uncompressed); // this is, again a no-op, but may be helpful for the EXERCISE
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

    printf("--------------\nEXERCISE\n");
    exercise_increasing_data();
    //modify transform and untransform to point to functions that modify the data
    // in a favorable way so that an increasing array maps to values nearly zero or nearly 1
    encode_with_adaptive_probability();

    return roundtrip[0];
}
