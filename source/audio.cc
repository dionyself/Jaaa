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


#include <math.h>
#include <cstring>
#include <fstream>
#include "audio.h"
#include "messages.h"


Audio::Audio (ITC_ctrl *cmain, const char *name) :
    A_thread ("Audio"),
    _jname (name),
    _cmain (cmain),
    _run_alsa (0),
    _run_jack (0),
    _active (false),
    _ncapt (8),
    _nplay (8),
    _input (-1),
    _data (0),
    _outs (0),
    _demulated_data (nullptr),
    _demulated_decimated_data (nullptr),
    _demod_decimation(100),
    _decim_counter(0),
    _decim_ind(0)
{
    _is_recording = false;
    _rec_duration = 0;
    _rec_date_start = 0;
    _rec_date_end = 0;
    _rec_capture_type = 0;
    _rec_file_type = 0;
    _rec_action = 0;
    _rec_decimation_factor = 100;
    _rec_fsamp = 0;
    _rec_host_freq = 500.0;
    _rec_cutoff_freq = 35.0;
    _host_freq = 500.0;
    _cutoff_freq = 35.0;
}


Audio::~Audio (void)
{
    if (_wav_file.is_open ()) _wav_file.close ();
    if (_run_alsa) close_alsa ();
    if (_run_jack) close_jack ();
    delete[] _outs;
    if (_demulated_data != nullptr) {
        delete[] _demulated_data;
        // fftwf_free(_demulated_data); // if fftwf is used
        _demulated_data = nullptr;
    }

    if (_demulated_decimated_data != nullptr) {
        delete[] _demulated_decimated_data;
        // fftwf_free(_demulated_decimated_data); // if fftwf is used
        _demulated_decimated_data = nullptr;
    }
}


void Audio::init (void)
{
    int i;

    for (i = 0; i < LSINE; i++)
    {
        _sine [i] = sin (2 * M_PI * i / LSINE);
    }
    _sine [LSINE] = 0;
    _g_bits = 0;
    _a_noise = 0.0;
    _a_sine1 = 0.0;
    _f_sine1 = 0.0;
    _p_sine1 = 0.0;
    _a_sine2 = 0.0;
    _f_sine2 = 0.0;
    _p_sine2 = 0.0;

    _wav_sample_count = 0;
    _demod_phase = 0.0;
    _demod_phase_inc = 0.0;
    _demod_decimation = 100;
    init_lpf_filter(_fsamp, _host_freq, _cutoff_freq);
}


void Audio::init_lpf_filter (float sample_rate, float host_freq, float cutoff_freq)
{
    _demod_phase_inc = 2.0 * M_PI * host_freq / sample_rate;
    _filter_stage1.setLowPass (sample_rate, cutoff_freq);
    _filter_stage2.setLowPass (sample_rate, cutoff_freq);
}

void Audio::demodulate_buffer (float* input_buffer, float* output_buffer_demulated, int num_samples)
{

    for (int i = 0; i < num_samples; i++)
    {
        double lo_signal = cos (_demod_phase);
        _demod_phase += _demod_phase_inc;
        if (_demod_phase > 2.0 * M_PI) _demod_phase -= 2.0 * M_PI;

        double mixed = input_buffer[i] * lo_signal;

        double filtered1 = _filter_stage1.process (mixed);
        double final_filtered = _filter_stage2.process (filtered1);

        // Here output_buffer_demulated is _demulated_data
        output_buffer_demulated[i] = (float) final_filtered;

        if (_decim_counter % _demod_decimation == 0)
        {
            _demulated_decimated_data[_decim_ind] = (float) final_filtered;
            _decim_ind++;

            // Decimated buffer becomes a ring buffer
            if (_decim_ind >= (_size / _demod_decimation)) {
                _decim_ind = 0; 
            }
            
            if (_is_recording)
            {
                write_sample_to_wav ((float) final_filtered);
            }
        }
        _decim_counter++;
    }
}

bool Audio::start_wav_recording (char filename[256], float original_sample_rate, float host_freq, float cutoff_freq, int decimation_factor)
{
    fprintf (stderr, "start_wav_recording ...\n");
    _wav_file.open (filename, std::ios::binary);
    if (!_wav_file.is_open ()) return false;

    _wav_sample_count = 0;
    _demod_decimation = decimation_factor;

    _demod_phase = 0.0;
    init_lpf_filter (original_sample_rate, host_freq, cutoff_freq);

    char dummy_header[44] = {0};
    _wav_file.write (dummy_header, 44);

    return true;
}

void Audio::write_sample_to_wav (float sample)
{
    if (!_wav_file.is_open ())
    {
        _is_recording = false;
        return;
    }

    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;

    int16_t int_sample = (int16_t) (sample * 32767.0f);

    _wav_file.write (reinterpret_cast<char*> (&int_sample), sizeof (int16_t));
    _wav_sample_count++;
}

void Audio::stop_wav_recording (float original_sample_rate)
{
    fprintf (stderr, "stop_wav_recording ...\n");
    _is_recording = false;
    init_lpf_filter(_fsamp, _host_freq, _cutoff_freq);

    if (!_wav_file.is_open ()) return;

    int subchunk2_size = _wav_sample_count * 2;
    int chunk_size = 36 + subchunk2_size;
    int sample_rate = (int) (original_sample_rate);
    int byte_rate = sample_rate * 2;

    _wav_file.seekp (0, std::ios::beg);

    _wav_file.write ("RIFF", 4);
    _wav_file.write (reinterpret_cast<char*> (&chunk_size), 4);
    _wav_file.write ("WAVE", 4);

    _wav_file.write ("fmt ", 4);
    int subchunk1_size = 16;
    int16_t audio_format = 1;
    int16_t num_channels = 1;
    int16_t bits_per_sample = 16;
    int16_t block_align = num_channels * (bits_per_sample / 8);

    _wav_file.write (reinterpret_cast<char*> (&subchunk1_size), 4);
    _wav_file.write (reinterpret_cast<char*> (&audio_format), 2);
    _wav_file.write (reinterpret_cast<char*> (&num_channels), 2);
    _wav_file.write (reinterpret_cast<char*> (&sample_rate), 4);
    _wav_file.write (reinterpret_cast<char*> (&byte_rate), 4);
    _wav_file.write (reinterpret_cast<char*> (&block_align), 2);
    _wav_file.write (reinterpret_cast<char*> (&bits_per_sample), 2);

    _wav_file.write ("data", 4);
    _wav_file.write (reinterpret_cast<char*> (&subchunk2_size), 4);

    _wav_file.close ();
}


void Audio::init_alsa (const char *playdev, const char *captdev, 
                       int fsamp, int fsize, int nfrags, int ncapt, int nplay)
{
    _run_alsa = true;
    _alsa_handle = new Alsa_pcmi (playdev, captdev, 0, fsamp, fsize, nfrags);
    if (_alsa_handle->state () < 0)
    {
        fprintf (stderr, "Can't connect to ALSA\n");
        exit (1);
    } 
    _ncapt = _alsa_handle->ncapt ();
    _nplay = _alsa_handle->nplay ();
    _fsamp = fsamp;
    _fsize = fsize;

    _outs = new float [fsize];
    init ();
   
    _cmain->put_event (EV_MESG, new M_jinfo (fsamp, fsize, _jname));
    _alsa_handle->printinfo ();
    fprintf (stderr, "Connected to ALSA with %d inputs and %d outputs\n", _ncapt, _nplay); 

    if (thr_start (SCHED_FIFO, -10, 0x00010000))
    {
        fprintf (stderr, "Can't create ALSA thread with RT priority\n");
        if (thr_start (SCHED_OTHER, 0, 0x00010000))
        {
            fprintf (stderr, "Can't create ALSA thread\n");
            exit (1);
	}
    }
}


void Audio::close_alsa ()
{
//    fprintf (stderr, "Closing ALSA...\n");
    _run_alsa = false;
    get_event (1 << EV_EXIT);
    delete _alsa_handle;
}


void Audio::thr_main (void) 
{
    unsigned long b, k, m, n;
    int  i;

    _alsa_handle->pcm_start ();

    while (_run_alsa)
    {
	k = _alsa_handle->pcm_wait ();  
        while (k >= _fsize)
       	{
            if (_ncapt)
	    { 
		_alsa_handle->capt_init (_fsize);

		if (_data)
		{
  	            m = _fsize;
		    n = _size - _dind;
		    if (m >= n)
		    {
			if (_input < 0) memset (_data + _dind, 0, n * sizeof (float));
			else _alsa_handle->capt_chan (_input, _data + _dind, n);
            // Demulate here
            demodulate_buffer(_data + _dind, _demulated_data + _dind, n);
			_dind = 0;
			m -= n;
		    }
		    if (m)
		    {
			if (_input < 0) memset (_data + _dind, 0, m * sizeof (float));
			else _alsa_handle->capt_chan (_input, _data + _dind, m);
            // Demulate here
            demodulate_buffer(_data + _dind, _demulated_data + _dind, m);
			_dind += m;
		    }
		}

		_alsa_handle->capt_done (_fsize);
	    }

            if (_nplay)
	    {
		generate (_fsize);
                b = _g_bits & 15;
		_alsa_handle->play_init (_fsize);

		for (i = 0; i < _nplay; i++, b >>= 1)
		{
		    if (b & 1) _alsa_handle->play_chan (i, _outs, _fsize);
		    else       _alsa_handle->clear_chan (i, _fsize);
		}               

		_alsa_handle->play_done (_fsize);
	    }

            k -= _fsize;
            _scnt += _fsize;
	}
        process ();
    }

    _alsa_handle->pcm_stop ();
    put_event (EV_EXIT);
}


void Audio::init_jack (const char *server)
{
    char           s [16];
    int             opts;
    jack_status_t  stat;

    opts = JackNoStartServer;
    if (server) opts |= JackServerName;
    if ((_jack_handle = jack_client_open (_jname, (jack_options_t) opts, &stat, server)) == 0)
    {
        fprintf (stderr, "Can't connect to JACK\n");
        exit (1);
    }

    jack_set_process_callback (_jack_handle, jack_static_callback, (void *)this);
    jack_on_shutdown (_jack_handle, jack_static_shutdown, (void *)this);

    if (jack_activate (_jack_handle))
    {
        fprintf(stderr, "Can't activate JACK\n");
        exit (1);
    }
    _run_jack = true;

    for (int i = 0; i < _nplay; i++)
    {
        sprintf(s, "out_%d", i + 1);
        _jack_out [i] = jack_port_register (_jack_handle, s, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
    for (int i = 0; i < _ncapt; i++)
    {
        sprintf(s, "in_%d", i + 1);
        _jack_in [i] = jack_port_register (_jack_handle, s, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    }

    _outs = new float [4096];
    
    _fsamp = jack_get_sample_rate (_jack_handle);
    _fsize = jack_get_buffer_size (_jack_handle);
    _jname = jack_get_client_name (_jack_handle);
    init ();
    _cmain->put_event (EV_MESG, new M_jinfo (_fsamp, _fsize, _jname));
    _active = true;
}


void Audio::close_jack ()
{
    jack_deactivate (_jack_handle);
    jack_client_close (_jack_handle);
}


void Audio::jack_static_shutdown (void *arg)
{
    return ((Audio *) arg)->jack_shutdown ();
}


void Audio::jack_shutdown (void)
{
    _cmain->put_event (EV_JACK);
}


int Audio::jack_static_callback (jack_nframes_t nframes, void *arg)
{
    return ((Audio *) arg)->jack_callback (nframes);
}


int Audio::jack_callback (jack_nframes_t nframes)
{
    unsigned long  b, m, n;
    int     i;
    float  *p;
 
    if (!_active) return 0;
    if (_data && _input >= 0)
    {
        p = (float *)(jack_port_get_buffer (_jack_in [_input], nframes));
	m = nframes;
        n = _size - _dind;
        if (m >= n)
	{
            memcpy (_data + _dind, p, sizeof(jack_default_audio_sample_t) * n);
            // Demulate and decimate here
            demodulate_buffer(_data + _dind, _demulated_data + _dind, n);
            _dind = 0;
            p += n;
            m -= n;
        }
        if (m)
	{
            memcpy (_data + _dind, p, sizeof(jack_default_audio_sample_t) * m);
            // Demulate and decimate here
            demodulate_buffer(_data + _dind, _demulated_data + _dind, m);
            _dind += m;
	}
        _scnt += nframes;
    }

    generate (nframes);
    b = _g_bits & 255;   
    for (i = 0; i < _nplay; i++, b >>= 1)
    {    
          
        p = (float *)(jack_port_get_buffer (_jack_out [i], nframes));
    	if (b & 1) memcpy (p, _outs, sizeof(jack_default_audio_sample_t) * nframes);
        else       memset (p, 0,     sizeof(jack_default_audio_sample_t) * nframes);
    }

    process ();

    return 0;
}


void Audio::process (void) 
{
    int       k;
    ITC_mesg *M;

    if (_data)
    {
        k = _scnt / _step;
        if (k && _cmain->put_event_try (EV_TRIG, k) == ITC_ctrl::NO_ERROR) _scnt -= k * _step;
    }
   
    if (get_event_nowait (1 << EV_MESG) == EV_MESG)
    {
	M = get_message ();
	if (M->type () == M_BUFFP)
	{
	    M_buffp *Z = (M_buffp *) M; 
	    _data = Z->_data;
	    _size = Z->_size;
	    _step = Z->_step; 
	    _dind = 0;
	    _scnt = 0;

        // Free previous memory (prevent memory leaks if _size changes)
        if (_demulated_data != nullptr) {
            delete[] _demulated_data;
            //fftwf_free(_demulated_data);
            _demulated_data = nullptr;
        }
        if (_demulated_decimated_data != nullptr) {
            delete[] _demulated_decimated_data;
            //fftwf_free(_demulated_decimated_data);
            _demulated_decimated_data = nullptr;
        }

        // New memory assigned
        _demulated_data = new float[_size];
        //_demulated_data = (float*) fftwf_malloc(sizeof(float) * _size)

        // The decimated buffer size is the original size divided by decimation factor
        int decimated_size = _size / _demod_decimation;
        _demulated_decimated_data = new float[decimated_size];
        //_demulated_decimated_data = (float*) fftwf_malloc(sizeof(decimated_size) * _size)

        // Initialize buffers (Zeroed)
        std::fill(_demulated_data, _demulated_data + _size, 0.0f);
        std::fill(_demulated_decimated_data, _demulated_decimated_data + decimated_size, 0.0f);

        // Restarting decimation indices
        _decim_counter = 0;
        _decim_ind = 0;
	}
	else if (M->type () == M_INPUT)
	{
	    M_input *Z = (M_input *) M; 
	    _input = Z->_input;
	    if (_input >= _ncapt) _input = -1;
	}
	else if (M->type () == M_GENPAR)
	{
	    M_genpar *Z = (M_genpar *) M; 
            _g_bits  = Z->_g_bits;  
            _a_noise = sqrt (0.5) * pow (10.0, 0.05 * Z->_a_noise);
            _a_sine1  = pow (10.0, 0.05 * Z->_a_sine1);
            _f_sine1  = LSINE * Z->_f_sine1 / _fsamp; 
            _a_sine2  = pow (10.0, 0.05 * Z->_a_sine2);
            _f_sine2  = LSINE * Z->_f_sine2 / _fsamp; 
	}
    else if (M->type () == M_FINFO)
    {
        M_finfo *Z = (M_finfo *) M; 
            
        // Update Rec/Dec/Dem/Fil State vars
        strncpy(_rec_filename, Z->_rec_filename, 126);
        _rec_filename[127] = '\0';
        _rec_duration = Z->_rec_duration;
        _rec_date_start = Z->_rec_date_start;
        _rec_date_end = Z->_rec_date_end;
        _rec_capture_type = Z->_rec_capture_type;
        _rec_file_type = Z->_rec_file_type;
        _rec_action = Z->_rec_action;
        _rec_decimation_factor = Z->_rec_decimation_factor;
        _rec_fsamp = Z->_rec_fsamp;
        _rec_host_freq = Z->_rec_host_freq;
        _rec_cutoff_freq = Z->_rec_cutoff_freq;

        fprintf(stderr, "We received filename: %s fsamp: %lu host_freq: %.2f cutoff: %.2f decimation: %d\n",
        _rec_filename, 
        _rec_fsamp, 
        _rec_host_freq, 
        _rec_cutoff_freq, 
        _rec_decimation_factor);

        if (_rec_action == 1)
        {
            _is_recording = start_wav_recording (_rec_filename, _rec_fsamp, _rec_host_freq, _rec_cutoff_freq, _rec_decimation_factor);
        }
        if (_rec_action == 0)
        {
            stop_wav_recording (_rec_fsamp);
        }

    }
	M->recover ();
    }
}


void Audio::generate (int size) 
{
    int   i, j;
    float a, p, r;

    if (size > 4096) size = 4096;

    memset (_outs, 0, size * sizeof (float));

    if (_g_bits & M_genpar::WNOISE)
    {
	for (i = 0; i < size; i++)
	{
	    _outs [i] += _a_noise * _rngen.grandf ();
	}
    }
    if (_g_bits & M_genpar::SINE1)
    {  
        p = _p_sine1;       
	for (i = 0; i < size; i++)
        {
	    j = (int) p;
            if (j == LSINE) a = 0;
	    else
	    {
   	        r = p - j;
                a = (1.0 - r) * _sine [j] + r * _sine [j + 1];
	    }
            _outs [i] += _a_sine1 * a;
            p += _f_sine1;
            if (p >= LSINE) p -= LSINE;         
	}
        _p_sine1 = p;       
    }
    if (_g_bits & M_genpar::SINE2)
    {  
        p = _p_sine2;       
	for (i = 0; i < size; i++)
        {
	    j = (int) p;
            if (j == LSINE) a = 0;
	    else
	    {
   	        r = p - j;
                a = (1.0 - r) * _sine [j] + r * _sine [j + 1];
	    }
            _outs [i] += _a_sine2 * a;
            p += _f_sine2;
            if (p >= LSINE) p -= LSINE;         
	}
        _p_sine2 = p;       
    }
}
