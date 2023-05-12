import argparse
import sys

import numpy as np
import matplotlib.pyplot as plt

sys.path.append("../../../")
from src.pulsed_power_ml.model_framework.data_io import load_binary_data_array
from src.pulsed_power_ml.model_framework.training_data_labelling import get_features_from_raw_data
from src.pulsed_power_ml.model_framework.data_io import read_parameters


def main():

    parser = argparse.ArgumentParser(description='CLI tool to produce training files from raw data')
    parser.add_argument('--input',
                        '-i',
                        help='Path to the input binary containing the raw data.',
                        required=True)
    parser.add_argument('--output-folder',
                        '-o',
                        help='Path to the output folder',
                        required=True)
    parser.add_argument('--parameter-file',
                        '-p',
                        help='Path to the parameter file',
                        required=True)
    parser.add_argument('--prefix',
                        help='Prefix for the output files',
                        default='')
    args = parser.parse_args()

    # Read parameter file
    parameter_dict = read_parameters(args.parameter_file)

    # Load data points
    data_point_array = load_binary_data_array(args.input,
                                              fft_size_data_point=parameter_dict['fft_size_real'])

    # Produce features
    features_array, switch_positions = get_features_from_raw_data(data_point_array, parameter_dict)

    # write features to file
    output_file_name = f'{args.output_folder}/{args.prefix}_features.csv'
    np.savetxt(fname=output_file_name,
               X=features_array,
               delimiter=',')

    switch_positions_output_file_name = f'{args.output_folder}/{args.prefix}_switch_positions.csv'
    np.savetxt(fname=switch_positions_output_file_name,
               X=switch_positions,
               delimiter=',')

    # Produce plots
    fig = plt.figure(figsize=(16, 9))
    ax = fig.add_subplot()
    apparent_power_array = data_point_array[:, -2]
    ax.plot(apparent_power_array,
            label='Apparent Power')
    ax.plot(switch_positions * max(apparent_power_array),
            label='Detected Switching Events',
            alpha=0.5)
    ax.set_xlabel('Time [a.u.]')
    ax.set_ylabel('[VA]')
    ax.grid(True)
    ax.legend()
    ax.set_title(f'Apparent Power & Switch Positions for {args.prefix}')

    # Add numbers to switching events
    for i, s in enumerate(np.nonzero(switch_positions)[0]):
        y = -1.25 if i % 2 != 0 else -0.5
        ax.text(x=s,
                y=y,
                s=i)

    figure_output_file_name = f'{args.output_folder}/{args.prefix}_switch_positions.pdf'
    fig.savefig(figure_output_file_name)

    return


if __name__ == '__main__':
    main()
