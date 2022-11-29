"""
This module contains functions needed by the Gupta classification algorithm.
"""
from typing import Tuple
import yaml
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
from scipy.optimize import curve_fit

import tensorflow as tf

from src.pulsed_power_ml.model_framework.data_io import load_fft_file


def read_parameters(parameter_path: str) -> dict:
    """
    Read parameters from a parameter yml file into a dictionary.

    Parameters
    ----------
    parameter_path
        Path to the .yml file.

    Returns
    -------
    Dictionary of parameters.
    """
    param_dic = yaml.safe_load(Path(parameter_path).read_text())
    param_dic['sec_per_fft'] = param_dic['fft_size'] / param_dic['sample_rate']
    param_dic['freq_per_bin'] = param_dic['sample_rate'] / param_dic['fft_size']
    return param_dic


def read_power_data_base(path_to_file: str) -> list:
    """
    Read yaml file containing the apparent power values of all known devices and return a list in this form:
    [(name_0, value_0), (name_1, value_1), ...]

    Parameters
    ----------
    path_to_file
        Path to yaml file

    Returns
    -------
    apparent_power_list
    """
    power_dict = yaml.safe_load(Path(path_to_file).read_text())
    apparent_power_list = list()
    for key, value in power_dict.items():
        apparent_power_list.append((key, value))
    return apparent_power_list


def calculate_background(background_points: np.ndarray) -> np.ndarray:
    """
    Calculates background from input spectra.

    Parameters
    ----------
    background_points
        2D array of spectra to calculate background from.

    Returns
    -------
    Background spectrum.
    """    
    background = background_points.mean(axis=0)
    return background


def subtract_background(raw_spectrum: np.ndarray, background: np.ndarray) -> np.ndarray:
    """
    Subtracts background from spectrum.

    Parameters
    ----------
    raw_spectrum
        Raw spectrum.
    background
        Background to be subtracted from raw spectrum -- needs to have same length as spectrum.

    Returns
    -------
    Background subtracted spectrum
    """
    return raw_spectrum-background


def switch_detected(res_spectrum: np.ndarray, threshold: int) -> Tuple[bool, bool]:
    """
    Scans background subtracted spectrum for switch event 
    (signal larger than input parameter threshold value) 
    to avoid dead time because of background re-calculation.

    Parameters
    ----------
    res_spectrum
        Background subtracted spectrum.
    threshold
        Threshold value

    Returns
    -------
    flag_tuple
        Tuple containing two boolean values, first is True, if a "switch on" event is detected, second is True if
        a "switch off" event ist detected.
    """
  
    # sum above threshold?
    sum_above_thr = res_spectrum.sum()>threshold
    sum_below_minus_thr = res_spectrum.sum()<-1*threshold

    switchon = False
    switchoff = False
    
    if sum_above_thr:# >= 1:
        #switchon = True # This way for raw specrra minus raw background
        switchoff = True # This way for normed spectra minus normed background

    elif sum_below_minus_thr:# >= 1:
        #switchoff = True # This way for raw specrra minus raw background
        switchon = True # This way for normed spectra minus normed background
        
    return switchon, switchoff
    


def update_background_vector(old_background_vector: np.ndarray, raw_spectrum: np.ndarray) -> np.ndarray:
    """
    Updates an existing background vector by removing the first element 
    and appending the input spectrum at the end.
    
    Parameters
    ----------
    old_background_vector
        Vector of n raw spectra containing x bins each.
    raw_spectrum
        Raw spectrum of length x to be added to the vector.

    Returns
    -------
    np.ndarray((n,x)): updated background_vector
    """
    new_background_vector = np.vstack((old_background_vector[1:], raw_spectrum))
    return new_background_vector


def gaussian(x: float, a: float, mu: float, sigma: float) -> float:
    """
    Gaussian with amplitude.

    Parameters
    ----------
    x
        Independent variable.
    a
        Amplitude
    mu
        Expected value
    sigma
        Sqrt(Variance)

    Returns
    -------
    a * exp(- (x - mu)**2 / (2 * sigma**2) )
    """
    return a * np.exp(-(x - mu)**2 / (2 * sigma**2))


def fit_gaussian_to_peak(frequency_window: np.ndarray, magnitude_window: np.ndarray) -> np.ndarray:
    """
    Fit a gaussian to the data points in **peak_window**

    Parameters
    ----------
    frequency_window
        Array of frequencies (x-values).
    magnitude_window
        Array of corresponding magnitudes (y-values)

    Returns
    -------
    parameters
        Array with [a, mu, sigma]
    """
    # Initial guesses
    a_init = magnitude_window.max()
    mu_init = frequency_window.mean()
    sigma_init = np.sqrt(sum(magnitude_window * (frequency_window - mu_init)**2) / sum(magnitude_window))

    popt, pcov = curve_fit(f=gaussian,
                           xdata=frequency_window,
                           ydata=magnitude_window,
                           p0=[a_init, mu_init, sigma_init],
                           full_output=False)
    return popt


def calculate_feature_vector(cleaned_spectrum: np.ndarray,
                             n_peaks_max: int,
                             fft_size_real: int,
                             sample_rate: int) -> np.ndarray:
    """
    Calculate a feature vector given a cleaned spectrum.

    Parameters
    ----------
    cleaned_spectrum
        Array. Spectrum with background removed.
    n_peaks_max
        Max. number of peaks that are used to calculate features.
    fft_size_real
        Number of points in the real part of the spectrum.
    sample_rate
        Sample rate of the DAQ.

    Returns
    -------
    feature_vector
        Array of length 3 * n_peaks_max of the form: [a_0, mu_0, sigma_0, a_1, mu_1, sigma_1, ...]
    """
    # get peaks
    min_peak_height = 4 * cleaned_spectrum.std()  # This could probably be optimized.
    if abs(cleaned_spectrum.min()) > abs(cleaned_spectrum.max()):
        switch_off_factor = -1
    else:
        switch_off_factor = 1
    peak_indices, peak_properties = find_peaks(x=cleaned_spectrum * switch_off_factor,
                                               height=min_peak_height)

    # remove peaks in the first and last bin of the spectrum (cannot be used for gaussian fit)
    peak_height_list = [(peak_index, peak_height)
                        for peak_index, peak_height in zip(peak_indices, peak_properties['peak_heights'])
                        if peak_index > 0 and peak_index < fft_size_real]

    # select highest peaks if more than n_peaks_max peaks have been found
    if len(peak_height_list) > n_peaks_max:
        peak_height_list = sorted(peak_height_list,
                                  key=lambda x: x[1],
                                  reverse=True)[:n_peaks_max]
        peak_height_list = sorted(peak_height_list,
                                  key=lambda x: x[0])

    # Apply fit and store fit parameters in feature vector
    freq_per_bin = sample_rate / fft_size_real
    feature_vector = np.zeros(3 * n_peaks_max)
    for i, (peak_index, peak_height) in enumerate(peak_height_list):
        frequencies = [j * freq_per_bin + freq_per_bin / 2 for j in [peak_index - 1, peak_index, peak_index + 1]]
        magnitudes = cleaned_spectrum[peak_index - 1:peak_index + 2]
        try:
            a, mu, sigma = fit_gaussian_to_peak(np.array(frequencies), np.array(magnitudes))
        except RuntimeError:
            a=0
            mu=0
            sigma=0
            print("WARNING: FIT NOT POSSIBLE")
        feature_vector[i * 3] = a  # * switch_off_factor For switch off events, amplitudes should be negative
        feature_vector[i * 3 + 1] = mu
        feature_vector[i * 3 + 2] = sigma

    return feature_vector


def tf_calculate_gaussian_params_for_peak(x: tf.Tensor, y: tf.Tensor) -> tf.Tensor:
    """Calculates the parameters of a gaussian function given the values in x and y.

    This is not a fit(!), but an exact calculation. x and y need to be of length 3.

    Parameters
    ----------
    x
        Tensor of shape (3, 1) containing the x values in ascending order.
    y
        Tensor of shape (3, 1) containing the matching y values.

    Returns
    -------
    gauss_params
        Tensor of shape (3, 1) containing the three parameters of a gaussian function.
    """

    z = tf.math.log(y)

    # Defining a bunch of temporary variables to make the equations less messy
    e = x[1]**2 - x[2]**2
    f = z[1] - z[2]
    g = x[1] - x[2]
    h = z[0] - z[1]
    i = x[0]**2 - x[1]**2
    j = x[0] - x[1]

    print("helpers")
    for var in (e, f, g, h, i, j):
        print(var)
    # calculate parameters of gaussian in quadratic form f(x) = exp(alpha*x^2 + beta*x + gamma)
    alpha = (f * j - g * h) / (e * j - g * i)
    beta = (h - alpha * i) / j
    gamma = z[0] - alpha * x[0]**2 - beta * x[0]

    print("alpha, beta, gamma")
    for var in (alpha, beta, gamma):
        print(f"{var}")
    # calculate the parameters of the gaussian in the exponential form f(x) = a * exp(- (x - b)**2 / 2*c**2)
    # alpha = tf.math.exp(c + a * b**2)
    # beta = -2 * a * b
    # gamma = tf.math.sqrt(-2 * a)
    #
    # alpha = tf.math.exp(c - b**2 / 4)
    # beta = (-1 * b) / (2*a)
    # gamma = tf.math.sqrt(-1 / (2 * a))

    c = tf.math.sqrt(-1 / (2 * alpha))
    b = beta * c**2
    a = tf.math.exp(gamma + (b**2 / (2*c**2)))

    # return tf.constant([alpha, beta, gamma], dtype=tf.float32)
    return a, b, c, alpha, beta, gamma


if __name__ == "__main__":
    pars = read_parameters("src/pulsed_power_ml/models/gupta_model/parameters.yml")

    # spectra = load_fft_file("../training_data/training_data_2022-10-12/led/LEDOnOff-FFTApparentPower-200KS-FFTSize16k_DAT_2022-10-12",pars['fft_size'])
    spectra = load_fft_file("../training_data/training_data_2022-10-12/halo/HaloOnOff-FFTApparentPower-200KS-FFTSize16k_DAT_2022-10-12",pars['fft_size'])
    # spectra = load_fft_file("../training_data/training_data_2022-10-12/tube/TubeOnOff-FFTApparentPower-200KS-FFTSize16k_DAT_2022-10-12",pars['fft_size'])
    # spectra = load_fft_file("../training_data/training_data_2022-10-12/mixed/MixedOnOff-FFTApparentPower-200KS-FFTSize16k_DAT_2022-10-12",pars['fft_size'])

    background_vector = spectra[np.arange(-pars['background_n'],0)]
    current_background = calculate_background(background_vector)
    
    plt.xlim([0,250])
    plt.imshow(spectra-current_background,aspect="auto")
    plt.show()
     
    # for spectrum in spectra[170:176]:  # Einschaltvorgang LED
    for spectrum in spectra[218:225]:  # Einschaltvorgang Halo
        #spectrum = 10**spectrum
        residual = subtract_background(spectrum, current_background)
        # plt.plot(residual)
        # plt.show()
        switch = switch_detected(residual,pars['threshold'])

        print("Switch detected: {}".format(True in switch))
