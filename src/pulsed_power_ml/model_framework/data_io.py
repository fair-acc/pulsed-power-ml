"""
This module contains functions for the model framework concerning data I/O.
"""

import glob
import warnings
from pathlib import Path

import numpy as np

import yaml

FFT_APPARENT_POWER_FIX = "FFTApparentPower"
FFT_VOLTAGE_FIX = "FFTVoltage"
FFT_CURRENT_FIX = "FFTCurrent"
APPARENT_POWER_FIX = "S_"
ACTIVE_POWER_FIX = "P_"
REACTIVE_POWER_FIX = "Q_"
PHASE_DIFFERENCE_FIX = "Phi_"


def load_binary_data_array(path_to_file: str,
                           fft_size_data_point: int = 2**16) -> np.array:
    """
    Load and transform the data point array from a binary. Output will have shape (n, 3*fft_size_data_point + 4)
    where n is the number of data points in the file.

    One data point consists of (Spectrum_U, Spectrum_I, Spectrum_S, P, Q, S, Phi)

    Parameters
    ----------
    path_to_file
        Path to the binary
    fft_size_data_point
        Size of the spectra in the binary (usually half of what has been used for the calculation of the FFT)

    Returns
    -------
    data_point_array
        Array of data points.
    """
    data_point_len = 3 * fft_size_data_point + 4
    return np.fromfile(path_to_file, dtype=np.float32).reshape((-1, data_point_len))


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


def load_fft_file(path_to_file: str,
                  fft_size: int) -> np.array:
    """
    Load and transform a fft spectrum from a binary file and reshape it to a 2d-array with timestep on axis 0 and
    frequency bins on axis 1.

    Parameters
    ----------
    path_to_file
        Path to the binary containing the spectrum.
    fft_size
        Number of points used for the FFT.

    Returns
    -------
    Array with timestep versus real part of the spectrum
    """
    spectrum = np.fromfile(path_to_file, dtype=np.float32)
    remainder = len(spectrum) % fft_size
    if remainder != 0:
        warnings.warn(f"Length of array in file {path_to_file} is not a multiple of FFT Size ({fft_size})!\n"
                      f"Ignoring last {remainder} entries to continue.")
        reshaped_spectrum = spectrum[:-remainder].reshape((-1, fft_size))
    else:
        reshaped_spectrum = spectrum.reshape((-1, fft_size))
    real_part = reshaped_spectrum[:, 0:int(fft_size/2)]
    return real_part


def reshape_one_dim_array(a: np.array,
                          target_size: int) -> np.array:
    """
    Reshape **a** to have the same dimension on axis 0 as **target_size**.

    Split **a** into parts of size a.shape[0] / target_size. Use the last value of this sub array for the new array.

    Parameters
    ----------
    a
        1D-array, which needs to be reshaped.
    target_size
        Desired size of the reshaped array.

    Returns
    -------
    reshaped_array
        1D-Array with the desired length
    """
    target_array = np.zeros(target_size)
    n_processed = 0
    for target_array_index in range(target_array.shape[0]):
        step_width = int(np.round((a.shape[0] - n_processed) / (target_size - target_array_index)))
        n_processed += step_width
        target_array[target_array_index] = a[n_processed - 1]

    return target_array


def load_pqsphi_file(path_to_file: str,
                     target_size: int) -> np.array:
    """
    Load one binary file containing either P, Q, S or Phi values.

    Parameters
    ----------
    path_to_file
        Path to the binary ccntaining the raw data.
    target_size
        Desired size of the 1D-array.

    Returns
    -------
    Array with the respective values
    """
    orig_array = np.fromfile(path_to_file, dtype=np.float32)
    reshaped_array = reshape_one_dim_array(orig_array, target_size).reshape((-1, 1))
    return reshaped_array


def read_training_files(path_to_folder: str,
                        fft_size: int) -> np.array:
    """
    Function to automatically convert binary training files (from GNU Radio) to an array of data points.

    Parameters
    ----------
    path_to_folder
        Path to the folder containing all training files (expects specific naming scheme).
    fft_size
        Number of data points that have been used to calculate the FFT in GNURadio.

    Returns
    -------
    list_of_data_points
        Array of data points as they would be streamed from GNURadio.
    """

    # Read spectra
    fft_voltage_file_name = glob.glob(path_to_folder + f"/*{FFT_VOLTAGE_FIX}*")[0]
    fft_voltage = load_fft_file(fft_voltage_file_name, fft_size)

    fft_current_file_name = glob.glob(path_to_folder + f"/*{FFT_CURRENT_FIX}*")[0]
    fft_current = load_fft_file(fft_current_file_name, fft_size)

    fft_apparent_power_file_name = glob.glob(path_to_folder + f"/*{FFT_APPARENT_POWER_FIX}*")[0]
    fft_apparent_power = load_fft_file(fft_apparent_power_file_name, fft_size)

    # read P, Q, S and Phi versus time
    active_power_file_name = glob.glob(path_to_folder + f"/*{ACTIVE_POWER_FIX}*")[0]
    active_power = load_pqsphi_file(active_power_file_name,
                                    fft_apparent_power.shape[0])

    reactive_power_file_name = glob.glob(path_to_folder + f"/*{REACTIVE_POWER_FIX}*")[0]
    reactive_power = load_pqsphi_file(reactive_power_file_name,
                                      fft_apparent_power.shape[0])

    apparent_power_file_name = glob.glob(path_to_folder + f"/*{APPARENT_POWER_FIX}*")[0]
    apparent_power = load_pqsphi_file(apparent_power_file_name,
                                      fft_apparent_power.shape[0])

    phase_difference_file_name = glob.glob(path_to_folder + f"/*{PHASE_DIFFERENCE_FIX}*")[0]
    phase_difference = load_pqsphi_file(phase_difference_file_name,
                                        fft_apparent_power.shape[0])

    list_of_data_arrays = [fft_voltage,
                           fft_current,
                           fft_apparent_power,
                           active_power,
                           reactive_power,
                           apparent_power,
                           phase_difference]
    # Take care of some minor errors during recording.

    min_length = min([len(data_array) for data_array in list_of_data_arrays])

    # Glue these arrays together accordingly (stack horizontally)
    data_array = np.hstack([fft_voltage[:min_length],
                            fft_current[:min_length],
                            fft_apparent_power[:min_length],
                            active_power[:min_length],
                            reactive_power[:min_length],
                            apparent_power[:min_length],
                            phase_difference[:min_length]])

    return data_array
