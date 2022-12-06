"""
This module implements the gupta model using the TensorFlow Python API in order to enable an export of
an instance of this model.
"""
from typing import Tuple

import tensorflow as tf
from tensorflow import keras

from src.pulsed_power_ml.models.gupta_model.tf_knn import TFKNeighborsClassifier
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_switch_detected
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_calculate_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_subtract_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_calculate_feature_vector


class TFGuptaClassifier(keras.Model):

    def __init__(self,
                 background_n: tf.Tensor = tf.constant(25, dtype=tf.int32),
                 fft_size_real: tf.Tensor = tf.constant(2**16, dtype=tf.int32),
                 sample_rate: tf.Tensor = tf.constant(2_000_000, dtype=tf.int32),
                 n_known_appliances: tf.Tensor = tf.constant(10, dtype=tf.int32),
                 spectrum_type: tf.Tensor = tf.constant(2, dtype=tf.int32),
                 switching_offset: tf.Tensor = tf.constant(1, dtype=tf.int32),
                 n_peaks_max: tf.Tensor = tf.constant(10, dtype=tf.int32),
                 apparent_power_list: tf.Tensor = tf.constant(value=tf.zeros([10]),
                                                              dtype=tf.float32),
                 n_neighbors: tf.Tensor = tf.constant(5, dtype=tf.int32),
                 distance_threshold: tf.Tensor = tf.constant(10, dtype=tf.float32),
                 name="TFGuptaClassifier",
                 **kwargs
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
            Tensor containing the apparent power per application. The order of the values is the same as
            for the labels for the KNN classifier.
        n_neighbors
            Number of nearest neighbors which should be checked during the classification.
        knn_weights
            How to weight the distance of al neighbors.
        distance_threshold
            If distance to the nearest neighbor is above this threshold, an event is classified as "other"
        """
        super(TFGuptaClassifier, self).__init__(name=name, **kwargs)

        # Parameters
        self.fft_size_real = fft_size_real
        self.sample_rate = sample_rate
        self.n_known_appliances = n_known_appliances
        self.spectrum_type = spectrum_type
        self.n_peaks_max = n_peaks_max

        # kNN-Classifier
        self.n_neighbors = n_neighbors
        self.distance_threshold = distance_threshold
        self.clf = TFKNeighborsClassifier(n_neighbors=n_neighbors)

        # Containers
        self.current_state_vector = tf.Variable(tf.zeros(shape=(n_known_appliances + 1),
                                                         dtype=tf.float32),
                                                dtype=tf.float32,
                                                name="current_state_vector")

        # Background attributes
        self.background_n = background_n
        self.background_vector = tf.Variable(
            initial_value=tf.zeros(shape=(self.background_n, self.fft_size_real), dtype=tf.float32),
            dtype=tf.float32,
            name="background_vector"
        )
        self.current_background_size = tf.Variable(initial_value=0,
                                                   dtype=tf.int32,
                                                   name="current_background_size")

        # Apparent Power data base
        self.apparent_power_list = apparent_power_list

        # Class attributes to accommodate for disturbances due to mechanical switches
        self.switching_offset = switching_offset
        self.n_data_points_skipped = tf.Variable(initial_value=0,
                                                 dtype=tf.int32)
        # self.skip_data_point = tf.Variable(initial_value=False,
        #                                    dtype=tf.bool)
        self.skip_data_point = tf.Variable(initial_value=0,
                                           dtype=tf.int32)

        return

    @tf.function
    def fit_knn(self, X: tf.Tensor, y: tf.Tensor) -> None:
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
        return

    @tf.function
    def call(self, X: tf.Tensor) -> tf.Tensor:
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
        # ToDo: Rework if-block
        # if tf.math.equal(self.skip_data_point, tf.constant(True, dtype=tf.bool)):
        if tf.math.greater(self.skip_data_point, tf.constant(0, dtype=tf.int32)):
            # Not enough data points have been skipped yet
            if tf.math.less(self.n_data_points_skipped, self.switching_offset):
                self.n_data_points_skipped.assign(tf.math.add(self.n_data_points_skipped, 1))
                # return self.current_state_vector
                return self.get_current_state_vector()

            # Enough data points have been skipped, do the classification now
            else:
                spectrum, apparent_power = self.crop_data_point(X)

                # calculate mean background
                current_background = tf_calculate_background(self.background_vector)

                # Get cleaned spectrum
                cleaned_spectrum = tf_subtract_background(raw_spectrum=spectrum,
                                                          background=current_background)

                # Classify
                self.current_state_vector = self.classify_switching_event(
                    cleaned_spectrum=cleaned_spectrum,
                    current_apparent_power=apparent_power
                )

                # clear background vector
                self.clear_background_vector()

                # reset skip flag
                # self.skip_data_point.assign(False)
                self.skip_data_point.assign(0)

                # return new state vector
                return self.current_state_vector

        # 0. Step: Remove unused information in data point
        spectrum, apparent_power = self.crop_data_point(X)

        # 1. Step: Check if background vector is full
        # ToDo: Rework this if-block
        if tf.math.greater(self.background_n, self.current_background_size):
            # Add current data point to background vector and return last state vector
            # self.background_vector.append(spectrum)
            tf.cond(
                pred=tf.math.less(self.current_background_size, self.background_n),
                true_fn=lambda: self.add_input_to_bk(spectrum),
                false_fn=lambda: self.replace_input_in_bk(spectrum),
            )

            self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=apparent_power)
            return self.current_state_vector

        # 2. Calculate background
        current_background = tf_calculate_background(self.background_vector)

        # 3. Subtract current background from current spectrum
        cleaned_spectrum = tf_subtract_background(raw_spectrum=spectrum,
                                                  background=current_background)

        # 4. Check if appliance has been changed its state
        event_detected_flag = tf_switch_detected(cleaned_spectrum,
                                                 threshold=1000)

        # ToDo: Rework if-block
        # if True not in event_detected_flag:
        if tf.math.logical_and(tf.math.equal(tf.constant(value=False, dtype=tf.bool), event_detected_flag[0]),
                               tf.math.equal(tf.constant(value=False, dtype=tf.bool), event_detected_flag[1])):
            # add spectrum to background vector and return last state vector
            tf.cond(
                pred=tf.math.less(self.current_background_size, self.background_n),
                true_fn=lambda: self.add_input_to_bk(spectrum),
                false_fn=lambda: self.replace_input_in_bk(spectrum),
            )

            self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=apparent_power)
            return self.current_state_vector

        # 5. If self.switching_offset is 0, then use the current data point to classify the event
        if self.switching_offset == 0:
            self.current_state_vector = self.classify_switching_event(cleaned_spectrum=cleaned_spectrum,
                                                                      current_apparent_power=apparent_power)
            # clear background vector
            self.clear_background_vector()

            # return new state vector
            return self.current_state_vector

        # If self.switching_offset is not 0, a number of data points needs to be skipped before a classification
        else:
            # self.skip_data_point.assign(True)
            self.skip_data_point.assign(0)
            self.n_data_points_skipped.assign_add(tf.constant(value=1, dtype=tf.int32))
            # return the current state vector until the classification is done
            return self.current_state_vector

    def predict(self, X: tf.Tensor) -> tf.Tensor:
        """
        Wrapper for call method

        Parameters
        ----------
        X

        Returns
        -------

        """
        return self.call(X)

    def crop_data_point(self, X: tf.Tensor) -> Tuple[tf.Tensor, tf.Tensor]:
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

    def clear_background_vector(self) -> None:
        """
        Clear background vector and reset current background size
        """
        self.background_vector.assign(tf.zeros(shape=(self.background_n, self.fft_size_real)))

        # self.background_vector.assign(
        #     tf.constant(
        #         value=tf.zeros(shape=(self.background_n, self.fft_size_real), dtype=tf.float32),
        #         dtype=tf.float32,
        #         name="background_vector"
        #     )
        # )

        self.current_background_size.assign(
            tf.constant(value=0,
                        dtype=tf.int32,
                        name="current_background_size")
        )
        return None

    def calculate_state_vector(self,
                               event_class: tf.Tensor) -> tf.Tensor:
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

        # updated_state_vector = copy.deepcopy(self.current_state_vector)

        event_index = tf.math.argmax(event_class, output_type=tf.int32)

        new_state_vector = tf.case(
            pred_fn_pairs=[
                # Case 1: Known appliance is switched on
                (tf.math.less(event_index, self.n_known_appliances),
                 lambda: tf.tensor_scatter_nd_update(tensor=self.current_state_vector,
                                                     indices=[[event_index]],
                                                     updates=[self.apparent_power_list[event_index]])),

                # Case 2: Known appliance is switched off
                (tf.math.logical_and(tf.math.greater_equal(event_index, self.n_known_appliances),
                                     tf.math.less(event_index, tf.math.multiply(self.n_known_appliances,
                                                                                tf.constant(2, dtype=tf.int32)))),
                 lambda: tf.tensor_scatter_nd_update(tensor=self.current_state_vector,
                                                     indices=[[event_index]],
                                                     updates=[tf.constant(0, dtype=tf.float32)])),
            ],
            default=lambda: self.current_state_vector
        )

        self.current_state_vector = new_state_vector

        return new_state_vector

    def calculate_unknown_apparent_power(self, current_apparent_power: tf.Tensor) -> tf.Tensor:
        """
        Calculate an updated version of the state vector with an updated value of "unknown".

        Parameters
        ----------
        current_apparent_power
            Current, total value of apparent power

        Returns
        -------
        updated_state_vector
        """
        # updated_state_vector = copy.deepcopy(self.current_state_vector)
        known_power = tf.math.reduce_sum(self.current_state_vector[:-1])
        power_difference = tf.math.subtract(current_apparent_power, known_power)
        unknown_power = tf.math.maximum(power_difference,
                                        tf.constant(0, dtype=tf.float32))
        updated_state_vector = tf.concat([self.current_state_vector[:-1],
                                          tf.expand_dims(unknown_power, axis=0)],
                                         axis=0)
        return updated_state_vector

    def classify_switching_event(self,
                                 cleaned_spectrum: tf.Tensor,
                                 current_apparent_power: tf.Tensor) -> tf.Tensor:
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
        feature_vector = tf_calculate_feature_vector(cleaned_spectrum=cleaned_spectrum,
                                                     n_peaks_max=self.n_peaks_max,
                                                     fft_size_real=self.fft_size_real,
                                                     sample_rate=self.sample_rate)

        # 2. Classify event
        distances, event_class = self.clf(feature_vector)

        # Check if known or unknown event via the smallest distance
        tf.cond(
            pred=tf.math.greater(tf.math.reduce_min(distances), self.distance_threshold),
            true_fn=lambda: tf.one_hot(indices=self.n_known_appliances,
                                       depth=tf.math.add(self.n_known_appliances, tf.constant(1, dtype=tf.int32))),
            false_fn=lambda: event_class
        )

        # 3. Update current state vector accordingly
        self.current_state_vector = self.calculate_state_vector(event_class=event_class)

        # 4. Update current state vector's power value for "unknown"
        self.current_state_vector = self.calculate_unknown_apparent_power(current_apparent_power=current_apparent_power)

        return self.current_state_vector

    def add_input_to_bk(self, input: tf.Tensor) -> None:
        """
        Function to add spectrum to background collection if background collection is not complete, yet.

        Parameters
        ----------
        input
            Spectrum
        """
        self.replace_input_in_bk(input)

        self.current_background_size.assign(
            tf.add(
                self.current_background_size,
                tf.constant(1, dtype=tf.int32)
            )
        )

        return None

    def replace_input_in_bk(self, input: tf.Tensor) -> None:
        """
        Function to replace oldest spectrum in background collection with **input**.

        Parameters
        ----------
        input
            Spectrum to be added to background collection.
        """
        self.background_vector.assign(
            tf.tensor_scatter_nd_update(
                tensor=tf.roll(input=self.background_vector, shift=1, axis=0),
                indices=tf.constant([[0]], dtype=tf.int32),
                updates=tf.reshape(input, shape=(1, -1))
            )
        )
        return None

    def get_current_state_vector(self) -> tf.Tensor:
        return self.current_state_vector


if __name__ == "__main__":

    # Model test
    import sys
    from typing import Tuple

    sys.path.append("/home/thomas/projects/nilm_at_fair/repository")
    import numpy as np
    import tensorflow as tf
    import matplotlib.pyplot as plt
    from scipy.signal import find_peaks
    import pandas as pd

    from src.pulsed_power_ml.models.gupta_model.gupta_clf import GuptaClassifier
    from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base
    from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters
    from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_calculate_gaussian_params_for_peak, gaussian, \
        tf_find_peaks

    from src.pulsed_power_ml.model_framework.data_io import read_training_files

    from src.pulsed_power_ml.model_framework.visualizations import plot_data_point_array

    # load training data
    training_data_folder = "/home/thomas/projects/nilm_at_fair/training_data/"
    features_file = f"{training_data_folder}/Features_ApparentPower.csv"
    labels_file = f"{training_data_folder}/Labels_ApparentPower.csv"

    features = tf.constant(value=pd.read_csv(features_file).values, dtype=tf.float32)
    labels = tf.constant(value=pd.read_csv(labels_file).values, dtype=tf.float32)

    # load apparent power list
    apparent_power_list = tf.constant(
        value=[4.8, 480, 490, 24, 42, 42, 27, 33, 50, 50, 50],
        dtype=tf.float32
    )

    # Instantiate model
    model = TFGuptaClassifier(
        background_n=tf.constant(5, dtype=tf.int32),
        n_peaks_max=tf.constant(10, dtype=tf.int32),  # Training data has 10 peaks...
        apparent_power_list=apparent_power_list,
        n_neighbors=tf.constant(3, dtype=tf.int32)
    )

    # Fit KNN
    model.fit_knn(X=features, y=labels)

    # Load GR data
    gr_data_folder = "/home/thomas/projects/nilm_at_fair/training_data/2022-10-25_training_data/mixed/"
    data_point_array = read_training_files(path_to_folder=gr_data_folder,
                                           fft_size=2**17)

    # Apply model to data
    state_vector_list = list()
    for data_point in data_point_array[:20]:
        state_vector = model(tf.constant(10**data_point, dtype=tf.float32))
        state_vector_list.append(state_vector)

    # print(state_vector_list)

    # Save model
    model.save("TFGuptaSaveTest")
