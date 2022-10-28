"""
This module contains various functions to produce helpful visualizations
"""

from typing import Union, List

import numpy as np
import matplotlib.pyplot as plt
import matplotlib


def plot_data_point_array(list_of_data_points: Union[list, np.array],
                          fft_size: int) -> matplotlib.figure.Figure:
    """
    Add a contour plot for all three spectra, one
    Parameters
    ----------
    list_of_data_points
        Array of data points.
    fft_size
        (Full) size of the FFT.
    Returns
    -------

    """
    # Create figure and axis objects
    fig = plt.figure(figsize=(16, 45 / 2),
                     dpi=400,
                     layout="tight")
    fft_u_ax = fig.add_subplot(5, 1, 1)
    fft_i_ax = fig.add_subplot(5, 1, 2)
    fft_s_ax = fig.add_subplot(5, 1, 3)
    pqs_ax = fig.add_subplot(5, 1, 4)
    phi_ax = fig.add_subplot(5, 1, 5)

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
    ax.imshow(X=spectrum.T,
              aspect="auto",
              interpolation="none",
              origin="lower")

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
