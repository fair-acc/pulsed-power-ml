from math import ceil
import yaml
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks, savgol_filter

from src.pulsed_power_ml.model_framework.data_io import load_fft_file, read_training_files

def read_parameters(parameter_path: str):
    """
    Read parameters from a parameter yml file into a dictionary.

    Parameters
    ----------
    path_to_file
        Path to the .yml file.

    Returns
    -------
    Dictionary of parameters.
    """
    param_dic = yaml.safe_load(Path(parameter_path).read_text())
    param_dic['sec_per_fft'] = param_dic['fft_size'] / param_dic['sample_size']
    param_dic['freq_per_bin'] = param_dic['sample_size'] / param_dic['fft_size']
    return param_dic


def calculate_background(background_points: np.ndarray):
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


def subtract_background(spectrum: np.ndarray, background: np.ndarray):
    """
    Subtracts background from spectrum.

    Parameters
    ----------
    spectrum
        Raw spectrum.
    background
        Background to be subtracted from raw spectrum -- needs to have same length as spectrum.

    Returns
    -------
    Background subtracted spectrum
    """
    return spectrum-background


def switch_detected(res_spectrum: np.ndarray):
    """
    Scans background subtracted spectrum for switch event 
    (signal larger than standard deviation across more than 3/4 of spectrum) 
    to avoid dead time because of background re-calculation.

    Parameters
    ----------
    res_spectrum
        Background subtracted spectrum.

    Returns
    -------
    Boolean: True if switch event was detected, false if no switch event was detected
    """    
    # what percentage of bins has to be above or below the respective thresholds?
    perc_threshold = ceil(res_spectrum.__len__()*0.75)
    
    # how many bins are above the standard deviation?
    bins_above_std = res_spectrum[res_spectrum>res_spectrum.std()]
    n_bins_above_std = bins_above_std.__len__()
    
    # how many bins are below the negative standard deviation?
    bins_below_minus_std = res_spectrum[res_spectrum<-1*res_spectrum.std()]
    n_bins_below_minus_std = bins_below_minus_std.__len__()
    
    if ((n_bins_above_std>=perc_threshold) or 
        (n_bins_below_minus_std>=perc_threshold)):
        switch = True
    else:
        switch = False
    return switch
    

def event_detected(res_spectrum: np.ndarray):
    """
    Scans background subtracted spectrum for peaks. 
    Threshold for peak detection is more than one peak above 4 times the standard deviation.

    Parameters
    ----------
    res_spectrum
        Background subtracted spectrum.

    Returns
    -------
    Boolean: True if peak was detected, false if no peak was detected
    """
    
    # ToDo: equivalent loop with negative signal to find dips (switch off)
    
    peaks, _ = find_peaks(res_spectrum, height=4*res_spectrum.std())

    if peaks.__len__()>1:
        peak_detected = True
    else:
        peak_detected = False
        
    return peak_detected


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
        residual = subtract_background(spectrum, current_background)
        # plt.plot(residual)
        # plt.show()
        print("Switch detected: {}".format(switch_detected(residual)))
        if switch_detected(residual):
            print("Continuing...")
        else:
            peaks, _ = find_peaks(residual, height=4*residual.std())
            print("Peak detected: {}".format(event_detected(residual)))
            print(peaks)
            plt.xlim([0,200])
            plt.plot(peaks, residual[peaks], "ob"); 
            plt.plot(residual); 
            plt.plot([0,residual.__len__()],[4*residual.std(),4*residual.std()]); 
            plt.legend(['peaks','residual', '4*stdev']);
            plt.show()
        print("---------------------------------")
        


        