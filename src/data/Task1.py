from pathlib import Path

import luigi
import pandas as pd


class Task1(luigi.Task):
    """This Luigi Task loads all raw ticket files and combines them into one"""

    output_path = Path('data/processed/task1_output.csv')

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
    luigi.build([Task1()], local_scheduler=True, detailed_summary=True)
