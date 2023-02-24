"""
This module contains various functions to produce helpful visualizations
"""

from typing import Union, List

import numpy as np
import matplotlib.pyplot as plt
import matplotlib

import sys
sys.path.append("../../../")

from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.models.gupta_model.gupta_utils import gupta_offline_switch_detection

def make_eval_plot(power_array: Union[np.array, List],
                   state_array: Union[np.array, List],
                   appliance_power: float = 1) -> matplotlib.figure.Figure:
    """
    This function creates a single plot showing the raw data and the state of one appliance versus time step

    Parameters
    ----------
    power_array
        Array of power values
    state_array
        Array containing zeros (off) and ones (on), indicating the state of the appliance.
    appliance_power
        Data base power value of the appliances

    Returns
    -------
    fig
        Figure containing the plot
    """
    fig = plt.Figure(figsize=(8, 4.5))
    ax = fig.add_subplot()
    ax.plot(power_array,
            label="Measured Power")
    ax.plot(state_array * appliance_power,
            label="Predicted Power")

    return fig

def plot_state_vector_array(state_vector_list: np.array,
                            label_list: Union[List[str], None] = None,
                            true_apparent_power: Union[np.array, None] = None,
                            v_line: Union[float, None] = None) -> matplotlib.figure.Figure:
    """
    Function to make one subplot for all appliances known to the model.

    Parameters
    ----------
    state_vector_list
        Array with state_vectors. Shape = (n_vectors, n_appliances)
    label_list
        List with names for each device in state vector.
    true_apparent_power
        Array w/ true, total apparent power values (S from raw data).
    v_line:
        Plot a vertical line in all plots to indicate which part of the data is unseen by the model.

    Returns
    -------
    Figure containing one plot per appliance
    """
    n_figures = state_vector_list.shape[1]
    fig = plt.figure(figsize=(16, 4.5 * n_figures), tight_layout=True)

    if label_list is not None:
        label_list.append("Other")

    for i in range(n_figures):
        ax = fig.add_subplot(n_figures, 1, i+1)
        ax.plot(state_vector_list[:,i],
                "-",
                label='Predicted')

        if true_apparent_power is not None:
            ax.plot(true_apparent_power,
                    "--",
                    label='Measured Apparent Power')

        if label_list is not None:
            ax.set_title(label_list[i])

        if v_line is not None:
            ax.axvline(x=int(len(state_vector_list) * v_line),
                       label="Training Data | Test Data",
                       color="C2",
                       linestyle=':',
                       linewidth=3)

        ax.grid(True)
        ax.set_ylabel("Apparent Power [VA]")
        ax.set_xlabel("Time [au]")
        ax.legend(loc='upper left')

    return fig

def plot_data_point_array(list_of_data_points: Union[List, np.array],
                          fft_size: int,
                          list_of_state_vectors: Union[List, np.array, None] = None,
                          plot_spectra: bool = False) -> matplotlib.figure.Figure:
    """
    Add a contour plot for all three spectra, one
    Parameters
    ----------
    list_of_data_points
        Array of data points.
    fft_size
        (Full) size of the FFT.
    list_of_state_vectors
        If provided add a plot showing the predicted power disaggregation.
    plot_spectra
        If True, add spectra to plot.

    Returns
    -------
    Figure
    """
    # Create figure and axis objects
    fig = plt.figure(figsize=(16, 45 / 2),
                     dpi=400,
                     layout="tight")

    if list_of_state_vectors is None:
        n_fig = 5
    else:
        n_fig = 6

    fft_u_ax = fig.add_subplot(n_fig, 1, 1)
    fft_i_ax = fig.add_subplot(n_fig, 1, 2)
    fft_s_ax = fig.add_subplot(n_fig, 1, 3)
    pqs_ax = fig.add_subplot(n_fig, 1, 4)
    phi_ax = fig.add_subplot(n_fig, 1, 5)

    if plot_spectra:
    # Add spectrum plots
        min_max_freq = [0, 1_000]
        spectrum_size = int(fft_size/2)
        fft_u_ax = add_contour_plot(spectrum=list_of_data_points[:, 0:spectrum_size],
                                    min_max_freq=min_max_freq,
                                    ax=fft_u_ax)
        fft_u_ax.set_title('Voltage Spectrum')

        fft_i_ax = add_contour_plot(spectrum=list_of_data_points[:, spectrum_size:2*spectrum_size],
                                    min_max_freq=min_max_freq,
                                    ax=fft_i_ax)
        fft_i_ax.set_title('Current Spectrum')

        fft_s_ax = add_contour_plot(spectrum=list_of_data_points[:, 2*spectrum_size:3*spectrum_size],
                                    min_max_freq=min_max_freq,
                                    ax=fft_s_ax)
        fft_s_ax.set_title('Apparent Power Spectrum')

    # add PQS plot
    pqs_ax = add_pqs_plot(list_of_data_points[:, -4:-1],
                          ax=pqs_ax)
    pqs_ax.set_title("Apparent, Active and Reactive Power")

    # add Phi plot
    phi_ax = add_phi_plot(list_of_data_points[:, -1],
                          ax=phi_ax)
    phi_ax.set_title("Phase Angle")

    # add prediction plot
    if list_of_state_vectors is not None:
        pqs_predicted_ax = fig.add_subplot(n_fig, 1, 6)
        pqs_predicted_ax = add_prediction_plot(state_vector_array=np.array(list_of_state_vectors),
                                               pqs_array=list_of_data_points[:, -4:-1],
                                               ax=pqs_predicted_ax)

    return fig


def get_frequencies_from_spectrum(spectrum: np.array,
                                  sample_rate: int) -> np.array:
    """
    Returns an array containing the central frequencies per bin.
    Parameters
    ----------
    spectrum
        Array containing the magnitudes.
    sample_rate
        Sampel rate of the DAQ.

    Returns
    -------
    frequencies
        Array with the same length as **spectrum** containing the central frequency for each bin in **spectrum**.
    """
    freq_per_bin = sample_rate / len(spectrum)
    frequencies = np.arange(freq_per_bin / 2, sample_rate, freq_per_bin)
    return frequencies


def add_contour_plot(spectrum: np.array,
                     min_max_freq: List[float],
                     ax: matplotlib.axis.Axis) -> matplotlib.axis.Axis:
    """
    Add a contour plot of the provided spectrum to *axes*.

    Parameters
    ----------
    spectrum
        Array containing the magnitude per frequency.
    min_max_freq:
        Array containing the min frequency and the max frequency.
    ax
        Axis object to add the plot to.

    Returns
    -------
    ax
        the provided axis object with the respective plot added.
    """

    # add spectrum to axis
    ax.imshow(X=np.log10(spectrum.T),
              aspect="auto",
              interpolation="none",
              origin="lower",
              cmap=matplotlib.colormaps["rainbow"])

    # add frequency ticks to y-axis
    y_labels = np.arange(min_max_freq[0], min_max_freq[1]+1, 250)
    y_ticks_position_list = np.linspace(0, len(spectrum[0]), len(y_labels))
    ax.set_yticks(ticks=y_ticks_position_list,
                  labels=y_labels)
    ax.set_ylabel("Frequency [kHz]")

    # add time ticks
    ax.set_xlabel("Timestep")

    # Miscellaneous
    ax.grid(True)

    return ax


def add_pqs_plot(pqs_array: np.array,
                 ax: matplotlib.axis.Axis) -> matplotlib.axis.Axis:
    """
    Add a P, Q, S versus time plot to the provided axis.

    Parameters
    ----------
    pqs_array
        array with shape (n_timesteps, 3), whereas the second dimension contains the values of P, Q, S for each
        timestep.
    ax
        Axis object to add the plot to.

    Returns
    -------
    ax
        the provided axis with the respective plot added.
    """

    # Add three line plots
    ax.plot(pqs_array[:, 0],
            label="P")
    ax.plot(pqs_array[:, 1],
            label="Q")
    ax.plot(pqs_array[:, 2],
            label="S")

    ax.set_xlabel("Timestep")
    ax.set_ylabel("P[W], Q[VAr], S[VA]")
    ax.grid(True)
    ax.legend()
    ax.set_xlim(0, pqs_array.shape[0])

    return ax


def add_phi_plot(phi_array: np.array,
                 ax: matplotlib.axis.Axis) -> matplotlib.axis.Axis:
    """
    Add a Phi versus time plot to the provided axis.

    Parameters
    ----------
    phi_array
        array with shape (n_timesteps, 1), whereas the second dimension contains the values of PHI for each timestep.
    ax
        Axis object to add the plot to.

    Returns
    -------
    ax
        the provided axis with the respective plot added.
    """

    # Add three line plots
    ax.plot(phi_array,
            label="Phi")

    ax.set_xlabel("Timestep")
    ax.set_ylabel("Phase Angle [Deg]")
    ax.grid(True)
    ax.legend()
    ax.set_xlim(0, len(phi_array))

    return ax


def add_prediction_plot(state_vector_array: np.array,
                        pqs_array: np.array,
                        ax: matplotlib.axis.Axis) -> matplotlib.axis.Axis:
    """
    Add a plot containing all predicted appliances incl. the respective predicted values and the actual apparent
    power versus time.

    Parameters
    ----------
    state_vector_array
        Array of state vectors.
    pqs_array
        Array of P, Q ans S values.
    ax
        Axis object to add the plot to.

    Returns
    -------
    ax
        the provided axis with the respective plot added.
    """
    # Add the total apparent power
    ax.plot(pqs_array[:, 2],
            label="Total measured apparent power")

    # Add one line for each appliance and "other"
    n_appliances = state_vector_array.shape[1]  # incl. "other"
    for appliance_index in range(n_appliances):
        ax.plot(state_vector_array[:, appliance_index],
                label=f"Appliance {appliance_index}")

    ax.set_xlabel("Timestep")
    ax.set_ylabel("Apparent Power[VA]")
    ax.grid(True)
    ax.legend()
    ax.set_xlim(0, state_vector_array.shape[0])

    return ax

def make_gupta_switch_detection_plot(path_to_data_folder: str,
                                     window_size: int = 25,
                                     threshold: float = 2000,
                                     log_scale: bool = False,
                                     fft_size: int = 2**17,
                                     add_labels: bool = True) -> matplotlib.figure.Figure:
    """
    Returns a figure containing one plot showing the apparent power versus time and the detected
    switching events.

    Parameters
    ----------
    path_to_data_folder
        Path to folder containing the raw data in binary format.
    window_size
        Number of frames for background and signal in switch detection algorithm
    threshold
        Switch detection threshold
    log_scale
        If True, convert difference spectrum to dBm scale before applying threshold.
    fft_size
        Full FFT size.
    add_labels
        If True, add numbers to each detected switching event.

    Returns
    -------
    fig
        An instance of figure containing the respective plot.
    """

    # Load data
    data_point_array = read_training_files(path_to_folder=path_to_data_folder,
                                           fft_size=fft_size)

    # Apply switch detection algorithm
    switch_array = gupta_offline_switch_detection(
        data_point_array=data_point_array,
        window_size=window_size,
        threshold=threshold,
        log_scale=log_scale
    )

    switch_positions = np.argwhere(switch_array==1).reshape((-1,))
    dead_time_positions = np.argwhere(switch_array==-1).reshape((-1,))

    # Create figure
    fig = plt.figure(figsize=(16, 9), layout='tight')
    ax = fig.add_subplot()

    # Plot s
    ax.plot(
        data_point_array[:, -2],
        label="Apparent Power"
    )

    y_max = max(data_point_array[:, -2])

    # Plots detected switches
    ax.vlines(
        x=switch_positions,
        ymin=0,
        ymax=y_max,
        linewidth=1,
        color='C1',
        alpha=0.5,
        label='Detected Switch'
    )

    # Plot dead time
    ax.vlines(
        x=dead_time_positions,
        ymin=0,
        ymax=y_max,
        linewidth=1,
        color='C3',
        alpha=0.5,
        label='Dead time'
    )

    # Add numbers to switching events
    for i, s in enumerate(switch_positions):
        y = -1.25 if i % 2 != 0 else -0.5
        ax.text(x=s,
                y=y,
                s=i)

    # Some cosmetics
    ax.legend()
    ax.grid(True)
    ax.set_title("Switch Detection - Gupta Approach")
    ax.set_xlabel("Time [au]")
    ax.set_ylabel("S [VA]")

    return fig





