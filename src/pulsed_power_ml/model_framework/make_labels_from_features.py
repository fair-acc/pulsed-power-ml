"""
Module to crate label csv-files from feature csv-files.
"""

import argparse

import numpy as np


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

    parser.add_argument('-i',
                        '--index',
                        help='Index of the device for which labels should be produced.',
                        required=True,
                        type=int)

    parser.add_argument('-n',
                        '--number-of-devices',
                        help='Total number of known devices. Default = 7.',
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

    # Produce label array
    label_array = np.zeros(shape=(n_labels, args.number_of_devices * 2 + 1))
    for i, label in enumerate(label_array):
        if i % 2 == 0:
            label_array[i, args.index] = 1
        else:
            label_array[i, args.index + args.number_of_devices] = 1

    # Write to disk
    print(f'Write produced labels into {args.output_file_name}')
    np.savetxt(fname=args.output_file_name,
               X=label_array,
               delimiter=',')

    return


if __name__ == '__main__':
    main()
