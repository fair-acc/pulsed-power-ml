from pathlib import Path

import luigi
import pandas as pd

from src.kol_db.data.Task1 import Task1


class Task2(luigi.Task):
    """This Luigi Task loads all raw ticket files and combines them into one"""

    output_path = Path('data/processed/task2_output.csv')

    def requires(self):
        """
        Task1 has to be run before Task2
        """

        return Task1()

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
    luigi.build([Task2()], local_scheduler=True, detailed_summary=True)
