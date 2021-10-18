from pathlib import Path

import luigi
import pandas as pd


class CreateTrainData(luigi.Task):
    """Read ZMQ stream and save training data to file"""

    output_path = Path(f'data/raw/train_data.csv')

    def output(self):
        """
        Mocks the Output
        """

        return luigi.LocalTarget(self.output_path)

    def run(self):
        """
        Do stuff
        """

        data = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
        data.to_csv(self.output_path)


if __name__ == '__main__':
    luigi.build([CreateTrainData()], local_scheduler=True, detailed_summary=True)
