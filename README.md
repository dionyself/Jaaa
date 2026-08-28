## JAAA: Geophysical Edition

This is a modified version of JACK ALSA AUDIO ANALYZER (JAAA by Fons Adriaensen), enhanced with specialized features for geophysical research, including support for ultra-low frequency (ULF) analysis and integration with SimpleQuakeSensor hardware.  [SimpleQuakeSensor Hardware](https://github.com/dionyself/SimpleQuakeSensor)  

## Features

### Core Analysis

- FFT-based spectrum analysis with windowing and interpolation for accurate peak measurements (0.25 dB precision)
- Video averaging for noise measurements with configurable averaging length
- Peak hold mode for signal detection
- Dual marker system for precise frequency and amplitude measurements
- Frequency and amplitude zoom/scroll with mouse gestures

### Geophysical Modes

#### ULF (Ultra Low Frequency) Mode

- Optimized for frequencies below 3 Hz
- Bandwidth: 0.366 Hz
- Ideal for seismic and Earth-tide studies

#### ELF (Extremely Low Frequency) Mode

- Optimized for frequencies up to 10 Hz
- Bandwidth: 0.732 Hz
- Suitable for Schumann resonances and magnetotelluric studies

#### LSB (Lower Sideband) Mode

- Heterodyne mode with configurable host frequency (default: 500 Hz)
- Allows spectral analysis of signals below the Nyquist frequency
- Useful for analyzing deep ULF signals

### Data Export Features

#### CSV Spectrum Export

- Scheduled automatic data capture with configurable start time
- Configurable averaging time for each capture pass
- Configurable number of captures (passes)
- Exports both raw and dB-scaled spectrum data
- Matrix format: passes × frequency bins

#### WAV Recording

- Scheduled recording with configurable start time
- Configurable duration (minutes)
- Automatic decimation to reduce sample rate (preserves low-frequency content)
- Filename generation with timestamp and parameters
- Records decimated, demodulated data for geological analysis

### Demulator/Processor

- Configurable host frequency for heterodyning (1 Hz - 100 kHz)
- Configurable cutoff frequency for decimation filtering
- Configurable decimation factor to reduce data rate
- Real-time decimation and demodulation of input signal
- Live spectrum display of processed signal

## Controls

Some buttons will light up yellow when you click on them,
and there will be at most one of those active at any time.
This indicates the 'current parameter', which in most
cases can be modified in a number of ways:

- by typing a new value into the text widget, followed by ENTER,
- by using the '<' or '>' buttons to decrement or increment,
- by using the mouse wheel,
- by mouse gestures inside the display area.

### Input Selection

Select one of the 8 inputs.

### Frequency Controls

- Min/Max: Set displayed frequency range
- Cent/Span: Center frequency and span controls
- Click on frequency axis to set center frequency
- Mouse gestures for scrolling (drag left) and zooming (drag right)
   
### Amplitude Controls

- Max: Maximum amplitude on y-axis
- Range: Full display range
- Vertical mouse gestures for scrolling and zooming
 

### Analyser Settings

- Bandw: FFT length control (affects frequency resolution)
- Av.Max: Set the Max sample number for Video averaging
- Vid.Av: Video averaging for noise measurements
- Pk Hold: Peak hold mode
- Freeze: Freeze current spectrum while maintaining interactive controls

The analyser is based on a windowed FFT. Actually the windowing is performed
by convolution after the FFT, and combined with interpolation. The windowing
and interpolation ensure that displayed peaks will be accurate to 0.25 dB even
if the peak falls between the FFT bins. More accurate measurements can be made
using the markers (see below).

'Bandw' sets the FFT length, and hence the bandwidth of the analyser. Depending
on this value, the size of the display and the frequency range, you may sometimes
see two traces. This happens when the resolution of the analyser is better than the
display, so that one pixel contains more than one analyser value. In that case, the
dark blue trace is the peak value over the frequency range represented by each pixel,
and lighter one is the average value. The first one is correct for discrete frequencies,
and the latter should be used to read noise densities.
There is no mouse gesture to change the bandwidth.

### Data Export (CSV)

- Start Time: Schedule capture start (minutes from now, 0 = immediate)
- Avg time: Averaging time per capture pass (seconds)
- Amount: Number of capture passes
- Start/Stop: Toggle accumulation

### Demulator/Processing

- Host Freq: Heterodyne frequency for LSB mode
- Cutoff F: Low-pass filter cutoff for decimation
- Decimate: Decimation factor (1-256)
- Apply: Apply new demodulator settings

### Recording

- Start Time: Schedule recording start (minutes from now, 0 = immediate)
- Duration: Recording duration (minutes, 0 = indefinite)
- Start/Stop: Toggle recording

### Markers

- Clear: Remove all markers
- Peak: Click to place marker at nearest peak (interpolated)
- Noise: Click to place marker with noise density calculation

Markers are used in order to accurately read off values in the display. There can be
up to two markers, set by clicking at the desired frequency inside the display.
When there are two markers, the second one will move with each click, while the first
remains fixed. Measured values for the two markers, and their difference in frequency
and level are displayed in the upper left corner of the display.

When 'Peak' is selected, clicking inside the display will set a marker at the nearest 
peak. The exact frequency and level of the peak are found by interpolation, so the
frequency can be much more accurate than the FFT step, and the level corresponds to
the true peak value regardless of display or analyser resolution.

When 'Noise' is selected, clicking inside the display will set a noise marker.
The noise density (energy per Hz) is calculated and displayed.

## Quick Start

### Installation:
```
git clone https://github.com/dionyself/jaaa
cd jaaa/source
make
sudo make install
```

### Basic Setup:

- Connect your geophone/geophysical sensor via JACK/ALSA
- Launch JAAA: jaaa
- Select appropriate input channel
- Choose a mode: ULF, ELF, or LSB

### ULF/ELF Analysis:

- Click ULF View for frequencies < 3 Hz
- Click ELF View for frequencies < 10 Hz
- Adjust frequency range as needed
- Use video averaging for stable noise floor

### Data Capture:

- Spectrum CSV: Set averaging time and number of captures, schedule if desired
- WAV Recording: Set duration, schedule if desired, recording includes decimation

### Mouse Gestures

Gesture  Action
Horizontal Drag Left (on display)   Scroll frequency
Horizontal Drag Right (on display)  Zoom frequency
Vertical Drag Left (on display)  Scroll amplitude
Vertical Drag Right (on display) Zoom amplitude
Click on frequency scale   Set center frequency
Click in display area   Place marker
Click with 'Peak' active   Place peak marker
Click with 'Noise' active  Place noise marker
Mouse wheel Increment/decrement current parameter

## Output File Formats

### CSV Spectrum Files

- Raw: Power values (use dB = 10 * log10(value + 1e-30))
- dB: Decibel-scaled values
- Naming: spect_demod_YYYYMMDD_HHMMSS__YYYYMMDD_HHMMSS__N_raw/dB.csv
- Visualising a \*db.csvfile:
- `csv_viewer.py file.dB -f 24 -s 2` 
- # see the second sample (s) and plot until 24kHz (f) 
- # 48kHz supported

### WAV Recording Files

- Decimated and demodulated data
- Naming: geological_record_YYYYMMDD_HHMMSS__YYYYMMDD_HHMMSS__D.wav
- Where D = decimation factor

## Technical Notes

### Frequency Resolution

- FFT length automatically adjusts based on bandwidth setting
- Minimum bandwidth: depends on sample rate
- Maximum bandwidth: sample-rate dependent
- Interpolation provides accurate peak detection between FFT bins

### Decimation and Demodulation

- Host frequency configurable (1 Hz - 100 kHz)
- Cutoff frequency for anti-aliasing filter
- Decimation factor reduces effective sample rate by factor N
- Useful for extracting low-frequency signals from high-rate data

### Recording Details

- Recording starts immediately or on schedule
- Scheduler uses real-time clock (supports future start)
- Countdown display when recording is scheduled
- Automatic stop after configured duration

### Requirements

- JACK Audio Connection Kit or ALSA
- FFTW3 library
- X11 with Xft support
- gcc with C++11 support

### Building from Source
```
# Install dependencies (Debian/Ubuntu)
sudo apt-get install libfftw3-dev libjack-jackd2-dev libxft-dev \
libclthreads-dev libclxclient-dev libzita-alsa-pcmi-dev libjack-dev

# Build
make
sudo make install
```

### License

GNU General Public License v2 (See COPYING file for details)

### References

- Original JAAA by Fons Adriaensen
- [SimpleQuakeSensor Hardware](https://github.com/dionyself/SimpleQuakeSensor)

### Contact

Dionys Rosario dionyself@gmail.com  