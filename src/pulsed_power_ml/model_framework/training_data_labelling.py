import numpy as np
from src.pulsed_power_ml.model_framework.data_io import load_fft_file, read_training_files
from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_background, read_parameters, subtract_background, switch_detected, update_background_vector

def data_labeller(spectra: np.ndarray, parameters: dict, appliance: str):
    labels=np.zeros((parameters['appliances'].__len__(),spectra.__len__()))
    ind = parameters['appliances'].index(appliance)
    
    background_vector = []
    current_background = None
    counter = 0
    for spectrum in spectra:
        spectrum = 10**spectrum

        # is there a background?
        if current_background is not None:
            residual = subtract_background(spectrum, current_background)
            if switch_detected(residual, parameters['threshold']):
                labels[ind,counter]=1
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
    return(labels)



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
    
    labels = data_labeller(spectra,pars,'tube')
    print(np.where(labels==1))
    