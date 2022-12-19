import sys

import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import pandas as pd
import yaml

import copy

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MaxAbsScaler

from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_switch_detected
from src.pulsed_power_ml.models.gupta_model.gupta_utils import switch_detected

from src.pulsed_power_ml.models.gupta_model.gupta_utils import calculate_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_calculate_background

from src.pulsed_power_ml.models.gupta_model.gupta_utils import subtract_background
from src.pulsed_power_ml.models.gupta_model.gupta_utils import tf_subtract_background

from src.pulsed_power_ml.model_framework.visualizations import plot_state_vector_array

from src.pulsed_power_ml.model_framework.data_io import read_training_files

from src.pulsed_power_ml.models.gupta_model.tf_gupta_clf import TFGuptaClassifier
from src.pulsed_power_ml.models.gupta_model.gupta_clf import GuptaClassifier

FFT_SIZE = 2**17

gr_file_folder = "/home/thomas/projects/nilm_at_fair/training_data/2022-11-16_training_data/"
test_series = "r1"
#
# tf.config.run_functions_eagerly(False)

if __name__ == "__main__":

    # # Switch detection
    #
    # data_point_array = read_training_files(path_to_folder=f"{gr_file_folder}/{test_series}",
    #                                        fft_size=FFT_SIZE)
    #
    # n_background = 5
    #
    # s_spectrum_start_index = int((FFT_SIZE / 2) * 2)
    # s_spectrum_stop_index = int((FFT_SIZE / 2) * 3)
    #
    # # Test python implementations
    #
    # py_switch_detected = np.zeros(shape=(data_point_array.shape[0]))
    # tf_switch_detected_list = np.zeros(shape=(data_point_array.shape[0]))
    #
    # for i in range(len(data_point_array) - n_background):
    #
    #     background_spectra = data_point_array[i:i+n_background, s_spectrum_start_index:s_spectrum_stop_index]
    #     current_spectrum = data_point_array[i + n_background, s_spectrum_start_index:s_spectrum_stop_index]
    #
    #     # test python implementations
    #
    #     py_background = calculate_background(background_spectra)
    #     py_normed_spectrum = current_spectrum / current_spectrum.max()
    #     py_normed_background = py_background / py_background.max()
    #     py_cleaned_spectrum = subtract_background(raw_spectrum=py_normed_spectrum,
    #                                               background=py_normed_background)
    #     py_switch_on, py_switch_off = switch_detected(res_spectrum=py_cleaned_spectrum,
    #                                                   threshold=30)
    #     if py_switch_on:
    #         py_switch_detected[i + n_background] = 1
    #     elif py_switch_off:
    #         py_switch_detected[i + n_background] = -1
    #
    #     # Test TF implementation
    #
    #     tf_current_spectrum = tf.constant(current_spectrum, dtype=tf.float32)
    #
    #     tf_background = tf_calculate_background(tf.constant(background_spectra, dtype=tf.float32))
    #     tf_normed_spectrum = tf.math.divide(tf_current_spectrum,
    #                                         tf.math.reduce_max(tf_current_spectrum))
    #     tf_normed_background = tf.math.divide(tf_background,
    #                                           tf.math.reduce_max(tf_background))
    #     tf_cleaned_spectrum = tf_subtract_background(raw_spectrum=tf_normed_spectrum,
    #                                                  background=tf_normed_background)
    #
    #     tf_switch_on, tf_switch_off = tf_switch_detected(res_spectrum=tf_cleaned_spectrum,
    #                                                      threshold=tf.constant(30, dtype=tf.float32))
    #
    #     if tf_switch_on:
    #         tf_switch_detected_list[i + n_background] = 1
    #     elif tf_switch_off:
    #         tf_switch_detected_list[i + n_background] = -1
    #
    # # Make plots
    #
    # max_s = max(data_point_array[:, -2])
    #
    # fig = plt.figure(figsize=(8, 9))
    # ax = fig.add_subplot(2, 1, 1)
    # ax.plot(data_point_array[:, -2],
    #         "-",
    #         label="S")
    # ax.plot(py_switch_detected * max_s / 2 + max_s / 2,
    #         "-",
    #         label="Python Switches")
    #
    # # alt py
    # ax = fig.add_subplot(2, 1, 2)
    # ax.plot(data_point_array[:, -2],
    #         "-",
    #         label="S")
    # ax.plot(tf_switch_detected_list * max_s / 2 + max_s / 2,
    #         "-",
    #         label="alt Switches")
    #
    # # fig.savefig("/home/thomas/projects/nilm_at_fair/training_data/model_validation/switch_comparison_py_tf.pdf")
    #
    #
    # KNN Classifier

    # Load features and labels
    #
    # training_data_folder = "/home/thomas/projects/nilm_at_fair/training_data/training_data_maria/fair/labels_20221202_9peaks_validation/"
    # features_file = f"{training_data_folder}/Features_ApparentPower_1_p.csv"
    # labels_file = f"{training_data_folder}/Labels_ApparentPower_1_p.csv"
    #
    #
    # features = pd.read_csv(features_file)
    # labels = pd.read_csv(labels_file)
    #
    # features_scaled = MaxAbsScaler().fit_transform(features)
    #
    # X_train, X_test, y_train, y_test = train_test_split(
    #     features_scaled,
    #     labels,
    #     test_size=0.3
    # )
    #
    # # Dummy features and labels
    # dummy_features = [
    #     [1, 0, 0, 0],
    #     [0, 1, 0, 0],
    #     [0, 0, 1, 0],
    #     [0, 0, 0, 1]
    # ]
    #
    # dummy_labels = [
    #     [1, 0, 0, 0],
    #     [0, 1, 0, 0],
    #     [0, 0, 1, 0],
    #     [0, 0, 0, 1]
    # ]
    #
    # dummy_test = [
    #     [0, 0, 0, 1],
    #     [0, 0, 1.1, 0],
    #     [0, 1, 0, 0],
    #     [0.9, 0, 0, 0]
    # ]
    #
    # real_data = True
    # if not real_data:
    #     X_train = dummy_features
    #     y_train = dummy_labels
    #     X_test = dummy_test
    #
    # # Get Python KNN
    #
    # py_model = GuptaClassifier(
    #     background_n=5,
    #     spectrum_type=2,
    #     apparent_power_list=[],
    #     n_neighbors=3,
    #     distance_threshold=100,
    #     n_known_appliances=11,
    #     n_peaks_max=9,
    # )
    #
    # # fit py model knn
    # _ = py_model.fit(X_train, y_train)
    #
    # # test py model knn
    #
    # py_y_predicted = py_model.clf.predict(X_test)
    #
    # print(py_y_predicted)
    #
    #
    # # load and train TF model
    #
    # tf_model = TFGuptaClassifier(
    #     background_n=tf.constant(5, dtype=tf.int32),
    #     n_known_appliances=tf.constant(11, dtype=tf.int32),
    #     n_peaks_max=tf.constant(9, dtype=tf.int32),
    #     apparent_power_list=tf.constant([], dtype=tf.float32),
    #     n_neighbors=tf.constant(3, dtype=tf.int32),
    #     training_data_features=tf.constant(X_train, dtype=tf.float32),
    #     training_data_labels=tf.constant(y_train, dtype=tf.float32)
    # )
    #
    # # test tf model
    # tf_distance_list = list()
    # tf_results_list = list()
    #
    # for test_feature_vector in X_test:
    #     tf_distances, tf_results = tf_model.call_knn(tf.constant(test_feature_vector, dtype=tf.float32))
    #     tf_distance_list.append(tf_distances.numpy())
    #     tf_results_list.append(tf_results.numpy())
    #
    # # Compare results
    # for combined_results in zip(py_y_predicted, tf_results_list):
    #     if (combined_results[0] == combined_results[1]).all():
    #         print("bassd :-)")
    #     else:
    #         print("bassd ned! :-(")
    #         print(np.argmax(combined_results[0]), np.argmax(combined_results[1]))


    #################################
    # Compare whole predict methods #
    #################################

    training_data_folder = ("/home/thomas/projects/nilm_at_fair/training_data/training_data_maria/fair/"
                            "labels_20221202_9peaks/")

    features_file = f"{training_data_folder}/Features_ApparentPower_0.7_p.csv"
    labels_file = f"{training_data_folder}/Labels_ApparentPower_0.7_p.csv"

    features = pd.read_csv(features_file)
    labels = pd.read_csv(labels_file)

    X_train, X_test, y_train, y_test = train_test_split(
        features,
        labels,
        test_size=0.3,
        random_state=42
    )

    # load apparent power list
    apparent_power_data_base = ("/home/thomas/projects/nilm_at_fair/repository/src/pulsed_power_ml/models/gupta_model/"
                                "apparent_power_data_base.yml")

    with open(apparent_power_data_base) as yaml_file:
        apparent_power_dict = yaml.load(yaml_file, Loader=yaml.loader.SafeLoader)

    appliance_names = apparent_power_dict.keys()

    apparent_power_list = tf.constant(
        value=list(apparent_power_dict.values()),
        dtype=tf.float32
    )

    py_app_power_list = list()
    for app_power in apparent_power_list:
        py_app_power_list.append(("app_name", app_power))

    py_model = GuptaClassifier(
        background_n=10,
        spectrum_type=2,
        apparent_power_list=py_app_power_list,
        n_neighbors=3,
        distance_threshold=100,
        n_known_appliances=11,
        n_peaks_max=9,
        switching_offset=1
    )

    # tf_model = TFGuptaClassifier(
    #     background_n=tf.constant(10, dtype=tf.int32),
    #     n_known_appliances=tf.constant(11, dtype=tf.int32),
    #     apparent_power_list=apparent_power_list,
    #     n_neighbors=tf.constant(3, dtype=tf.int32),
    #     training_data_features=tf.constant(features, dtype=tf.float32),
    #     training_data_labels=tf.constant(labels, dtype=tf.float32),
    #     switching_offset=tf.constant(1, dtype=tf.int32)
    # )

    tf_model = tf.saved_model.load("/home/thomas/projects/nilm_at_fair/repository/src/pulsed_power_ml/models/"
                                   "gupta_model/TFGuptaModel_v1-0")

    # fit py model knn
    _ = py_model.fit(X_train, y_train)

    # load GR data

    folder_name_list = [
        "e1",
        "f1",
        "f2",
        "fp1",
        "h1",
        "h2",
        "l1",
        "mixed1",
        "mixed2",
        "ps1",
        "r1",
    ]

    for folder_name in folder_name_list:

        gr_data_folder = f"/home/thomas/projects/nilm_at_fair/training_data/2022-11-16_training_data/{folder_name}"
        data_point_array = read_training_files(path_to_folder=gr_data_folder,
                                               fft_size=2 ** 17)

        tf_state_vector_list = list()
        py_state_vector_list = list()

        for i, data_point in enumerate(data_point_array):
            # inference w/ tf model
            tf_state_vector = tf_model(tf.constant(data_point, dtype=tf.float32))
            tf_state_vector_list.append(copy.deepcopy(tf_state_vector.numpy()))
            # inference w/ py model
            py_state_vector = py_model.predict(data_point)
            py_state_vector_list.append(py_state_vector)

            # check differences
            diff_vec = tf_state_vector.numpy() - py_state_vector

            if np.sum(np.abs(diff_vec)) > 1:
                print(f"Large difference found in frame {i}")
                print(tf_state_vector.numpy)
                print(py_state_vector)
                print("hmmm :-(")


        make_plots = True
        if make_plots:
            print("Make Plots")
            fig = plot_state_vector_array(state_vector_list=np.array(tf_state_vector_list),
                                          label_list=list(appliance_names),
                                          true_apparent_power=data_point_array[:,-2])
            fig.savefig("/home/thomas/projects/nilm_at_fair/training_data/model_validation/"
                        f"tf_model_validation_{folder_name}_plot.pdf")

            py_fig = plot_state_vector_array(state_vector_list=np.array(py_state_vector_list),
                                             label_list=list(appliance_names),
                                             true_apparent_power=data_point_array[:,-2])
            py_fig.savefig("/home/thomas/projects/nilm_at_fair/training_data/model_validation/"
                           f"py_model_validation_{folder_name}_plot.pdf")
