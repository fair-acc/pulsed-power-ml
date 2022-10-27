"""
This module contains the implementation of a classifier following the approach by Gupta.
"""
from typing import Self
from collections import deque

import numpy as np
from sklearn.base import BaseEstimator, ClassifierMixin
from sklearn.utils.validation import check_X_y, check_array, check_is_fitted
from sklearn.utils.multiclass import unique_labels
from sklearn.metrics import euclidean_distances

from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import subtract_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import switch_detected
from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_feature_vector


class GuptaClassifier(BaseEstimator, ClassifierMixin):

    def __init__(self,
                 background_n: int = 25,
                 fft_size_real: int = 2**16,
                 sample_rate: int = 2_000_000,
                 n_known_appliances: int = 10,
                 spectrum_type: int = 0,
                 n_peaks_max: int = 10) -> None:
        """
        Instantiation method for the Gupta classifier.

        Parameters
        ----------
        background_n
            Number of data points used for the background estimation.
        fft_size_real
            Number of points in the spectrum (original FFT size / 2).
        sample_rate
            Sample rate of the DAQ.
        n_known_appliances
            Number of known appliances.
        spectrum_type
            Determines which spectrum is used. 0: voltage, 1: current, 2: apparent power
        n_peaks_max:
            Max. number of peaks, which are used to calculate features.
        """
        # Parameters
        self.background_n = background_n
        self.fft_size_real = fft_size_real
        self.sample_rate = sample_rate
        self.n_known_appliances = n_known_appliances
        self.spectrum_type = spectrum_type
        self.n_peaks_max = n_peaks_max

        # Containers
        self.current_state_vector = np.zeros(n_known_appliances + 1)  # one additional for "others"
        self.background_vector = deque(maxlen=self.background_n)

        return


    def fit(self, X: np.array, y: np.array) -> Self:
        """
        Method to fit the internal kNN-Classifier to the provided data.

        Parameters
        ----------
        X
            Training data (feature arrays for switching events).
        y
            Labels of the switching events.

        Returns
        -------
        Self
        """
        return self


    def predict(self, X: np.array) -> np.array:
        """
        Prediction method of the Gupta classifier.

        Parameters
        ----------
        X
            One data point containing spectra, power and phase data.

        Returns
        -------
        y
            state_vector containing the current estimated power for each appliance.
        """
        # 0. Step: Remove unused information in data point
        spectrum, apparent_power = self.crop_data_point(X)

        # 1. Step: Check if background vector is full
        if not self.background_n != len(self.background_vector):
            # Add current data point to background vector and return last state vector
            self.background_vector.append(spectrum)
            return self.current_state_vector

        # 2. Calculate background
        current_background = calculate_background(np.array(self.background_vector))

        # 3. Subtract current background from current spectrum
        cleaned_spectrum = subtract_background(spectrum=spectrum,
                                               background=current_background)

        # 4. Check if appliance has been changed its state
        event_detected_flag = switch_detected(cleaned_spectrum)
        if not event_detected_flag:
            # add spectrum to background vector and return last state vector
            self.background_vector.append(spectrum)
            return self.current_state_vector

        # 5. Calculate feature vector
        feature_vector = calculate_feature_vector(cleaned_spectrum=cleaned_spectrum,
                                                  n_peaks_max=self.n_peaks_max,
                                                  fft_size_real=self.fft_size_real,
                                                  sample_rate=self.sample_rate)

        # 6. Empty background vector
        self.background_vector.clear()

        # 7. Classify event
        # event_class = self.knn_clf.predict(feature_vector)

        # 8. Update current state vector accordingly
        # self.current_state_vector = self.update_state_vector(event_class)

        return self.current_state_vector


    def crop_data_point(self, X: np.array) -> (np.array, int):
        """
        One data point consists of multiple spectra and quantities. For this algorithm, however, only a subset is
        needed. This functions removes all unnecessary values from a data point.

        Parameters
        ----------
        X
            One data point.

        Returns
        -------
        cropped_data_point:
            Tuple: (spectrum, apparent_power)
        """
        i = self.spectrum_type * self.fft_size_real
        j = (self.spectrum_type + 1) * self.fft_size_real
        spectrum = X[i:j]
        apparent_power = X[-2]
        return (spectrum, apparent_power)
