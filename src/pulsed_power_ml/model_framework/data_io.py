"""
This module contains functions for the model framework concerning data I/O.
"""

import glob

import numpy as np


FFT_APPARENT_POWER_FIX = "FFTApparentPower"
FFT_VOLTAGE_FIX = "FFTVoltage"
FFT_CURRENT_FIX = "FFTCurrent"
APPARENT_POWER_FIX = "S"
ACTIVE_POWER_FIX = "P"
REACTIVE_POWER_FIX = "Q"
PHASE_SHIFT_FIX = "Phi"


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
    full_spectrum = np.fromfile(path_to_file, dtype=np.float32).reshape((-1, fft_size))
    real_part = full_spectrum[:, 0:int(fft_size/2)]
    return real_part


def read_training_files(path_to_folder: str,
                        fft_size: int,
                        sample_size: int) -> np.array:
    """
    Function to automatically convert binary training files (from GNU Radio) to an array of data points.

    Parameters
    ----------
    path_to_folder
        Path to the folder containing all training files (expects specific naming scheme).
    fft_size
        Number of data points that have been used to calculate the FFT in GNURadio.
    sample_size
        Number of samples per second which has been used to collect the raw data.

    Returns
    -------
    list_of_data_points
        Array of data points as they would be streamed from GNURadio.
    """

    # Read spectra
    fft_voltage_file_name = glob.glob(path_to_folder + f"/*-{FFT_VOLTAGE_FIX}-*")[0]
    fft_voltage = load_fft_file(fft_voltage_file_name, fft_size)

    fft_current_file_name = glob.glob(path_to_folder + f"/*-{FFT_CURRENT_FIX}-*")[0]
    fft_current = load_fft_file(fft_current_file_name, fft_size)

    fft_apparent_power_file_name = glob.glob(path_to_folder + f"/*-{FFT_APPARENT_POWER_FIX}-*")[0]
    fft_apparent_power = load_fft_file(fft_apparent_power_file_name, fft_size)

    # read P, Q, S and Phi versus time
    # ToDo: Need to synchronize file sinks in GNU Radio first. Use dummy values as place holder
    dummy_power = -1  # to make sure, that no one interprets this as a real value
    apparent_power = np.ones((fft_apparent_power.shape[0], 1)) * dummy_power
    active_power = np.ones((fft_apparent_power.shape[0], 1)) * dummy_power
    reactive_power = np.ones((fft_apparent_power.shape[0], 1)) * dummy_power
    phase_difference = np.ones((fft_apparent_power.shape[0], 1)) * (-1000)

    # Glue these arrays together accordingly (stack horizontally)
    data_array = np.hstack([fft_voltage,
                            fft_current,
                            fft_apparent_power,
                            apparent_power,
                            active_power,
                            reactive_power,
                            phase_difference])

    return data_array


if __name__ == "__main__":
    test = read_training_files(
        "/home/thomas/projects/nilm_at_fair/training_data/training_data_2022-10-12/halo/",
        fft_size=2**14,
        sample_size=200_000,
    )
    print(test.shape)
