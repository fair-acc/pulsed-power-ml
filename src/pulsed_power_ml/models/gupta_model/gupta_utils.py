"""
This module contains functions needed by the Gupta classification algorithm.
"""
from typing import Tuple, Union
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

    # Normalize cleaned spectrum


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
    min_peak_height = 0 * cleaned_spectrum.std()  # This could probably be optimized.
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

def gupta_offline_switch_detection(data_point_array: np.array,
                                   window_size: int,
                                   spectrum_type: int = 2,
                                   fft_size: int = 2**17,
                                   step_size: Union[int, None] = None,
                                   threshold: float = 2000,
                                   log_scale: bool = False) -> np.array:
    """
    This function implements the switch detection algorithm according to Gupta.

    Parameters
    ----------
    data_point_array
        Array with respective data points
    window_size
        Number of spectra, that should be included in one window
    fft_size
        Full size of FFT.
    step_size
        Step size for the windows. Default = window_size
    log_scale
        If True, use logarithmic scale for data points.

    Returns
    -------
    switch_array
        Array w/ the same length as data_point_array containing 1s
    """
    if step_size is None:
        step_size = window_size

    spectrum_type_offset = int(spectrum_type * fft_size / 2)

    max_len = len(data_point_array)

    switch_array = np.zeros(max_len)

    dead_time_counter = 0

    for i in range(int(max_len / step_size)):

        if dead_time_counter != 0:
            dead_time_counter -= 1
            continue

        j = i * step_size
        k = i * step_size + window_size
        l = i * step_size + 2 * window_size
        m = i * step_size + 3 * window_size

        raw_background = data_point_array[j:k,
                                          spectrum_type_offset : spectrum_type_offset + int(fft_size / 2)]

        raw_signal = data_point_array[k:l,
                                      spectrum_type_offset : spectrum_type_offset + int(fft_size / 2)]


        background_mean = raw_background.mean(axis=0)
        signal_mean = raw_signal.mean(axis=0)

        if log_scale:
            difference_spectrum = 10 * np.log10(np.max(np.abs(background_mean - signal_mean))) + 30     # Watt to dBm
        else:
            difference_spectrum = np.max(np.abs(background_mean - signal_mean))

        if np.abs(difference_spectrum).max() > threshold:
            switch_array[k:l] = 1 # somewhere in this window a switch has been detected
            switch_array[l:m] = -1 # Dead time
            dead_time_counter = 2 # number of windows that will be skipped

    return switch_array

def tf_switch_detected(res_spectrum: tf.Tensor, threshold: tf.Tensor) -> Tuple[tf.Tensor, tf.Tensor]:
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
    spectrum_sum = tf.math.reduce_sum(res_spectrum)
    sum_above_thr = tf.math.greater(spectrum_sum, threshold)
    sum_below_minus_thr = tf.math.less(spectrum_sum, tf.math.multiply(tf.constant(-1, dtype=tf.float32), threshold))

    return sum_below_minus_thr, sum_above_thr

@tf.function
def tf_calculate_gaussian_params_for_peak(x: tf.Tensor, y: tf.Tensor) -> Tuple[tf.Tensor, tf.Tensor, tf.Tensor]:
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

    # calculate parameters of gaussian in quadratic form f(x) = exp(alpha*x^2 + beta*x + gamma)
    alpha = (f * j - g * h) / (e * j - g * i)
    beta = (h - alpha * i) / j
    gamma = z[0] - alpha * x[0]**2 - beta * x[0]

    # calculate the parameters of the gaussian in the exponential form f(x) = a * exp(- (x - b)**2 / 2*c**2)
    c = tf.math.sqrt(-1 / (2 * alpha))
    b = beta * c**2
    a = tf.math.exp(gamma + (b**2 / (2*c**2)))

    return a, b, c

@tf.function(
    input_signature=[tf.TensorSpec(shape=(2**16), dtype=tf.float32),
                     tf.TensorSpec(shape=(), dtype=tf.float32),
                     tf.TensorSpec(shape=(), dtype=tf.int32)]
)
def tf_find_peaks(data: tf.Tensor,
                  min_height: tf.Tensor,
                  min_output_length: tf.Tensor = tf.constant(10, dtype=tf.int32)) -> Tuple[tf.Tensor, tf.Tensor]:
    """Find peaks in 1-D tensor x.

    Considers three values at a time, if the max value is located at the center of the window (length=3) a peak is found.

    Parameters
    ----------
    data
        1-D tensor containing the data
    min_height
        Minimum height of peaks
    min_output_length
        Minimum length of output tensor

    Returns
    -------
    peak_indices, peak_heights
    """
    # use max pool to get maximum value in a windows of three points
    max_pool = tf.nn.max_pool1d(tf.reshape(data, (1, -1, 1)),
                                ksize=3,
                                strides=1,
                                padding='VALID')

    # check where the max value in a window of three is equal to the data point at the same position -> peak
    equal = tf.math.equal(tf.reshape(max_pool, (-1, )), data[1:-1])

    # only consider peaks greater or equal to min_height
    above_min_height = tf.math.less(x=min_height, y=data)

    equal = tf.math.logical_and(equal, above_min_height[1:-1])

    # add two false values to match the shape of the input
    equal = tf.concat([tf.constant([False], dtype=tf.bool),
                       equal,
                       tf.constant([False], dtype=tf.bool)],
                      axis=0)

    # get the peak indices
    peak_indices = tf.cast(x=tf.where(equal),
                           dtype=tf.int32)

    # get the peak heights
    peak_heights = tf.gather_nd(params=data, indices=peak_indices)

    return peak_indices, peak_heights


def tf_calculate_background(background_points: tf.Tensor) -> tf.Tensor:
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
    background = tf.math.reduce_mean(
        input_tensor=background_points,
        axis=0,
        name="calculate_background"
    )
    return background


def tf_subtract_background(raw_spectrum: tf.Tensor, background: tf.Tensor) -> tf.Tensor:
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
    return tf.math.subtract(x=raw_spectrum, y=background, name="subtract_background")


@tf.function(
    input_signature=[
        tf.TensorSpec(shape=(2**16), dtype=tf.float32),
        tf.TensorSpec(shape=(), dtype=tf.int32),
        tf.TensorSpec(shape=(), dtype=tf.int32),
        tf.TensorSpec(shape=(), dtype=tf.int32)
    ]
)
def tf_calculate_feature_vector(cleaned_spectrum: tf.Tensor,
                                n_peaks_max: tf.Tensor,
                                fft_size_real: tf.Tensor,
                                sample_rate: tf.Tensor) -> tf.Tensor:
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
    # min_peak_height = 4 * tf.math.reduce_std(input_tensor=cleaned_spectrum,
    #                                          axis=0,
    #                                          name="min_peak_height")

    # ToDo: Is min peak height necessary?
    min_peak_height = tf.constant(0, dtype=tf.float32)
    # Determine if peaks have positive or negative amplitude
    switch_off_factor = tf.cond(
        pred=tf.math.less(
            x=tf.math.abs(tf.math.reduce_min(cleaned_spectrum)),
            y=tf.math.abs(tf.math.reduce_max(cleaned_spectrum))
        ),
        true_fn=lambda: tf.constant([1], dtype=tf.float32, name="switch_off_factor"),
        false_fn=lambda: tf.constant([-1], dtype=tf.float32, name="switch_off_factor")
    )

    # Get peaks
    peak_indices, peak_heights = tf_find_peaks(data=cleaned_spectrum * switch_off_factor,
                                               min_height=min_peak_height,
                                               min_output_length=n_peaks_max)

    # select only the highest n_peaks_max peaks
    _, indices_unsorted = tf.math.top_k(
        input=peak_heights,
        k=n_peaks_max,
        name="k_largest_peaks",
    )

    indices = tf.sort(values=indices_unsorted)

    # k_largest_peaks_indices = tf.gather_nd(params=tf.reshape(peak_indices, (-1)),
    #                                        indices=tf.reshape(indices, (-1, 1)))

    k_largest_peaks_indices = tf.gather_nd(peak_indices,
                                           indices=tf.reshape(indices, (-1, 1)))


    k_largest_peaks_lower_indices = k_largest_peaks_indices - 1
    k_largest_peaks_higher_indices = k_largest_peaks_indices + 1

    # fit gaussian to every peak
    freq_per_bin = tf.cast(x=(sample_rate / fft_size_real), dtype=tf.float32)
    # ToDo: There is probably a more efficient way to implement this loop (see tf.while_loop)
    feature_vector = tf.TensorArray(
        dtype=tf.float32,
        size=n_peaks_max * 3,
        dynamic_size=False
    )
    for i in tf.range(tf.size(k_largest_peaks_indices)):
        low_index = k_largest_peaks_lower_indices[i]
        mid_index = k_largest_peaks_indices[i]
        high_index = k_largest_peaks_higher_indices[i]

        frequencies = tf.cast(
            x=tf.stack([low_index, mid_index, high_index], axis=0),
            dtype=tf.float32
        ) * freq_per_bin + freq_per_bin / 2

        amplitudes = cleaned_spectrum[low_index[0]:high_index[0]+1]

        a, mu, sigma = tf_calculate_gaussian_params_for_peak(
            x=frequencies,
            y=amplitudes * switch_off_factor
        )

        feature_vector = feature_vector.write(i*3, a * switch_off_factor)
        feature_vector = feature_vector.write(i*3+1, mu)
        feature_vector = feature_vector.write(i*3+2, sigma)


    feature_tensor = feature_vector.stack()

    feature_vector.close()
    return feature_tensor

@tf.function(
    input_signature=[
        tf.TensorSpec(shape=(2**16), dtype=tf.float32)])
def tf_transform_spectrum(new_spectrum: tf.Tensor) -> tf.Tensor:
    """
    Function to transform apparent power spectra from the new and probably correct flowgraph to be more like the
    training data, which have been produced w/ an old and incorrect version of the flowgraph.
    As soon as trainind data and inference is done w/ the exact same flowgraph, this function can be removed.

    Parameters
    ----------
    new_spectrum
        Apparent power spectrum

    Returns
    -------
    transformed_apparent_power_spectrum
    """

    factor = tf.constant(0.5002719012720522, dtype=tf.float32)

    offset = tf.constant(-8.881469725920565, dtype=tf.float32)

    new_spectrum_dbm = 10 * tf.math.log(new_spectrum) / tf.math.log(tf.constant(10, dtype=tf.float32)) + 30

    transformed_spectrum_dbm = tf.math.add(
        tf.math.multiply(new_spectrum_dbm, factor),
        offset
    )

    transformed_apparent_power_spectrum = tf.math.pow(
        tf.constant(10, dtype=tf.float32),
        tf.math.multiply(
            tf.math.subtract(
                transformed_spectrum_dbm,
                tf.constant(30, dtype=tf.float32)
            ),
            tf.constant(0.1, dtype=tf.float32)
        )
    )

    # reversed_transformed_apparent_power_spectrum = tf.reverse(transformed_apparent_power_spectrum,
    #                                                           axis=tf.constant([0]))

    return transformed_apparent_power_spectrum
