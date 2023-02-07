import argparse
import sys

import numpy as np

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
    args = parser.parse_args()

    # Read parameter file
    parameter_dict = read_parameters(args.parameter_file)

    # Load data points
    data_point_array = read_training_files(args.input_folder, fft_size=parameter_dict['fft_size'])

    # Produce features
    features_array = get_features_from_raw_data(data_point_array, parameter_dict)

    # write features to file
    output_file_name = f'{args.output_folder}/features.csv'
    np.savetxt(fname=output_file_name,
               X=features_array,
               delimiter=',')

    return

if __name__ == '__main__':
    print(sys.path)
    main()
