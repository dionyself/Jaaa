// ----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2018 Fons Adriaensen <fons@linuxaudio.org>
//  Copyright (C) 2026 Dionys Rosario <dionyself@gmail.com>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#ifndef __AUDIO_H
#define __AUDIO_H


#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <vector>
#include <zita-alsa-pcmi.h>
#include <clthreads.h>
#include <jack/jack.h>
#include "rngen.h"

class BiquadLPF {
private:
    double z1, z2;
    double a1, a2, b0, b1, b2;

public:
    BiquadLPF() : z1(0), z2(0), a1(0), a2(0), b0(0), b1(0), b2(0) {}

    void setLowPass(double sample_rate, double cutoff_freq) {
        double w0 = 2.0 * M_PI * cutoff_freq / sample_rate;
        double cos_w0 = cos(w0);
        double alpha = sin(w0) / (2.0 * 0.70710678);

        double a0 = 1.0 + alpha;

        a1 = (-2.0 * cos_w0) / a0;
        a2 = (1.0 - alpha) / a0;
        b0 = ((1.0 - cos_w0) / 2.0) / a0;
        b1 = (1.0 - cos_w0) / a0;
        b2 = ((1.0 - cos_w0) / 2.0) / a0;

        z1 = 0; z2 = 0;
    }

    inline double process(double input) {
        double output = (b0 * input) + z1;
        z1 = (b1 * input) - (a1 * output) + z2;
        z2 = (b2 * input) - (a2 * output);
        return output;
    }
};

class Audio : public A_thread
{
public:

    Audio (ITC_ctrl *cmain, const char *name);
    virtual ~Audio (void);

    void  init_alsa (const char *playdev, const char *captdev,
                     unsigned int fsamp, int frsize, int nfrags, int ncapt, int nplay);
    void  init_jack (const char *server);

    // WAV Recording control
    size_t predict_memory_requirements(float original_sample_rate, int decimation_factor);
    bool  start_wav_recording (char _rec_filename[128], float original_sample_rate, float host_freq, float cutoff_freq, int decimation_factor);
    void  stop_wav_recording (char filename[128], float original_sample_rate);

private:

    enum { LSINE = 4096, LRAND = 4096, MRAND = LRAND - 1 };

    virtual void thr_main (void);
    void  init (void);
    void  process (void);
    void  generate (int size);
    void  close_alsa (void);
    void  close_jack (void);
    void  jack_shutdown (void);
    int   jack_callback (jack_nframes_t nframes);
    void  init_lpf_filter (unsigned int sample_rate, float host_freq, float cutoff_freq);
    void  demodulate_buffer (float* input_buffer, float* output_buffer_demulated, int num_samples);
    void  write_sample_to_wav (float sample);
    void  write_wav_header(float original_sample_rate);

    const char     *_jname;
    ITC_ctrl       *_cmain;

    volatile bool   _run_alsa;
    Alsa_pcmi      *_alsa_handle;

    volatile bool   _run_jack;
    jack_client_t  *_jack_handle;
    jack_port_t    *_jack_in  [8];
    jack_port_t    *_jack_out [8];

    bool           _active;
    unsigned int  _fsamp;
    unsigned long  _fsize;
    int            _ncapt;
    int            _nplay;
    int            _input;
    float         *_data;

    

    int            _dind;
    int            _size;
    int            _step;
    int            _scnt;
    float          _sine [LSINE + 1];
    float         *_outs;
    int            _g_bits;
    float          _a_noise;
    float          _a_sine1;
    float          _f_sine1;
    float          _p_sine1;
    float          _a_sine2;
    float          _f_sine2;
    float          _p_sine2;
    Rngen          _rngen;

    // Output buffers
    float         *_demulated_data;
    
    // Decimation vars
    int _demod_decimation;
    int _decim_counter;

    // WAV recording vars
    std::ofstream   _wav_file;
    std::vector<float> _wav_buffer;
    int             _wav_sample_count;

    // Demulator/filter vars
    double          _demod_phase;
    double          _demod_phase_inc;
    BiquadLPF       _filter_stage1;
    BiquadLPF       _filter_stage2;
    float           _host_freq;
    float           _cutoff_freq;

    // Rec/Dec/Dem/Fil State vars
    bool _is_recording;
    float _rec_duration;
    time_t _rec_date_start;
    time_t _rec_date_end;
    int _rec_capture_type;
    int _rec_file_type;
    int _rec_action;
    int _rec_decimation_factor;
    char _rec_filename[128];
    unsigned int _rec_fsamp;
    float _rec_host_freq;
    float _rec_cutoff_freq;
  
    static void jack_static_shutdown (void *arg);
    static int  jack_static_callback (jack_nframes_t nframes, void *arg);
};

#endif
