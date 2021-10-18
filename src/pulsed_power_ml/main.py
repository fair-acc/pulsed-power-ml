import luigi

from src.pulsed_power_ml.data.CreateTrainData import CreateTrainData
from pulsed_power_ml.models.TrainModel import TrainModel


class ExampleWrapperTask(luigi.WrapperTask):
    """Run workflow to create word count and feature files

    """

    def requires(self):
        """Requires method for Luigi

        https://luigi.readthedocs.io/en/stable/tasks.html
        """

        return [CreateTrainData(), TrainModel()]


def main():
    num_workers = 3

    luigi.build([ExampleWrapperTask()],
                local_scheduler=True, detailed_summary=True, workers=num_workers)


if __name__ == '__main__':
    main()
