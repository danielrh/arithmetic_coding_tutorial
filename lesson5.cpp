#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "test_harness.h"
#include "dyn_prob.h"

/////////////////
//// Lesson 5 - More Priors and adaptive probabilities
/////////////////
/*
 This lesson illustrates that multiple dimensions of priors can be used.
 In this case the system is producing certain values if the buffer is 0 % 2, 3, 5, or 7

 One important thing to note is that if your CODING_BUFFER_SIZE is still 1024 (the default)

 Adding the prior of index % 7 == 0 actually *hurts* performance here.
 That's because the number of probabilities needed to be trained runs out entirely before
 The stream has gotten very far. So the system is actually better off having fewer priors for short
 Streams.  This is a counterintuitive result of having the probabilities being trained as the
 stream is seen for the first time

 Lastly another way to improve an arithmetic coder is to rescale your probabilities every so often
 So that if the data stream gets into a "new mode" that the encoder has a chance to learn that
 mode without being overbiased by old probabilities.
 We do this rescaling by simply dividing all seen values by 2.


  The EXERCISE here is to add the index % 7 == 0 prior to the 3 dimensional prior
  And observe how that affects the probability of the rel_prime_with_2_3_5_and_7
  Notice for short streams it gets worse, but if you modify CODING_BUFFER_SIZE in test_harness.h
  And make the number longer, like a megabyte, then the index % 7 prior begins to add usefulness
  And significantly reduces the file size.  This shows that additional priors can be a double edged
  sword and must be used with care.  Sometimes adding 2 priors can be a way to add data
  without adding too much data to the size of the model.

  The final EXERCISE here is to being looking at
  https://github.com/danielrh/losslessh264/blob/master/codec/decoder/core/inc/macroblock_model.h
  And starting to understand the kinds of probability models the lossles H.264 video coding system
  uses. To guess what the next bits in the video stream are going to be, based on a number of
  properties of the decoder state.  In these lessons we usually used index or the previous value
  lossless video compression uses other data like previous frames' data, encoder state,
  vertical neighbors, etc.

 */

enum {
    RESCALE_CONSTANT = 32 // <-- normally a good value for this is 512 or > 2x the max prob
                          //     except for really really short streams like the default
};
void DynProb::record_bit_and_rescale(bool bit) {
    record_bit(bit);
    if (true_count > RESCALE_CONSTANT || false_count > RESCALE_CONSTANT) {
        true_count /= 2; // try not to overtrain by rescaling our true and false counts
        false_count /= 2;// so the system can learn on future, different, data more quickly
    }
}


void encode_with_adaptive_probability() {
    DynProb encode[2][2][2][256];
    // one probability per number encoded so far and per index mod 2
    // also include index % 3== 0 and index % 5 == 0
    // EXERCISE: add another dimension as index % 7
    // Note: If we also include a prior for index % 7 == 0, our compression gets *worse* because our
    // input data is *too short* to train on it.
    vpx_writer wri ={0};
    vpx_start_encode(&wri, compressed);
    for (size_t i = 0; i < sizeof(uncompressed); ++i) {
        uint8_t encoded_so_far = 0; // <-- this is the number encoded so far (in reverse order), as a prior for the rest
        for(int bit = 0; bit < 8; ++bit) {
            bool cur_bit = !!(uncompressed[i] & (1 << (7 - bit)));
            vpx_write(&wri, cur_bit, encode[i % 2][i % 3 ==0][i % 5 == 0][encoded_so_far + (1 << bit)].prob);
            encode[i % 2][i % 3 ==0][i % 5 == 0][encoded_so_far + (1 << bit)].record_bit_and_rescale(cur_bit);
            if (cur_bit) {
                encoded_so_far |= (1 << bit); // <-- this is the number encoded so far, as a prior for the rest
            }
        }
    }
    vpx_stop_encode(&wri);
    printf("Buffer encoded dynamically results in %d size (%.2f%%)\n",
           wri.pos,
           100 * wri.pos / float(sizeof(uncompressed)));
    DynProb decode[2][2][2][256]; // for the EXERCISE this array needs to be increased as well
    vpx_reader rea={0};
    vpx_reader_init(&rea,
                    wri.buffer,
                    wri.pos);
    memset(roundtrip, 0, sizeof(roundtrip));
    for (size_t i = 0; i < sizeof(roundtrip); ++i) {
        uint8_t decoded_so_far = 0; // <-- this is the number decoded so far, as a prior for the rest
        for(int bit = 0; bit < 8; ++bit) {
            if (vpx_read(&rea, decode[i % 2][i % 3 ==0][i % 5 == 0][decoded_so_far + (1 << bit)].prob)) {
                roundtrip[i] |= (1 << (7 - bit));
                decode[i % 2][i % 3 ==0][i % 5 == 0][decoded_so_far + (1 << bit)].record_bit_and_rescale(true);
                decoded_so_far |= (1 << bit); // <-- this is the number decoded so far, as a prior for the rest
            } else {
                decode[i % 2][i % 3 ==0][i % 5 == 0][decoded_so_far + (1 << bit)].record_bit_and_rescale(false);
            }
        }
    }
    assert(vpx_reader_has_error(&rea) == 0);
    assert(memcmp(uncompressed, roundtrip, sizeof(uncompressed)) == 0);
}

int main () {
    printf("even = 0, odd is 1, or 3\n");
    populate_even_data();
    encode_with_adaptive_probability();
    printf("even = 0, odd is 1, or 3 followed by even = 1 or 3, odd is 0 or 2\n");
    populate_shifting_even_data();
    encode_with_adaptive_probability();
    printf("multiple of 3,5,7 is 3, even = 0, 1 otherwise\n");
    populate_rel_prime_with_2_3_5_and_7_data();
    encode_with_adaptive_probability();

    return roundtrip[0];
}
