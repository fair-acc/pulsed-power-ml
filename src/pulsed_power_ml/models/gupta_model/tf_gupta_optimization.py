"""
This module implement functions to help eliminate shortcomings of the tf_gupta_model and to enhance its performance in
a productive scenario.
"""
from typing import List, Tuple

from tqdm import tqdm
import tensorflow as tf
import pandas as pd
import numpy as np

import matplotlib.pyplot as plt

from src.pulsed_power_ml.models.gupta_model.tf_gupta_clf import TFGuptaClassifier

from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.model_framework.visualizations import make_eval_plot
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base

from src.pulsed_power_ml.model_framework.training_data_labelling import trainingdata_switch_detector


# Some constants
BASE_FOLDER = "/home/thomas/projects/fair/call_3/data/"
FEATURES_FILE_PATH = f"{BASE_FOLDER}/training_data/one_class_per_appliance_training_data/100_0_split/features.csv"
LABELS_FILE_PATH = f"{BASE_FOLDER}/training_data/one_class_per_appliance_training_data/100_0_split/labels.csv"
RAW_DATA_FOLDER = f"{BASE_FOLDER}/raw_data/2022-11-16_training_data/"

APPARENT_POWER_LIST_PATH = "src/pulsed_power_ml/models/gupta_model/apparent_power_data_base_appliance_type.yml"
PARAMETER_FILE_PATH = "src/pulsed_power_ml/models/gupta_model/parameters_appliance_types.yml"

OUTPUT_PATH = "src/assets/pulsed_power_ml/model_evaluation/gupta_model/model_optimization_intermediate_results/"


def reset_model(model: tf.keras.Model, data_point: tf.Tensor) -> tf.keras.Model:
    """
    Reset the Gupta model

    Parameters
    ----------
    model
        Instance of a tf gupta model
    data_point
        Example of an input tensor to get the correct shape.

    Returns
    -------
        Reset Version of the gupta model
    """
    reset_tensor = tf.math.multiply(
        -1.0,
        tf.ones_like(
            tf.constant(
                data_point,
                dtype=tf.float32),
            dtype=tf.float32
        )
    )

    _ = model(reset_tensor)

    return model


def instantiate_model():
    """
    Create a trained instance of the tf_gupta_model for optimization purposes.

    Returns
    -------
    model
        A trained and instance of the TF gupta model.
    """
    # Load the parameter dict
    parameter_dict = read_parameters(PARAMETER_FILE_PATH)

    # Load training features and labels
    features = tf.constant(pd.read_csv(FEATURES_FILE_PATH, header=None).values, dtype=tf.float32)
    labels = tf.constant(pd.read_csv(LABELS_FILE_PATH, header=None).values, dtype=tf.float32)

    # Load the apparent power list
    power_list_with_names = read_power_data_base(APPARENT_POWER_LIST_PATH)
    _, apparent_power_list = list(zip(*power_list_with_names))

    # Instantiate the model
    model = TFGuptaClassifier(
        background_n=tf.constant(parameter_dict["background_n"], dtype=tf.int32),
        n_known_appliances=tf.constant(parameter_dict["n_known_appliances"], dtype=tf.int32),
        apparent_power_list=tf.constant(apparent_power_list, dtype=tf.float32),
        n_neighbors=tf.constant(parameter_dict["n_neighbors"], dtype=tf.int32),
        switching_offset=tf.constant(parameter_dict["switching_offset"], dtype=tf.int32),
        distance_threshold=tf.constant(parameter_dict["distance_threshold"], dtype=tf.float32),
        fft_size_real=tf.constant(parameter_dict["fft_size_real"], dtype=tf.int32),
        sample_rate=tf.constant(parameter_dict["sample_rate"], dtype=tf.int32),
        spectrum_type=tf.constant(parameter_dict["spectrum_type"], dtype=tf.int32),
        training_data_features=features,
        training_data_labels=labels
    )

    return model


def get_raw_data(name: str) -> Tuple[List[tf.Tensor], List]:
    """
    Load the raw data of a particular measurement series.

    Parameters
    ----------
    name
        Name of the measurement series (basically name of the respective folder

    Returns
    -------
    list_of_data_point_tensors
        List containing tensors of data points of the respective measurement series
    """
    parameter_dict = read_parameters(PARAMETER_FILE_PATH)
    data_point_array = read_training_files(path_to_folder=f"{RAW_DATA_FOLDER}/{name}",
                                           fft_size=parameter_dict["fft_size"])

    list_of_data_point_tensors = list()
    for data_point in tqdm(data_point_array, desc="Convert data points to tensors"):
        list_of_data_point_tensors.append(tf.constant(value=data_point, dtype=tf.float32))

    return list_of_data_point_tensors, data_point_array


def main():

    appliance = "r1"

    # Load parameters
    parameter_dict = read_parameters(PARAMETER_FILE_PATH)

    start_frame = 0
    stop_frame = -1

    # load raw data
    raw_data_list, data_point_array = get_raw_data(appliance)

    switch_positions = trainingdata_switch_detector(data_point_array[start_frame:stop_frame,2*2**16:3*2**16],
                                                    parameter_dict)

    del data_point_array

    # instantiate model
    model = instantiate_model()

    # reset model
    reset_model(model=model, data_point=raw_data_list[0])

    # Apply model to data

    state_vector_list = list()
    state_vector_sums = list()
    for data_point in tqdm(raw_data_list[start_frame:stop_frame], desc="Applying model"):
        state_vector = model(data_point)
        state_vector_list.append(state_vector)
        state_vector_sums.append(tf.reduce_sum(state_vector[:-1]))

    # Make plot
    fig = make_eval_plot(power_array=np.array(raw_data_list)[start_frame:stop_frame, -2],
                         state_array=np.array(state_vector_list)[:,6])

    fig.savefig(f"{OUTPUT_PATH}/knn_debugging_tf_gupta_{appliance}_r1_.pdf")

    del raw_data_list

    # calculate change of state vector (to crosscheck detection of switching events)
    model_switching_events = np.diff(state_vector_sums)
    fig_model_switches = plt.Figure()
    ax_model_switches = fig_model_switches.add_subplot()
    ax_model_switches.plot(model_switching_events)
    fig_model_switches.savefig(f"{OUTPUT_PATH}/knn_debugging_tf_gupta_model_switch_positions_{appliance}.pdf")

    # print(state_vector_list)
    # print(switch_positions)

    fig_2 = plt.Figure()
    ax = fig_2.add_subplot()
    ax.plot(switch_positions[0])
    ax.plot(switch_positions[1])
    fig_2.savefig(f"{OUTPUT_PATH}/knn_debugging_tf_gupta_check_switch_positions_{appliance}.pdf")

    # Save figure

    return

if __name__ == "__main__":
    main()
