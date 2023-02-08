import argparse
import sys

import numpy as np
import matplotlib.pyplot as plt

sys.path.append("../../../")
from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.model_framework.training_data_labelling import get_features_from_raw_data
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters


def main():

    parser = argparse.ArgumentParser(description='CLI tool to produce training files from raw data')
    parser.add_argument('--input-folder',
                        '-i',
                        help='Path to the input folder',
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
    data_point_array = read_training_files(args.input_folder, fft_size=parameter_dict['fft_size'])

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

    figure_output_file_name = f'{args.output_folder}/{args.prefix}_switch_positions.pdf'
    fig.savefig(figure_output_file_name)

    return

if __name__ == '__main__':
    print(sys.path)
    main()
