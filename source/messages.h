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


#ifndef __MESSAGES_H
#define __MESSAGES_H


#include <clthreads.h>


#define  EV_MESG   0
#define  EV_X11   16
#define  EV_JACK  29
#define  EV_TRIG  30
#define  EV_EXIT  31

#define  M_BUFFP  1
#define  M_INPUT  2
#define  M_JINFO  3
#define  M_GENPAR 4
#define  M_FINFO  5


class M_buffp : public ITC_mesg
{
public:

    M_buffp (float *data, float *demod_data, int size, int step, float host_freq, float cutoff_freq,  int decimation_factor) : 
	ITC_mesg (M_BUFFP),
        _data (data),
        _demod_data (demod_data),
        _size (size),
        _step (step),
        _rec_host_freq (host_freq),
        _rec_cutoff_freq (cutoff_freq),
        _decimation_factor(decimation_factor) {}

    float    *_data;
    float    *_demod_data;
    int       _size;
    int       _step;
    float _rec_host_freq;
    float _rec_cutoff_freq;
    int _decimation_factor;
};

class M_finfo : public ITC_mesg
{
public:

    M_finfo (const char *filename, time_t date_start, time_t date_end, int duration, int capture_type,  int file_type, int action, unsigned int fsamp, float host_freq, float cutoff_freq, int decimation_factor) : 
    ITC_mesg (M_FINFO),
        _rec_date_start (date_start),
        _rec_date_end (date_end),
        _rec_duration (duration),
        _rec_capture_type (capture_type),
        _rec_file_type (file_type),
        _rec_action (action),
        _rec_fsamp (fsamp),
        _rec_host_freq (host_freq),
        _rec_cutoff_freq (cutoff_freq),
        _rec_decimation_factor (decimation_factor) {
            strncpy(_rec_filename, filename, 126);
            _rec_filename[127] = '\0';
        }

    char _rec_filename[128];
    time_t _rec_date_start;
    time_t _rec_date_end;
    int _rec_duration;
    int _rec_capture_type;
    int _rec_file_type;
    int _rec_action;
    unsigned int _rec_fsamp;
    float _rec_host_freq;
    float _rec_cutoff_freq;
    int _rec_decimation_factor;
 
};

class M_input : public ITC_mesg
{
public:

    M_input (int input) :
        ITC_mesg (M_INPUT),
        _input (input) {}

    int       _input;
};

class M_jinfo : public ITC_mesg
{
public:

    M_jinfo (unsigned int fsamp, unsigned long fsize, const char *jname) :
        ITC_mesg (M_JINFO),
        _fsamp (fsamp),
	_fsize (fsize),
        _jname (jname) {}

    unsigned int _fsamp;
    unsigned long _fsize;
    const char   *_jname;
};

class M_genpar : public ITC_mesg
{
public:

    enum { WNOISE = 256, SINE1 = 512, SINE2 = 1024 };

    M_genpar (void) :
        ITC_mesg (M_GENPAR) {}

    int         _g_bits;
    float       _a_noise;
    float       _a_sine1;
    float       _f_sine1;
    float       _a_sine2;
    float       _f_sine2;
};


#endif
