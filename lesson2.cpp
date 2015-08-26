#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "test_harness.h"
#include "dyn_prob.h"


/////////////////
//// Lesson 2 - Basic model
/////////////////
/*
  This lesson illustrates that the probabilities being adaptively tracked need not be limited to
  a single value over the whole stream.
  For instance in this model we track a probability per bit in each byte.
  This set of 8 dynamically learned probabilities is known as a "model".
  In this case the model works well for ascii, which has the 7th bit as zero always


  The EXERCISE asks you to see if you can compress a datastream consisting of single bit flips
  There are a number of ways to approach this.
  One interesting way is to divide the observations of the probabilities repeatedly so that
  The encoder quickly 'learns' that a particular bit is in the 'one' state or the 'zero' state

  Another approach is to use the transformation functions to subtract the previous value from the
  Current value
 */

void nop(unsigned char*){}
void (*transform)(unsigned char* tmp) = nop;
void (*untransform)(unsigned char* tmp) = nop;

unsigned char tmp[sizeof(uncompressed)];
void encode_with_adaptive_probability() {
    memcpy(tmp, uncompressed, sizeof(uncompressed));
    (*transform)(tmp);

    DynProb encode[8];// one probability per bit position <-- new for lesson2
    vpx_writer wri ={0};
    vpx_start_encode(&wri, compressed);
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        int bit_index = 0;
        for(int bit = 1; bit < 256; ++bit_index, bit <<= 1) {
            bool cur_bit = !!(uncompressed[i] & bit);
            vpx_write(&wri, cur_bit, encode[bit_index].prob); // <-- bit_index is new for lesson2
            encode[bit_index].record_bit(cur_bit); // <-- bit_index is new for lesson2
        }
    }
    vpx_stop_encode(&wri);
    printf("Buffer encoded with final prob(0) = {%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f} results in %d size (%.2f%%)\n",
           encode[0].prob / 255.,
           encode[1].prob / 255.,
           encode[2].prob / 255.,
           encode[3].prob / 255.,
           encode[4].prob / 255.,
           encode[5].prob / 255.,
           encode[6].prob / 255.,
           encode[7].prob / 255.,
           wri.pos,
           100 * wri.pos / float(sizeof(uncompressed)));
    DynProb decode[8];
    vpx_reader rea={0};
    vpx_reader_init(&rea,
                    wri.buffer,
                    wri.pos);
    memset(roundtrip, 0, sizeof(roundtrip));
    for (size_t i = 0; i < sizeof(roundtrip); ++i) {
        int bit_index = 0;
        for(int bit = 1; bit < 256; ++bit_index, bit <<= 1) {
            if (vpx_read(&rea, decode[bit_index].prob)) { // <-- bit_index is new for lesson2
                roundtrip[i] |= bit;
                decode[bit_index].record_bit(true); // <-- bit_index is new for lesson2
            } else {
                decode[bit_index].record_bit(false); // <-- bit_index is new for lesson2
            }
        }
    }
    assert(vpx_reader_has_error(&rea) == 0);
    (*untransform)(uncompressed);
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
    printf("------------------\nEXERCISE\n");
    exercise_single_bit_flip();
    encode_with_adaptive_probability();
    return roundtrip[0];
}
