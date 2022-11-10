import numpy as np
from src.pulsed_power_ml.model_framework.data_io import load_fft_file, read_training_files
from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_background, read_parameters, subtract_background, switch_detected, update_background_vector

def trainingdata_switch_detector(spectra: np.ndarray, parameters: dict):
    """
    Function to automatically label training spectra (from GNU Radio).

    Parameters
    ----------
    spectra
        array of raw spectra.
    parameters
        Dictionary of input parameters, containing 'appliances' and 'threshold'.

    Returns
    -------
    switch_positions
        Array of switch_positions for switch on and switch off.
    """
    switch_positions=np.zeros((2,spectra.__len__()))
    
    background_vector = []
    current_background = None
    counter = 0
    for spectrum in spectra:
        spectrum = 10**spectrum

        # is there a background?
        if current_background is not None:
            residual = subtract_background(spectrum, current_background)
            switch = switch_detected(residual, parameters['threshold'])
            if True in switch:
                switch_positions[switch.index(True),counter]=1
                print ("switch detected")
                print(counter)
                background_vector = []
                current_background = None
            else:
                background_vector = update_background_vector(background_vector, spectrum)
                current_background = calculate_background(background_vector)
        else:
            if background_vector.__len__() == 0:
                background_vector = spectrum
            else:
                background_vector = np.vstack((background_vector, spectrum))

            # is there enough data to calculate background?
            if background_vector.__len__() == pars['background_n']:
                current_background = calculate_background(background_vector)
                print("background calculated")
                print(counter)
        
        counter+=1
    return switch_positions


def get_switch_spectra(spectra, switch_positions, parameters):
    """
    Function to get clean spectra for switch events. Please make sure that there are at least 26 regular spectra for background calculation 

    Parameters
    ----------
    spectra
        array of raw spectra.
    switch_positions
        Switch positions as returned by the trainingdata_switch detector.
    parameters
        Dictionary of input parameters, containing 'appliances' and 'threshold'.

    Returns
    -------
    switch_spectra
        Array of clean switch_spectra for each switch event marked in switch_positions.
    """

    switch_indices = np.where(switch_positions==1)[1]
    switch_spectra = []
    for ind in switch_indices:
        raw_spectra = spectra[ind-parameters['background_n']-1:ind]
        background_vector = []
        for spectrum in raw_spectra:
            spectrum = 10**spectrum
            if background_vector.__len__() == 0:
                background_vector = spectrum
            else:
                background_vector = np.vstack((background_vector, spectrum))
        
        current_background = calculate_background(background_vector)
        
        raw_switch_spectrum = spectra[ind+1]
        raw_switch_spectrum = 10**raw_switch_spectrum
        clean_switch_spectrum = subtract_background(raw_switch_spectrum, current_background)
        if switch_spectra.__len__() == 0:
            switch_spectra = clean_switch_spectrum
        else:
            switch_spectra = np.vstack((switch_spectra, clean_switch_spectrum))
            
    return switch_spectra
        


def make_labeled_training_data(training_file: str, parameter_file: str):
    """
    Function to get clean spectra and respective labels. 

    Parameters
    ----------
    training_file
        Path to the binary containing the spectrum.
    parameter_file
        Path to the yml file containing the parameters.

    Returns
    -------
    switch_spectra
        Array of clean switch_spectra for each switch event marked in switch_positions.
    labels
        Array of labels, labels[0] referring to switch-on, labels[1] referring to switch-off events.
    """
    # 1 load specturm array
    pars = read_parameters(parameter_file)
    spectra = load_fft_file(training_file, pars["fft_size"])
    # detect switiching events
    switch_positions = trainingdata_switch_detector(spectra,pars)
    # disect spectrum during switching event 
    switch_spectra = get_switch_spectra(spectra,switch_positions,pars)
    # add label 
    events = np.where(switch_positions==1)[0] # 0 = on, 1 = off
    labels = np.zeros((2,switch_spectra.__len__()))
    counter = 0
    for event in events:
        if event == 0:
            labels[0,counter] = 1
        else:
            labels[1,counter] = 1
        counter += 1
    return [switch_spectra, labels]


def explode_to_complete_label_vector(labels: np.ndarray, appliance: str, parameter_file: str):
    """
    Function to explode single appliance label vector to complete label vector size of form
    [appliance1_on, appliance1_off, appliance2_on, appliance2_off,....]

    Parameters
    ----------
    labels
        Array of single appliance labels, array[0] containing switch-on, array[1] switch_off events
    appliance
        Appliance name, matching name given in parameter file.
    parameter_file
        Path to the yml file containing the parameters.
        
    Returns
    -------
    complete_labels
        Array of labels including all appliances given in parameter file.
    """
    # get index of single appliance in parameter file appliances list
    pars = read_parameters("src/pulsed_power_ml/models/gupta_model/parameters.yml")
    ind = pars["appliances"].index(appliance)
    
    # construct zeroes array of total size 
    complete_label_vector = np.zeros((pars["appliances"].__len__()*2,labels[0].__len__()))
    
    # replace respective entries with input labels
    complete_label_vector[ind*2] = labels[0]
    complete_label_vector[(ind*2)+1] = labels[1]
    
    return complete_label_vector




def make_labeled_training_data(training_file, appliance_id):
    # 1 load specturm array
    # detecti switiching events
    # disect spectrum durin gswitching event
    # add label
    return [spectrum_switching_event, label]

if __name__ == "__main__":
    pars = read_parameters("src/pulsed_power_ml/models/gupta_model/parameters.yml")
    print(pars)
    spectra = load_fft_file("../training_data/2022-10-25_training_data/tube/FFTApparentPower_TubeOnOff_FFTSize131072",
                        2**17)
    
    switch_positions = trainingdata_switch_detector(spectra,pars)
    print(np.where(switch_positions==1))
    
    training_file = "../training_data/2022-10-25_training_data/tube/FFTApparentPower_TubeOnOff_FFTSize131072"
    parameter_file = "src/pulsed_power_ml/models/gupta_model/parameters.yml"
    spectra, label = make_labeled_training_data(training_file, parameter_file)
    compl_labels = explode_to_complete_label_vector(label,"tube",parameter_file)
    