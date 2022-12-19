"""
Simple script to make quick plots of recorded training data
"""
import argparse

from data_io import read_training_files
from visualizations import plot_data_point_array


if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("-i",
                        "--input-folder",
                        help="Path to folder containing recorded files",
                        required=True,
                        type=str)

    parser.add_argument("-o",
                        "--output-name",
                        help="Output file name",
                        required=True,
                        type=str)

    parser.add_argument("--fft-size",
                        help="Total size of the FFT",
                        type=int,
                        default=2**17)

    args = parser.parse_args()

    print(f"Loading files in {args.input_folder}")
    data_point_array = read_training_files(path_to_folder=args.input_folder,
                                           fft_size=args.fft_size)

    print(f"Plotting and storing plot in {args.output_name}")
    fig = plot_data_point_array(list_of_data_points=data_point_array,
                                fft_size=args.fft_size)

    fig.savefig(args.output_name, dpi=300)

    exit()
