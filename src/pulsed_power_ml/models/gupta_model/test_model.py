"""
This module implements an easy method to test the performance of a Gupta model.
"""

import argparse
import sys
import os

import tensorflow as tf
import numpy as np
from tqdm import tqdm

sys.path.append("../../../../")

from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters
from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base
from src.pulsed_power_ml.model_framework.visualizations import plot_state_vector_array
from src.pulsed_power_ml.model_framework.visualizations import plot_data_point_array


def main():

    # Parser CLI arguments
    parser = argparse.ArgumentParser(
        description='This script provides an easy and quick way to test a NILM model given previously recorded data.'
    )
    parser.add_argument('-i',
                        '--input-folder',
                        help='Path to folder containing raw data.',
                        required=True)
    parser.add_argument('-m',
                        '--model',
                        help='Path to TensorFlow model.',
                        required=True)
    parser.add_argument('-o',
                        '--output-folder',
                        help='Path to folder where results should be stored.',
                        required=True)
    parser.add_argument('-p',
                        '--parameters',
                        help='Path to parameter file.',
                        required=True)
    parser.add_argument('-d',
                        '--power-data-base',
                        help='Path to data base containing apparent power for each known appliance.',
                        required=True)
    parser.add_argument('-v',
                        '--vertical-line',
                        help='Pass this flag to include a vertical line into prediction plot at 70%% of time.',
                        action='store_true',
                        default=False)
    parser.add_argument('--new-flowgraph',
                        help='Use alternative method to read raw files from disk.',
                        action='store_true',
                        default=False)
    parser.add_argument('--input-filename',
                        help='Name of the input file (no path). Required if "--new-flowgraph" option is used.',
                        type=str)
    args = parser.parse_args()

    # Load parameters
    print(f'\nRead parameters from {args.parameters}')
    parameter_dict = read_parameters(args.parameters)

    # Load data base for apparent power values
    print(f'\nRead apparent power values from {args.power_data_base}')
    labels, apparent_power_list = list(zip(*read_power_data_base(args.power_data_base)))

    # Load model
    print(f'\nLoad model {args.model}')
    model = tf.saved_model.load(args.model)

    # Load data
    print(f'\nLoad data in {args.input_folder}')
    if args.new_flowgraph:
        if args.input_filename is None:
            print(f'ERROR: "input-filename" must be provided if "--new-flowgraph" flag is set!')
            exit(-1)
        data_point_array = np.fromfile(
            file=f'{args.input_folder}/{args.input_filename}',
            dtype=np.float32)\
            .reshape((-1, 3 * int(parameter_dict['fft_size_real']) + 4))

    else:
        data_point_array = read_training_files(path_to_folder=args.input_folder,
                                               fft_size=parameter_dict["fft_size"])

    # Apply model
    state_vector_list = list()
    for data_point in tqdm(data_point_array, 'Apply model...'):
        state_vector = model(tf.constant(data_point, dtype=tf.float32))
        state_vector_list.append(state_vector)

    # Make plot of predictions
    print(f'\nStore plots in {args.output_folder}')
    if args.vertical_line:
        v_line_percentage = 0.7
    else:
        v_line_percentage = None
    prediction_figure = plot_state_vector_array(state_vector_list=np.array(state_vector_list),
                                                label_list=list(labels),
                                                true_apparent_power=data_point_array[:, -2],
                                                v_line=v_line_percentage)
    prediction_figure.savefig(f'{args.output_folder}/prediction_figure.pdf')

    # Make plot of raw data
    raw_data_figure = plot_data_point_array(list_of_data_points=data_point_array,
                                            fft_size=parameter_dict["fft_size"],
                                            plot_spectra=True)
    raw_data_figure.savefig(f'{args.output_folder}/raw_data.png', dpi=300)


if __name__ == '__main__':
    main()
