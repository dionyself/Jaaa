# Using default frequency (24 kHz)
python script.py datos.csv

# using 48 kHz
python script.py datos.csv -f 48

# Using 48 kHz for sample #12
python script.py datos.csv --freq 48 -s 12


#csv_viewer dependencies
pip install pandas matplotlib numpy

# Dev Requirements (sudo apt install <pkg name>)
libclthreads-dev
libclxclient-dev
libfftw3-dev
libzita-alsa-pcmi-dev
libjack-dev

# If  libjack-dev is not available,
# try libjack-jackd2-dev

# Ignore your local changes to Makefile
git update-index --skip-worktree Makefile

# Edit your Makefile to optimize your local compilation
# uncomment the line "CXXFLAGS += -march=native"
# also you may want to change optimizatio level -O3 to -O2
#-CXXFLAGS += -O3 -Wall
#-CXXFLAGS += -O2 -Wall

# clean
make clean

# Build
make

# Install to tmp location
DESTDIR=/tmp/jaaa_build make install

# Run on card 0
/tmp/jaaa_build/usr/local/bin/jaaa -A -d hw:0
