"""
This module contains the definition of a 1D convolutional deep neural network.
"""

import tensorflow as tf


def create_1dconv_model(n_steps: int,
                        n_features: int,
                        n_appliances: int,
                        n_filters: int = 64,
                        kernel_size: int = 21,
                        pool_size: int = 4) -> tf.keras.Model:
    """
    This function returns a compiled model in order to disaggregate the true power of all components.

    Parameters
    ----------
    n_steps : int
        Number of time steps, which should be considered for one prediction.
    n_features : int
        Number of features (e.g. current, voltage, phase_shift, power, ...).
    n_appliances : int
        How many appliances should be monitored.
    n_filters : int
        Number of filters used in the convolutional layers.
    kernel_size : int
        Determines the size of the kernels used in the convolutional layers.
    pool_size : int
        Size of the max pooling window
    Returns
    -------
    model : tf.keras.Model
        A compiled model with input shape (n_steps, n_features) and output shape (n_appliances)
    """

    # Define model architecture
    inputs = tf.keras.Input(shape=(n_steps, n_features))
    h_0 = tf.keras.layers.Conv1D(filters=n_filters, kernel_size=kernel_size, activation='relu')(inputs)
    h_1 = tf.keras.layers.Conv1D(filters=n_filters, kernel_size=kernel_size, activation='relu')(h_0)
    h_2 = tf.keras.layers.Dropout(0.25)(h_1)
    h_3 = tf.keras.layers.MaxPooling1D(pool_size=pool_size)(h_2)
    h_4 = tf.keras.layers.Flatten()(h_3)
    h_5 = tf.keras.layers.Dense(128, activation='relu')(h_4)
    outputs = tf.keras.layers.Dense(n_appliances)(h_5)

    model = tf.keras.Model(inputs=inputs, outputs=outputs, name='1D Conv Model')

    # Compile
    model.compile(
        loss='mse',
        optimizer='adam'
    )

    return model


if __name__ == "__main__":
    test_model = create_1dconv_model(
        n_steps=100,
        n_features=8,
        n_appliances=4,
    )

    test_model.summary()
