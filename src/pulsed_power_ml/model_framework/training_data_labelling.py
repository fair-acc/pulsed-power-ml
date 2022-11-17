import numpy as np
from glob import glob
from src.pulsed_power_ml.model_framework.data_io import load_fft_file
from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_background, read_parameters, subtract_background, switch_detected, update_background_vector, calculate_feature_vector


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
    switch_positions = np.zeros((2, spectra.__len__()))

    background_vector = np.array([])
    current_background = None
    counter = 0
    for spectrum in spectra:
        spectrum = 10**spectrum

        # is there a background?
        if current_background is not None:
            residual = subtract_background(spectrum, current_background)
            switch = switch_detected(residual, parameters['threshold'])
            if True in switch:
                switch_positions[switch.index(True), counter] = 1
                print("switch detected")
                print(counter)
                background_vector = np.array([])
                current_background = None
            else:
                background_vector = update_background_vector(background_vector,
                                                             spectrum)
                current_background = calculate_background(background_vector)
        else:
            if background_vector.__len__() == 0:
                background_vector = spectrum.reshape(1, -1)
            else:
                background_vector = np.vstack((background_vector, spectrum))

            # is there enough data to calculate background?
            if background_vector.__len__() == parameters['background_n']:
                current_background = calculate_background(background_vector)
                print("background calculated")
                print(counter)
        
        counter += 1
    return switch_positions


def get_switch_features(spectra, switch_positions, parameters):
    """
    Function to get clean spectra for switch events. Please make sure that
    there are at least 26 regular spectra for background calculation.

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
    switch_features
        Array of feature vectors for each switch event marked in switch_positions.
    """

    switch_indices = np.where(switch_positions == 1)[1]
    switch_features = []
    for ind in switch_indices:
        raw_spectra = spectra[ind-parameters['background_n']-1:ind]
        background_vector = np.array([])
        for spectrum in raw_spectra:
            spectrum = 10**spectrum
            if background_vector.__len__() == 0:
                background_vector = spectrum.reshape(1,-1)
            else:
                background_vector = np.vstack((background_vector, spectrum))
        
        current_background = calculate_background(background_vector)
        
        raw_switch_spectrum = spectra[ind+parameters["switching_offset"]]
        raw_switch_spectrum = 10**raw_switch_spectrum
        clean_switch_spectrum = subtract_background(raw_switch_spectrum, current_background)
        if switch_features.__len__() == 0:
            switch_features = calculate_feature_vector(clean_switch_spectrum, 
                                                       parameters['n_peaks'], 
                                                       parameters['fft_size']/2,
                                                       parameters['sample_rate']).reshape(1,-1)
        else:
            switch_features = np.vstack((switch_features, calculate_feature_vector(clean_switch_spectrum,
                                                                                   parameters['n_peaks'], 
                                                                                   parameters['fft_size']/2,
                                                                                   parameters['sample_rate'])))
            
    return switch_features
        


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
    switch_features
        Array of clean switch_features for each switch event marked in switch_positions.
    labels
        Array of labels, labels[0] referring to switch-on, labels[1] referring to switch-off events.
    """
    # 1 load specturm array
    pars = read_parameters(parameter_file)
    spectra = load_fft_file(training_file, pars["fft_size"])
    # detect switiching events
    switch_positions = trainingdata_switch_detector(spectra,pars)
    # disect spectrum during switching event 
    switch_features = get_switch_features(spectra,switch_positions,pars)
    # add label 
    events = np.where(switch_positions==1)[0] # 0 = on, 1 = off
    labels = np.zeros((switch_features.__len__(),2))
    counter = 0
    for event in events:
        if event == 0:
            labels[counter,0] = 1
        else:
            labels[counter,1] = 1
        counter += 1
    return [switch_features, labels]


def explode_to_complete_label_vector(labels: np.ndarray, appliance_id: int, parameter_file: str):
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
    # ToDo: Make appliance_id list-compatible for mixed spectra
    
    # get index of single appliance in parameter file appliances list
    pars = read_parameters(parameter_file)
    ind = appliance_id-1
    
    # construct zeroes array of total size 
    complete_label_vector = np.zeros((labels.__len__(), (pars["appliances"].__len__()*2)+1))
    
    # replace respective entries with input labels
    counter = 0
    for label in labels:
        if label[0]==1:
            complete_label_vector[counter][ind] = 1
        elif label[1]==1:
            complete_label_vector[counter][ind+pars['appliances'].__len__()] = 1
        counter += 1
    
    return complete_label_vector


def write_training_data_csvs(path_to_training_data_folders: str, output_path: str, parameter_file: str):
    """
    Function to write csvs containing training features and respective labels. 

    Parameters
    ----------
    path_to_training_data_folders
        Path to the folder containing the subfolders for all the binary files of one date
    output_path
        Path to folder where csvs will be written to
    parameter_file
        Path to the yml file containing the parameters.

    Returns
    -------
    1 if run without errors
    """
    pars = read_parameters(parameter_file)
    #sorts = ["ApparentPower","Voltage","Current"]
    sorts = ["ApparentPower"]
    appliances = list()
    appliance_ids = list()
    for item in pars['appliances']:
        appliances.append(list(item.values())[0])
        appliance_ids.append(list(item.keys())[0])
    # for each sort of spectrum (ApparentPower, Current, Voltage files)
    # make list of files of all appliances (glob)
    # loop over appliances to make features and labels
    for sort in sorts:
        filepath = '{0}/*/*{1}*'.format(path_to_training_data_folders, sort)
        files = glob(filepath)
        allfeatures = []
        alllabels = []
        for file in files: # loop over appliances
            appliance = None
            appliance_id = None
            if any(appl in file for appl in appliances):
                appliance = appliances[np.where([appliance in file for appliance in appliances])[0][0]]
                appliance_id = appliance_ids[appliances.index(appliance)]
            if appliance is not None: # single appliance, not mixed
                features, labels = make_labeled_training_data(file, parameter_file)
                if features.__len__() == 0:
                    print("Warning: No switches found for "+ appliance + " in " + sort + "!")
                else:
                    compl_labels = explode_to_complete_label_vector(labels,appliance_id,parameter_file)
                    if allfeatures.__len__() == 0:
                        allfeatures = features
                        alllabels = compl_labels
                    else:
                        allfeatures = np.vstack((allfeatures, features))
                        alllabels = np.vstack((alllabels, compl_labels))    

        np.savetxt(output_path + "Features_" + sort + ".csv", allfeatures, delimiter=",")
        np.savetxt(output_path + "Labels_" + sort + ".csv", alllabels, delimiter=",")
        
    return(1)
    

if __name__ == "__main__":
    # pars = read_parameters("src/pulsed_power_ml/models/gupta_model/parameters.yml")
    # print(pars)
    # spectra = load_fft_file("../training_data/2022-10-25_training_data/led/FFTApparentPower_LEDOnOff_FFTSize131072",
    #                     2**17)
    
    # switch_positions = trainingdata_switch_detector(spectra,pars)
    # print(np.where(switch_positions==1))
    
    # training_file = "../training_data/2022-10-25_training_data/led/FFTCurrent_ledOnOff_FFTSize131072"
    # parameter_file = "src/pulsed_power_ml/models/gupta_model/parameters.yml"
    # features, labels = make_labeled_training_data(training_file, parameter_file)
    # compl_labels = explode_to_complete_label_vector(labels,1,parameter_file)
    
    path_to_training_data_folders = "../training_data/2022-10-25_training_data/"
    output_path = "../training_data/labels_20221025/"
    parameter_file = "src/pulsed_power_ml/models/gupta_model/parameters.yml"
    _ = write_training_data_csvs(path_to_training_data_folders,output_path,parameter_file)