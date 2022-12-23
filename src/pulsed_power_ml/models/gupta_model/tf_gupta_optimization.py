"""
This module implement functions to help eliminate shortcomings of the tf_gupta_model and to enhance its performance in
a productive scenario.
"""
from typing import List

from tqdm import tqdm
import tensorflow as tf
import pandas as pd
import numpy as np

from src.pulsed_power_ml.models.gupta_model.tf_gupta_clf import TFGuptaClassifier

from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.model_framework.visualizations import make_eval_plot
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base


# Some constants
BASE_FOLDER = "/home/thomas/projects/fair/call_3/data/"
FEATURES_FILE_PATH = f"{BASE_FOLDER}/training_data/labels_20221202_9peaks/Features_ApparentPower_0.7_p.csv"
LABELS_FILE_PATH = f"{BASE_FOLDER}/training_data/labels_20221202_9peaks/Labels_ApparentPower_0.7_p.csv"
RAW_DATA_FOLDER = f"{BASE_FOLDER}/raw_data/2022-11-16_training_data/"

APPARENT_POWER_LIST_PATH = "src/pulsed_power_ml/models/gupta_model/apparent_power_data_base.yml"
PARAMETER_FILE_PATH = "src/pulsed_power_ml/models/gupta_model/parameters.yml"

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
    features = tf.constant(pd.read_csv(FEATURES_FILE_PATH).values, dtype=tf.float32)
    labels = tf.constant(pd.read_csv(LABELS_FILE_PATH).values, dtype=tf.float32)

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


def get_raw_data(name: str) -> List[tf.Tensor]:
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

    return list_of_data_point_tensors


def main():

    # load raw data
    raw_data_list = get_raw_data("r1")

    # instantiate model
    model = instantiate_model()

    # reset model
    reset_model(model=model, data_point=raw_data_list[0])

    # Apply model to data
    start_frame = 0
    stop_frame = 100
    state_vector_list = list()
    for data_point in tqdm(raw_data_list[start_frame:stop_frame], desc="Applying model"):
        state_vector = model(data_point)
        state_vector_list.append(state_vector)


    # Make plot
    fig = make_eval_plot(power_array=np.array(raw_data_list)[start_frame:stop_frame, -2],
                         state_array=np.array(state_vector_list)[:,10])

    # Save figure
    fig.savefig(f"{OUTPUT_PATH}/tf_gupta_optimization.pdf")
    return

if __name__ == "__main__":
    main()
