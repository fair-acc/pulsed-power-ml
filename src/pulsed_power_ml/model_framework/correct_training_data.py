"""
A simple command line tool to remove false positive switching events from features.csv and switch_positions.csv
"""

import sys

sys.path.append("../../../")

import argparse

import numpy as np
import matplotlib.pyplot as plt

from training_data_labelling import remove_false_positive_switching_events
from data_io import load_binary_data_array


def main():

    parser = argparse.ArgumentParser(
        description=('Command line tool to remove false positive switching events from a features file and the '
                     'corresponding switch_positions file')
    )
    parser.add_argument('-r',
                        '--raw-data',
                        help='Path to raw data',
                        required=True)
    parser.add_argument('-f',
                        '--feature-file',
                        help='Path to the feature csv-file',
                        required=True)
    parser.add_argument('-s',
                        '--switch-positions',
                        help='Path to the switch positions csv-file',
                        required=True)
    parser.add_argument('-o',
                        '--output-folder',
                        help='Path to the output folder',
                        required=True)
    parser.add_argument('--fft-size',
                        help='FFT size (full)',
                        type=int,
                        default=2**17)
    parser.add_argument('--prefix',
                        help='Prefix for the output files',
                        default='corrected')

    args = parser.parse_args()

    # Load training files
    data_point_array = load_binary_data_array(args.raw_data, int(args.fft_size / 2))
    apparent_power_array = data_point_array[:, -2]

    # Load feature file
    features = np.loadtxt(fname=args.feature_file,
                          delimiter=',')

    # Load switch positions file
    switch_positions = np.loadtxt(fname=args.switch_positions,
                                  delimiter=',')

    # Make plot with original switch positions and apparent power
    fig = plt.figure(figsize=(16, 9))
    ax = fig.add_subplot()
    ax.grid(True)
    ax.plot(apparent_power_array,
            label='Apparent Power')
    ax.plot(switch_positions * max(apparent_power_array),
            label='Original Switch Positions',
            alpha=0.5)

    original_switch_indices = np.nonzero(switch_positions)[0]

    for i, s in enumerate(original_switch_indices):
        y = -1.125 if i % 2 != 0 else -0.5
        ax.text(x=s,
                y=y,
                s=i)

    ax.legend()
    original_switch_pos_file_name = f'{args.output_folder}/{args.prefix}_original_switch_position.pdf'
    print(f'Store plot with original switch positions in {original_switch_pos_file_name}.')
    fig.savefig(original_switch_pos_file_name)

    # Get input from user which switching events should be removed
    correct_user_input = False
    input_int = None
    while not correct_user_input:
        print('\n')
        print(f'Which switching events should be removed?')
        print(f'Input indices of switches seperated with a whitespace!')
        user_input = input()

        if len(user_input) == 0:
            print(f'No input provided! Exit!')
            exit()

        try:
            input_int = [int(x) for x in user_input.split(' ') if len(x) != 0]
            correct_user_input = True
        except ValueError:
            print('Input could not be converted to list of ints! Use only ints seperated with whitespaces!')

    # Remove incorrect switches
    corrected_features, corrected_switch_positions = remove_false_positive_switching_events(
        features,
        switch_positions,
        input_int
    )

    # Make plot with corrected switches
    fig = plt.figure(figsize=(16, 9))
    ax = fig.add_subplot()
    ax.grid(True)
    ax.plot(apparent_power_array,
            label='Apparent Power')
    ax.plot(corrected_switch_positions * max(apparent_power_array),
            label='Corrected Switch Positions',
            alpha=0.5,
            color='C2')
    ax.legend()

    corrected_switch_pos_file_name = f'{args.output_folder}/{args.prefix}_corrected_switch_position.pdf'
    print(f'Store plot with corrected switch positions in {corrected_switch_pos_file_name}.')
    fig.savefig(corrected_switch_pos_file_name)

    # Write corrected features and csv to disk
    corrected_features_file_name = f'{args.output_folder}/{args.prefix}_corrected_features.csv'
    print(f'Write corrected features in {corrected_features_file_name}')
    np.savetxt(fname=corrected_features_file_name,
               X=corrected_features,
               delimiter=',')

    corrected_switch_positions_file_name = f'{args.output_folder}/{args.prefix}_corrected_switch_positions.csv'
    print(f'Write corrected switch positions in {corrected_switch_positions_file_name}')
    np.savetxt(fname=corrected_switch_positions_file_name,
               X=corrected_switch_positions,
               delimiter=',')

    exit()


if __name__ == '__main__':
    main()
