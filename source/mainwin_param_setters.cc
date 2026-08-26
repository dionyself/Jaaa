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

#include "mainwin.h"


void Mainwin::set_bw(float bw) {
  if (bw > _bmax)
    bw = _bmax;
  if (bw < _bmin)
    bw = _bmin;
  _spect->_bw = _p_val = _bw = bw;
  _spect->_bits &= ~(Spectdata::YP_VAL | Spectdata::YM_VAL);
  alloc_fft(_spect);
}

void Mainwin::set_vamax(float avmax) {
  if (avmax >= 2 && avmax <= 100000) {
    if (_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc) {
      fprintf(stderr, "Error: You cannot update Host frequency "
                      "settings while Recording\n");
    } else {
      _spect->_avmax = avmax;
      //set_param(AVMAX);
      //_spect->_avcnt = 0;
      if (_butt[VIDAV]->stat()) {
        _spect->_avcnt = 1;
        _spect->_bits &= ~Spectdata::PEAKH;
      }
      redraw();
    }
  } else {
    fprintf(stderr, "Error: Vid averaging should be between 2 ands 100000\n");
  }
}

void Mainwin::set_f0(float f) {
  if (f > _f1 - _fm)
    f = _f1 - _fm;
  else if (f < _fmin)
    f = _fmin;
  if (f != _f0) {
    _spect->_f0 = _p_val = _f0 = f;
    _fc = 0.5 * (_f0 + _f1);
    _ngx = 0;
    redraw();
  }
}

void Mainwin::set_f1(float f) {
  if (f < _f0 + _fm)
    f = _f0 + _fm;
  else if (f > _fmax)
    f = _fmax;
  if (f != _f1) {
    _spect->_f1 = _p_val = _f1 = f;
    _fc = 0.5 * (_f0 + _f1);
    _ngx = 0;
    redraw();
  }
}

void Mainwin::set_fc(float f) {
  float d = _fc - _f0;
  _f0 += f - _fc;
  _f1 += f - _fc;
  if (_f0 < _fmin) {
    _f0 = _fmin;
    _f1 = _f0 + 2 * d;
  }
  if (_f1 > _fmax) {
    _f1 = _fmax;
    _f0 = _f1 - 2 * d;
  }
  _p_val = _fc = 0.5 * (_f0 + _f1);
  _spect->_f0 = _f0;
  _spect->_f1 = _f1;
  _ngx = 0;
  redraw();
}

void Mainwin::set_fs(float f) {
  if (f < 0.00001 * (_fmax - _fmin))
    f = 0.00001 * (_fmax - _fmin);
  _f0 = _fc - 0.5 * f;
  _f1 = _fc + 0.5 * f;
  if (_f0 < _fmin) {
    _f0 = _fmin;
    _f1 = 2 * _fc - _f0;
  }
  if (_f1 > _fmax) {
    _f1 = _fmax;
    _f0 = 2 * _fc - _f1;
  }
  _p_val = _f1 - _f0;
  _spect->_f0 = _f0;
  _spect->_f1 = _f1;
  _ngx = 0;
  redraw();
}

void Mainwin::set_host_f(float host_freq) {
  if (host_freq >= 0.0f && !(_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc)) {
    _host_freq = host_freq;
    //_p_val = _host_freq;
    if (_is_lsb_view) {
      _f0 = 0.0f;
      set_f1(_host_freq);
      //set_param(HOSTF);
    }
    redraw();
  } else {
    fprintf(stderr, "Error: You cannot update Host frequency settings "
                    "while Recording\n");
  }
}

void Mainwin::set_rec_dec(float rec_dec) {
  if (rec_dec >= 1.0f && !(_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc)) {
    _decimation_factor = (int)rec_dec;
    //set_param(REC_DEC);
    redraw();
  } else {
    fprintf(stderr, "Error: You cannot update decimation settings "
                    "while Recording\n");
  }
}

void Mainwin::set_cutoff(float cutoff) {
  if (cutoff >= 0.0f && !(_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc)) {
    _cutoff_freq = cutoff;
    //set_param(CUTOFF);
    redraw();
  } else {
    fprintf(stderr, "Error: You cannot update cutoff frequency "
                    "settings while Recording\n");
  }
}

void Mainwin::set_sched(float sched) {
  if (sched >= 0.0f && !(_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc)) {
    _p_val = sched;
    if (sched == 0.0f) {
      _rec_date_start = time(nullptr);
      _rec_start_countdown = 0;
    } else {
      _rec_date_start = time(nullptr) + (time_t)(sched * 60);
      float frames_per_sec = (_ipmod * INP_LEN > 0) ? ((float)_fsamp / (_ipmod * INP_LEN)) : 1.0f;
      _rec_start_countdown = (long long)(sched * 60.0f * frames_per_sec);
      set_param(SCHED); // Updating param to "_rec_date_start" since now _p_val is outdated
    }
    redraw();
  } else {
    fprintf(stderr, "Error: You cannot schedule a new recording while Recording\n");
  }
}

void Mainwin::set_rec_dt(float rec_dt) {
  if (rec_dt >= 0.0f && !(_is_recording || _rec_scheduled || _is_accumulating_csv || _is_scheduled_csv_acc)) {
    _rec_duration = rec_dt;
    float frames_per_sec = (_ipmod * INP_LEN > 0) ? ((float)_fsamp / (_ipmod * INP_LEN)) : 1.0f;
    _rec_samples_remaining = (long long)(rec_dt * 60.0f * frames_per_sec);
    set_param(TIMER);
    redraw();
  } else {
    fprintf(stderr, "Error: You cannot update duration settings while Recording\n");
  }
}

void Mainwin::set_a1(float a) {
  float d = _a1 - _a0;
  _a1 = a;
  _a0 = _a1 - d;
  if (_a1 > _amax) {
    _a1 = _amax;
    _a0 = _a1 - d;
  }
  if (_a0 < _amin) {
    _a0 = _amin;
    _a1 = _a0 + d;
  }
  _p_val = _a1;
  _ngy = 0;
  redraw();
}

void Mainwin::set_ar(float a) {
  _a0 = _a1 - a;
  if (_a0 < _amin)
    _a0 = _amin;
  if (_a0 > _a1 - 5)
    _a0 = _a1 - 5;
  _p_val = _a1 - _a0;
  _ngy = 0;
  redraw();
}

void Mainwin::set_a_nse(float a) {
  _a_nse = a;
  send_genp();
}

void Mainwin::set_a_si1(float a) {
  _p_val = _a_si1 = a;
  send_genp();
  show_param();
}

void Mainwin::set_f_si1(float a) {
  _f_si1 = a;
  send_genp();
}

void Mainwin::set_a_si2(float a) {
  _p_val = _a_si2 = a;
  send_genp();
  show_param();
}

void Mainwin::set_f_si2(float a) {
  _f_si2 = a;
  send_genp();
}