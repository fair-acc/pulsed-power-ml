"""
Module to crate label csv-files from feature csv-files.
"""

import argparse
import sys

import numpy as np

sys.path.append('../../../')
from src.pulsed_power_ml.model_framework.data_io import read_parameters


def main():

    parser = argparse.ArgumentParser(
        description=('Tool to create simple (!) label csv-files from feature-csv files. Alternating ON-OFF labels for '
                     'one device only! First label will be the ON-switch of the respective device.')

    )

    parser.add_argument('-f',
                        '--feature-file',
                        help='Path to feature file',
                        required=True)

    parser.add_argument('-o',
                        '--output-file-name',
                        help='Name of output file (label csv-file)',
                        required=True)

    parser.add_argument('-b',
                        '--base-name',
                        help=('Base name (e.g. "e1" or "r2") of the device, that is being processed. '
                              'Either basename and parameter file or index and number-of-devices need to be provided'),
                        type=str,
                        required=True)

    parser.add_argument('-p',
                        '--parameter-file',
                        help=('Path to parameter file. '
                              'Either basename and parameter file or index and number-of-devices need to be provided'),
                        type=str,
                        required=True)

    parser.add_argument('-i',
                        '--index',
                        help=('Index of the device for which labels should be produced. '
                              'Either basename and parameter file or index and number-of-devices need to be provided'),
                        type=int)

    parser.add_argument('-n',
                        '--number-of-devices',
                        help=('Total number of known devices. Default = 7. '
                              'Either basename and parameter file or index and number-of-devices need to be provided'),
                        default=7,
                        type=int)

    args = parser.parse_args()

    # Read feature file
    print(f'Reading features in file {args.feature_file}')
    feature_array = np.loadtxt(args.feature_file, delimiter=',')
    n_labels = feature_array.shape[0]
    print(f'Found {n_labels} features.')
    if n_labels % 2 != 0:
        print(f'This is an uneven number of switching events. This seems odd. Is this intended?')

    # Read parameter file and derive index and number of devices
    if args.base_name is not None and args.parameter_file is not None:
        parameter_dict = read_parameters(args.parameter_file)
        match_found = False
        appliance_list = parameter_dict['appliances']
        for index, item in enumerate(appliance_list):
            if args.base_name.lower() in list(item.values())[0].lower().split(', '):
                match_found = True
                break

        assert match_found, f'Base name: "{args.base_name}" not contained in parameter file "{args.parameter_file}"!'

        n_devices = len(appliance_list)

    elif args.index is not None and args.number_of_devices is not None:
        index = args.index
        n_devices = args.args.number_of_devices

    else:
        raise RuntimeError("Either basename and parameter file or index and number-of-devices need to be provided!")

    # Produce label array
    label_array = np.zeros(shape=(n_labels, n_devices * 2 + 1))
    for i, label in enumerate(label_array):
        if i % 2 == 0:
            label_array[i, index] = 1
        else:
            label_array[i, index + n_devices] = 1

    # Write to disk
    print(f'Write produced labels into {args.output_file_name}')
    np.savetxt(fname=args.output_file_name,
               X=label_array,
               delimiter=',')

    return


if __name__ == '__main__':
    main()
