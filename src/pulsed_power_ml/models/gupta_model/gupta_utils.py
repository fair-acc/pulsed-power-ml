import yaml
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
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


def calculate_background(background_points):
    background = background_points.mean(axis=0)
    return background


if __name__ == "__main__":
    pars = read_parameters("src/pulsed_power_ml/models/gupta_model/parameters.yml")
    test = read_training_files(
        "../training_data/training_data_2022-10-12/halo",
        fft_size=pars['fft_size'],
        sample_size=pars['sample_size'],
    )
    spectra = load_fft_file(("../training_data/training_data_2022-10-12/halo/"
                             "HaloOnOff-FFTApparentPower-200KS-FFTSize16k_DAT_2022-10-12"),
                            pars['fft_size'])
    
    background_vector = spectra[np.arange(-pars['background_n'], 0)]
    current_background = calculate_background(background_vector)
    
    for spectrum in spectra[[400]]:
        plt.plot(spectrum-current_background)
        plt.show()
