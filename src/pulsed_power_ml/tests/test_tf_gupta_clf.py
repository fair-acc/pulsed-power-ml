"""
This module contains test for the TensorFlow Version of the Gupta Model.
"""
import pytest
import pandas as pd
import tensorflow as tf
import numpy as np

from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters
from src.pulsed_power_ml.models.gupta_model.tf_gupta_clf import TFGuptaClassifier


# Some constants (not sure how to include this in automatic testing...)
BASE_FOLDER = "/home/thomas/projects/fair/call_3/data/"
FEATURES_FILE_PATH = f"{BASE_FOLDER}/training_data/labels_20221202_9peaks/Features_ApparentPower_0.7_p.csv"
LABELS_FILE_PATH = f"{BASE_FOLDER}/training_data/labels_20221202_9peaks/Labels_ApparentPower_0.7_p.csv"
RAW_DATA_FOLDER = f"{BASE_FOLDER}/raw_data/2022-11-16_training_data/h1/"

APPARENT_POWER_LIST_PATH = "src/pulsed_power_ml/models/gupta_model/apparent_power_data_base.yml"
PARAMETER_FILE_PATH = "src/pulsed_power_ml/models/gupta_model/parameters.yml"

@pytest.fixture
def features():
    return tf.constant(pd.read_csv(FEATURES_FILE_PATH).values, dtype=tf.float32)

@pytest.fixture
def labels():
    return tf.constant(pd.read_csv(LABELS_FILE_PATH).values, dtype=tf.float32)

@pytest.fixture
def apparent_power_list():
    power_list_with_names = read_power_data_base(APPARENT_POWER_LIST_PATH)
    _, power_list = list(zip(*power_list_with_names))
    return power_list

@pytest.fixture
def parameter_dict():
    return read_parameters(PARAMETER_FILE_PATH)

@pytest.fixture
def data_point_array(parameter_dict):
    return read_training_files(RAW_DATA_FOLDER,
                               parameter_dict["fft_size"])

@pytest.fixture
def tf_gupta_model(features,
                   labels,
                   apparent_power_list,
                   parameter_dict):
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

class TestTFGuptaClassifier:

    def test_save_load_model_equality(self, tf_gupta_model, data_point_array, tmp_path):

        original_state_vector_list = list()
        for data_point in data_point_array:
            original_state_vector = tf_gupta_model(tf.constant(data_point, dtype=tf.float32))
            original_state_vector_list.append(original_state_vector)

        tf.saved_model.save(tf_gupta_model, tmp_path / "test_gupta_original_model")

        loaded_model = tf.saved_model.load(tmp_path / "test_gupta_original_model")

        loaded_state_vector_list = list()
        for data_point in data_point_array:
            loaded_state_vector = loaded_model(tf.constant(data_point, dtype=tf.float32))
            loaded_state_vector_list.append(loaded_state_vector)

        assert np.allclose(np.array(original_state_vector_list),
                           np.array(loaded_state_vector_list),
                           rtol=0.1,
                           atol=10)
