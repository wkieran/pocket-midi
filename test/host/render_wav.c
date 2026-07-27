#include <stdio.h>
#include <math.h>
#include "make_wav.h"

// typedef struct
// {
//     double top_freq;
//     double wavetable_len;
//     float *wavetable;
// } wavetable_t;
//
// void add_wavetable(int len, float *wavetable_in, double top_freq) {}
//
// void set_frequency(double inc) {}
//
// void set_phase_offset(double offset) {}
//
// void update_phase(void) {}
//
// float get_output(void) {}
//
// float get_output_minus_offset(void) {}

void convert_table(float *data, short *data_short, int length)
{
    float x;
    for (int i = 0; i < length; i++) {
        x = data[i];
        if (x > 1.0)
            x = 1.0;
        if (x < -1.0)
            x = -1.0;
        data_short[i] = (short)(x * 32767.0);
    }
}

int main()
{
    // double phasor;       // phase accumulator
    // double phase_inc;    // phase increment
    // double phase_offset; // phase offset for PWN
    //
    // int num_wavetables;
    // wavetable_t wavetables[num_wavetable_slots];

    // float amp;
    // float freq;
    // float phase;
    // float time;
    // float d_time;

    int sample_rate = 44100;
    int num_seconds = 5;
    int num_samples = sample_rate * num_seconds;

    int wavetables_length = 2048;
    float wavetable[wavetables_length];

    float output[num_samples];
    short s_output[num_samples];

    for (int i = 0; i < wavetables_length; i++) {
        wavetable[i] = cosf(i * 2 * (float)(M_PI / (float)(wavetables_length)));
    }

    int pos = 0;
    for (int i = 0; i < num_samples; i++) {
        output[i] = wavetable[pos];
        pos += 4;
        if (pos >= wavetables_length)
            pos = 0;
    }
    convert_table(output, s_output, num_samples);

    write_wav("test.wav", num_samples, s_output, 44100);

    return 0;
}
