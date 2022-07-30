import math
import numpy as np
#import librosa

WHITENING_NUM_BANDS = 30
WHITENING_WINDOW_SIZE = 4096
WHITENING_K = 2*WHITENING_WINDOW_SIZE
WHITENING_SAMPLE_RATE = 44100
whitening_band_center_freq_indices = [0] * (WHITENING_NUM_BANDS+2)


#-----------------------------------------------------------------------
def whitening_init_band_center_freqs():
  #this ends up ranging from 26.0 Hz to 6935 Hz
  global whitening_band_center_freq_indices;
  for b in range(WHITENING_NUM_BANDS+2):
    c_b = 229 * (pow(10, (b+1) / 21.4) - 1)
    c_b = c_b * (WHITENING_K / WHITENING_SAMPLE_RATE)

    #todo: could get better low-freq by not rounding and doing proper interpolation
    whitening_band_center_freq_indices[b] = round(c_b)
    if whitening_band_center_freq_indices[b] < 1:
      whitening_band_center_freq_indices[b] = 1
    if whitening_band_center_freq_indices[b] >= WHITENING_WINDOW_SIZE :
      whitening_band_center_freq_indices[b] = WHITENING_WINDOW_SIZE-1

#-----------------------------------------------------------------------
#input should be dft magnitude of WHITENING_WINDOW_SIZE samples padded to length WHITENING_K
#whitens magnitude from indices self->whitening_band_center_freq_indices[1] to self->whitening_band_center_freq_indices[WHITENING_NUM_BANDS] inclusive
#k=5 to k=576, inclusive for WINDOW_SIZE 2048
#k=10 to k=1153 inclusive for WINDOW_SIZE 4096
def spectral_whitening(magnitude):
  bands = [0] * WHITENING_NUM_BANDS
  centers = whitening_band_center_freq_indices
  nu_minus_1 = 0.33 - 1

  for b in range(WHITENING_NUM_BANDS):
    #rising side of triangular window
    step = 1.0 / (centers[b+1] - centers[b])
    H_b_k = 0
    for k in range(centers[b], centers[b+1]):
      #first coefficient is H_b_k but it is left for clarity
      bands[b] += H_b_k * magnitude[k]
      H_b_k += step

    #falling side of triangular window
    step = 1.0 / (centers[b+2] - centers[b+1])
    H_b_k = 1
    for k in range(centers[b+1], centers[b+2]):
      #todo: optimize by pre-computing magnitude[k] * magnitude[k]
      bands[b] += H_b_k * magnitude[k] * magnitude[k]
      H_b_k -= step

    #rest of equation 2
    bands[b] /= WHITENING_K;
    if bands[b] > 0:
      bands[b] = math.sqrt(bands[b])
      #compression coefficients
      bands[b] = pow(bands[b], nu_minus_1)

  for b in range(WHITENING_NUM_BANDS-1):
    step = (bands[b+1] - bands[b]) / (centers[b+2] - centers[b+1])
    gamma_k = bands[b]
    for k in range(centers[b+1], centers[b+2]):
      magnitude[k] *= gamma_k
      gamma_k += step

  magnitude[k] *= bands[b];

##-----------------------------------------------------------------------
#def test_print_arr(a):
#  print("-----length: {}-----".format(len(a)))
#  for i in range(len(a)):
#    print(a[i])
#
#
##-----------------------------------------------------------------------
#wave, sr = librosa.load("test.wav", sr=WHITENING_SAMPLE_RATE, mono=True)
#if len(wave) < WHITENING_WINDOW_SIZE:
#  print("test.wav could not be loaded because it is too short.");
#  exit(0)
#wave = wave[0:WHITENING_WINDOW_SIZE]
#wave = np.multiply(wave, np.hanning(WHITENING_WINDOW_SIZE))
#wave = np.pad(wave, (0, WHITENING_WINDOW_SIZE), 'constant', constant_values=(0, 0))
#spectra = np.fft.rfft(wave)
#spectra = spectra[0:WHITENING_WINDOW_SIZE]
#spectra = abs(spectra);
##test_print_arr(spectra)
#
#whitening_init_band_center_freqs()
#spectral_whitening(spectra)
#
#spectra = spectra[5:577]
#
#test_print_arr(spectra)
