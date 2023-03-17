"""
This module contains tests for the visualization module.
"""
import copy

import pytest
import numpy as np
import matplotlib.pyplot as plt

from src.pulsed_power_ml.model_framework.visualizations import add_pca_plot

FEATURE_VECTOR_LENGTH = 100
N_DATA_POINTS = 100
N_CLASSES = 10


@pytest.fixture
def random_feature_array(n_data_points=N_DATA_POINTS, feature_vector_length=FEATURE_VECTOR_LENGTH):
    return np.random.rand(n_data_points, feature_vector_length)


@pytest.fixture
def random_label_array(n_classes=N_CLASSES, n_data_points=N_DATA_POINTS):
    return np.random.randint(0, n_classes, n_data_points)


@pytest.fixture
def random_feature_array_with_nan(random_feature_array):
    nan_feature_array = copy.deepcopy(random_feature_array)
    nan_feature_array[42, 42] = np.nan
    return nan_feature_array


@pytest.fixture
def empty_axis():
    fig = plt.figure()
    ax = fig.add_subplot(111)
    return ax


class TestAddPcaPlot:

    def test_for_nan_check(self, empty_axis, random_feature_array_with_nan, random_label_array):
        # Should raise Assertion error if features contain nans
        with pytest.raises(AssertionError, match=r'nan'):
            add_pca_plot(empty_axis, random_feature_array_with_nan, random_label_array)

    def test_for_same_length(self, empty_axis, random_feature_array, random_label_array):
        # Should raise Assertion error if length of features and labels does not match
        with pytest.raises(AssertionError, match=r'length'):
            add_pca_plot(empty_axis, random_feature_array, random_label_array[:-1])

    def test_normal_behavior(self, empty_axis, random_feature_array, random_label_array):
        try:
            _ = add_pca_plot(empty_axis, random_feature_array, random_label_array)
        except:
            pytest.fail("Normal behavior failed!")
