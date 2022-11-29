"""
This module contains the implementation of a classifier following the approach by Gupta.
"""
from typing import Any
from collections import deque
import copy

import numpy as np
from sklearn.base import BaseEstimator
from sklearn.base import ClassifierMixin
from sklearn import neighbors

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
                 switching_offset: int = 1,
                 n_peaks_max: int = 10,
                 apparent_power_list: list = [],
                 n_neighbors: int = 5,
                 knn_weights: str = "distance",
                 distance_threshold: float = 10,
                 ) -> None:
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
        switching_offset
            Number of frames to skip after a switching event has been detected before classifying the event.
        n_peaks_max
            Max. number of peaks, which are used to calculate features.
        apparent_power_list
            List with one tuple per known appliance containing name and apparent power value:
            [(name_0, value_0), (name_1, value_1), ...]
        n_neighbors
            Number of nearest neighbors which should be checked during the classification.
        knn_weights
            How to weight the distance of al neighbors.
        distance_threshold
            If distance to the nearest neighbor is above this threshold, an event is classified as "other"
        """
        # Parameters
        self.background_n = background_n
        self.fft_size_real = int(fft_size_real)
        self.sample_rate = sample_rate
        self.n_known_appliances = n_known_appliances
        self.spectrum_type = spectrum_type
        self.n_peaks_max = n_peaks_max

        # kNN-Classifier
        self.n_neighbors = n_neighbors
        self.knn_weights = knn_weights
        self.distance_threshold = distance_threshold
        self.clf = neighbors.KNeighborsClassifier(n_neighbors=self.n_neighbors,
                                                  weights=self.knn_weights)

        # Containers
        self.current_state_vector = np.zeros(n_known_appliances + 1)  # one additional for "others"
        self.background_vector = deque(maxlen=self.background_n)

        # Apparent Power data base
        self.apparent_power_list = apparent_power_list

        # Class attributes to accommodate for disturbances due to mechanical switches
        self.switching_offset = switching_offset
        self.n_data_points_skipped = 0
        self.skip_data_point = False

        return

    def fit(self, X: np.array, y: np.array) -> Any:
        """
        Method to fit the internal kNN-Classifier to the provided data.

        Parameters
        ----------
        X
            Training data (feature arrays for switching events).
        y
            Labels of the switching events (one-hot-encoded).

        Returns
        -------
        Self
        """
        self.clf.fit(X, y)
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
        # If a switching event has been detected and self.switching_offset is not 0, skip frames before attempting
        # a classification
        if self.skip_data_point is True:
            # Not enough data points have been skipped yet
            if self.n_data_points_skipped < self.switching_offset:
                self.n_data_points_skipped += 1
                return self.current_state_vector
            # Enough data points have been skipped, do the classification now
            else:
                spectrum, apparent_power = self.crop_data_point(X)

                # calculate mean background
                current_background = calculate_background(np.array(self.background_vector))

                # Get cleaned spectrum
                cleaned_spectrum = subtract_background(raw_spectrum=spectrum,
                                                       background=current_background)

                # Classify
                self.current_state_vector = self.classify_switching_event(cleaned_spectrum=cleaned_spectrum,
                                                                          current_apparent_power=apparent_power)

                # clear background vector
                self.background_vector.clear()

                # reset skip flag
                self.skip_data_point = False

                # return new state vector
                return self.current_state_vector

        # 0. Step: Remove unused information in data point
        spectrum, apparent_power = self.crop_data_point(X)

        # 0.1 Step: Correct spectrum (ToDo: Remove this step once training data w/o log_10 amplitudes are available)
        #spectrum = 10**spectrum

        # 1. Step: Check if background vector is full
        if self.background_n > len(self.background_vector):
            # Add current data point to background vector and return last state vector
            self.background_vector.append(spectrum)
            self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=apparent_power)
            return self.current_state_vector

        # 2. Calculate background
        current_background = calculate_background(np.array(self.background_vector))

        # 3. Subtract current background from current spectrum
        cleaned_spectrum = subtract_background(raw_spectrum=spectrum,
                                               background=current_background)

        normed_spectrum = subtract_background(raw_spectrum=spectrum/spectrum.max(),
                                              background=current_background/current_background.max())
        # 4. Check if appliance has been changed its state
        event_detected_flag = switch_detected(normed_spectrum,threshold=30)
#                                              threshold=1000)
        if True not in event_detected_flag:
            # add spectrum to background vector and return last state vector
            self.background_vector.append(spectrum)
            self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=apparent_power)
            return self.current_state_vector

        # 5. If self.switching_offset is 0, then use the current data point to classify the event
        if self.switching_offset == 0:
            self.current_state_vector = self.classify_switching_event(cleaned_spectrum=cleaned_spectrum,
                                                                      current_apparent_power=apparent_power)
            # clear background vector
            self.background_vector.clear()

            # return new state vector
            return self.current_state_vector

        # If self.switching_offset is not 0, a number of data points needs to be skipped before a classification
        else:
            self.skip_data_point = True
            self.n_data_points_skipped += 1
            # return the current state vector until the classification is done
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
        return spectrum, apparent_power

    def calculate_state_vector(self,
                               event_class: np.array) -> np.array:
        """
        Given the current state vector and a classification results, return an updated state vector.

        Parameters
        ----------
        event_class
            One-hot encoded array of length 2N+1 (N is the number of known appliances).

        Returns
        -------
        updated_state_vector
        """

        updated_state_vector = copy.deepcopy(self.current_state_vector)

        if np.argmax(event_class) < self.n_known_appliances:
            # Case 1: Known appliance is switched on
            state_vector_index = np.argmax(event_class)
            print(state_vector_index)
            updated_state_vector[state_vector_index] = self.apparent_power_list[state_vector_index][1]

        elif self.n_known_appliances <= np.argmax(event_class) < self.n_known_appliances * 2:
            # Case 2: Known appliance is switched off
            state_vector_index = int(np.argmax(event_class) - self.n_known_appliances)
            updated_state_vector[state_vector_index] = 0

        else:
            # Case 3: Unknown event
            pass

        return updated_state_vector

    def calculate_unknown_apparent_power(self, current_apparent_power: float) -> np.array:
        """
        Update the value of "unknown" in the state vector.

        Parameters
        ----------
        current_apparent_power
            Current, total value of apparent power

        Returns
        -------
        updated_state_vector
        """
        updated_state_vector = copy.deepcopy(self.current_state_vector)
        known_power = sum(self.current_state_vector[:-1])
        updated_state_vector[-1] = max(current_apparent_power - known_power, 0)
        return updated_state_vector

    def classify_switching_event(self,
                                 cleaned_spectrum: np.array,
                                 current_apparent_power: float) -> np.array:
        """
        Classify a switching event based on the cleaned spectrum.

        Parameters
        ----------
        cleaned_spectrum
            Array containing the cleaned spectrum.
        current_apparent_power
            Total of the apparent power.
        Returns
        -------
        updated_state_vector
        """
        # 1. Calculate feature vector
        print("Event detected: Calculate feature vector")
        feature_vector = calculate_feature_vector(cleaned_spectrum=cleaned_spectrum,
                                                  n_peaks_max=self.n_peaks_max,
                                                  fft_size_real=self.fft_size_real,
                                                  sample_rate=self.sample_rate)

        # 2. Classify event
        distances, _ = self.clf.kneighbors([feature_vector])

        # Check if known or unknown event via the smallest distance
        if distances.min() > self.distance_threshold:
            # Case 1: Unknown event
            event_class = np.zeros(self.n_known_appliances * 2 + 1)[-1] = 1
        else:
            # Case 2: Known event
            event_class = self.clf.predict([feature_vector])

        # 3. Update current state vector accordingly
        self.current_state_vector = self.calculate_state_vector(event_class=event_class)

        # 4. Update current state vector's power value for "unknown"
        self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=current_apparent_power)

        return self.current_state_vector
