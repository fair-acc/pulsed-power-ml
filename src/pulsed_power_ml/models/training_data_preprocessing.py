"""
This module contains functions for the further processing of data in order to use them for training.
"""

import pandas as pd
import numpy as np


def split_sequence(sequence: np.array,
                   window_len: int = 200,
                   n_label_cols: int = 3,
                   stride: int = 1) -> (np.array, np.array):
    """
    Split a sequence along its first axis into sequences of length *window_len*, moving along with a stride of 1.

    Parameters
    ----------
    sequence
        Array with shape [n, m].

    window_len
        How many time steps should be contained in one window

    n_label_cols
        last *n_label_cols* columns should be used as labels. Only last row of a windows will written to y.

    stride
        How many time steps should be moved forward from one window to the next.

    Returns
    -------
    X, y
        X with shape [k, window_len, m - n_label_cols]
        y with shape [k, n_label_cols]
    """
    len_sequence = sequence.shape[0]
    n_features = sequence.shape[1] - n_label_cols

    X_list = list()
    y_list = list()

    # iterate over sequence
    for i in range(0, len_sequence, stride):
        end_ix = i + window_len

        # Check if end of next window is still within sequence
        if end_ix > len(sequence) - 1:
            break

        # Define next datapoint and label
        seq_x = sequence[i:end_ix, 0:n_features]
        seq_y = sequence[end_ix, n_features:]

        # Append to lists
        X_list.append(seq_x)
        y_list.append(seq_y)

    # Convert to arrays and return
    X = np.array(X_list)
    y = np.array(y_list)

    return X, y

