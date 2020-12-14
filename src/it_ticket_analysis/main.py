import luigi

from src.it_ticket_analysis.data.Task1 import Task1
from src.it_ticket_analysis.data.Task2 import Task2


class ExampleWrapperTask(luigi.WrapperTask):
    """Run workflow to create word count and feature files

    """

    def requires(self):
        """Requires method for Luigi

        https://luigi.readthedocs.io/en/stable/tasks.html
        """

        return [Task1(), Task2()]


def main():
    luigi.build([ExampleWrapperTask()],
                local_scheduler=True, detailed_summary=True, workers=7)


if __name__ == '__main__':
    main()
