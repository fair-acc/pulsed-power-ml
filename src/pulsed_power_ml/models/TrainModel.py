from pathlib import Path

import luigi
import pandas as pd

from src.pulsed_power_ml.data.CreateTrainData import CreateTrainData


class TrainModel(luigi.Task):
    """This Luigi Task loads all raw ticket files and combines them into one"""

    output_path = Path('data/processed/task2_output.csv')

    def requires(self):
        """
        CreateTrainData has to be run before TrainModel
        """

        return CreateTrainData()

    def output(self):
        """
        Mocks the Output
        """

        return luigi.LocalTarget(self.output_path)

    def run(self):
        """
        Do stuff
        """

        data = pd.read_csv(self.input().path)

        data.to_csv(self.output_path)


if __name__ == '__main__':
    luigi.build([TrainModel()], local_scheduler=True, detailed_summary=True)
