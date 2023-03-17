"""
Simple script to make quick plots of recorded training data
"""
import argparse
import os

import sys
sys.path.append("../../../")

from src.pulsed_power_ml.model_framework.data_io import read_training_files
from src.pulsed_power_ml.model_framework.data_io import load_binary_data_array
from src.pulsed_power_ml.model_framework.visualizations import plot_data_point_array


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument("-i",
                        "--input",
                        help="Path to binary containing recorded data, or the input folder for old format",
                        required=True,
                        type=str)

    parser.add_argument("-o",
                        "--output-name",
                        help="Output file name",
                        required=True,
                        type=str)

    parser.add_argument("--fft-size",
                        help="Size of each spectrum in the data.",
                        type=int,
                        default=2**16)

    parser.add_argument('--plot-spectra',
                        help='Plot spectra if set to True. Default False.',
                        action='store_true',
                        default=False)

    args = parser.parse_args()

    # check if input is folder (old format) or single file (new format)
    if os.path.isfile(args.input):
        print(f'Loading file (new format) in {args.input}')
        data_point_array = load_binary_data_array(path_to_file=args.input,
                                                  fft_size_data_point=args.fft_size)

    elif os.path.isdir(args.input):
        print(f"Loading files in {args.input}")
        data_point_array = read_training_files(path_to_folder=args.input,
                                               fft_size=args.fft_size * 2)

    else:
        print(f'ERROR! No file or directory in {args.input}!')
        exit(-1)

    print(f"Plotting and storing plot in {args.output_name}")
    fig = plot_data_point_array(list_of_data_points=data_point_array,
                                fft_size=args.fft_size * 2,
                                plot_spectra=args.plot_spectra)

    fig.savefig(args.output_name, dpi=500)

    return


if __name__ == "__main__":
    main()
