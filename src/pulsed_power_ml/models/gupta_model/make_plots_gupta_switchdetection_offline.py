import os

from src.pulsed_power_ml.model_framework.visualizations import make_gupta_switch_detection_plot


def main():

    INPUT_FOLDER = ('/home/thomas/projects/fair/call_3/data/raw_data/'
                    '2022-11-16_training_data/')

    OUTPUT_FOLDER = ('/home/thomas/projects/fair/call_3/experiments/gupta_switch_detection/'
                     'reimplemented_gupta_switch_detection/')

    for current_dir in os.listdir(INPUT_FOLDER):
        full_path = f'{INPUT_FOLDER}/{current_dir}'
        full_output_name = f'{OUTPUT_FOLDER}/gupta_switch_detection_dBm_{current_dir}.pdf'
        print(f'Processing data in {full_path}')

        fig = make_gupta_switch_detection_plot(
            path_to_data_folder=full_path,
            window_size=10,
            threshold=55,
            log_scale=True
        )

        print(f'Storing data in {full_output_name}')
        fig.savefig(full_output_name)

if __name__ == '__main__':
    main()
