struct DynProb {
    uint8_t prob;
    int true_count;
    int false_count;
    DynProb() {
        prob = 127;
        true_count = 0;
        false_count = 0;
    }
    void record_bit(bool bit) {
        if (bit) {
            ++true_count;
        } else {
            ++false_count;
        }
        prob = 256 * (false_count + 1) / (true_count + 2 + false_count);
    }
    void record_bit_and_rescale(bool bit);
};
