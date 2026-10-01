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


bool Mainwin::is_csv_busy() {
  if (_is_accumulating_csv || _is_scheduled_csv_acc){
    fprintf(stderr, "Error: You cannot update this setting while procesing CSV\n");
    return true;
  }
  return false;
}

bool Mainwin::is_wav_busy() {
  if ( _is_recording || _rec_scheduled) {
    fprintf(stderr, "Error: You cannot update this setting while procesing WAV\n");
    return true;
  }
  return false;
}

bool Mainwin::is_capture_busy() {
  return is_wav_busy() || is_csv_busy();
}

void Mainwin::stop_video_avg(void) {
  _butt[VIDAV]->set_stat(0);
  _spect->_avcnt = 0;
}

void Mainwin::start_video_avg(void) {
  _butt[VIDAV]->set_stat(2);
  _butt[PEAKH]->set_stat(0);
  _spect->_avcnt = 1;
  _spect->_bits &= ~Spectdata::PEAKH;
}

void Mainwin::stop_video_peakh(void) {
  _butt[PEAKH]->set_stat(0);
  _spect->_bits &= ~Spectdata::PEAKH;
}

void Mainwin::start_video_peakh(void) {
    _butt[PEAKH]->set_stat(2);
    _butt[VIDAV]->set_stat(0);
    _spect->_bits |= Spectdata::PEAKH;
    _spect->_avcnt = 0;
}

void Mainwin::toggle_video_avg(void) {
  if (_butt[VIDAV]->stat()) {
    stop_video_avg();
  } else {
    start_video_avg();
  }
}

void Mainwin::toggle_video_peakh(void) {
  if (_butt[PEAKH]->stat()) {
    stop_video_peakh();
  } else {
    start_video_peakh();
  }
}

void Mainwin::enable_ulf(void) {
  _f0 = 0.0f;
  set_f1(10.0f);
  set_bw(1.0f);
  set_param(BANDW);
  _butt[ULF_MOD]->set_stat(2);
  _butt[ELF_MOD]->set_stat(0);
  _is_lsb_view = false;
  _butt[DMOD]->set_stat(0);
  set_vamax(10.0f);
  start_video_avg();
  set_cutoff(12.0f);
  update_demulator();
}

void Mainwin::disable_ulf(void) {
  _f0 = 0.0f;
  set_f1(_fmax);
  set_bw(46.875f);
  set_param(BANDW);
  _butt[ULF_MOD]->set_stat(0);
  stop_video_avg();
  update_demulator();
}

void Mainwin::toggle_ulf(void) {
  if (_butt[ULF_MOD]->stat()) {
    disable_ulf();
  } else {
    enable_ulf();
  }
}

void Mainwin::enable_elf(void) {
  _f0 = 0.0f;
  set_f1(40.0f);
  set_bw(2.5f);
  set_param(BANDW);
  _butt[ELF_MOD]->set_stat(2);
  _butt[ULF_MOD]->set_stat(0);
  _is_lsb_view = false;
  _butt[DMOD]->set_stat(0);
  set_vamax(10.0f);
  start_video_avg();
  set_cutoff(42.0f);
  update_demulator();
}

void Mainwin::disable_elf(void) {
  _f0 = 0.0f;
  set_f1(_fmax);
  set_bw(46.875f);
  set_param(BANDW);
  _butt[ELF_MOD]->set_stat(0);
  stop_video_avg();
  update_demulator();
}

void Mainwin::toggle_elf(void) {
  if (_butt[ELF_MOD]->stat()) {
    disable_elf();
  } else {
    enable_elf();
  }
}

void Mainwin::enable_usb(void) {
  _f0 = 0.0f;
  if (_alias_host_freq > 0){
    set_f1(_alias_host_freq);
  } else{
    set_f1(_host_freq);
  }
  set_bw(375.0f);
  set_param(BANDW);
  _butt[DMOD]->set_stat(2);
  _butt[ELF_MOD]->set_stat(0);
  _butt[ULF_MOD]->set_stat(0);
  _butt[VIDAV]->set_stat(0);
  _spect->_avcnt = 0;
  _is_lsb_view = true;
  update_demulator();
}

void Mainwin::disable_usb(void) {
  _f0 = 0.0f;
  set_f1(_fmax);
  set_bw(375.0f);
  set_param(BANDW);
  _butt[DMOD]->set_stat(0);
  _is_lsb_view = false;
  update_demulator();
}

void Mainwin::toggle_usb(void) {
  if (_is_lsb_view) {
    disable_usb();
  } else {
    enable_usb();
  }
}
