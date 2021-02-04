import unittest
import sys
from hypothesis.extra.pandas import column, data_frames, range_indexes
from hypothesis import given
import src.it_ticket_analysis.train_tensorboard


class BasicDevEnvironmentTests(unittest.TestCase):
    """Basic tests
    """
    def test_happy_case(self):
        """ Happy Case
        """
        self.assertTrue(True)

    def test_python_version(self):
        """ Test python version

        Tests if python version is 3.7
        """
        self.assertEqual(3, sys.version_info.major)  # Major Version = 3
        self.assertGreaterEqual(7, sys.version_info.minor)  # Minor Version between 6 and 7
        self.assertLessEqual(6, sys.version_info.minor)


class InputDataframeTests(unittest.TestCase):
    @given(data_frames([column('fixed acidity', dtype=float),
                        column('volatile acidity', dtype=float),
                        column('citric acid', dtype=float),
                        column('residual sugar', dtype=float),
                        column('chlorides', dtype=float),
                        column('free sulfur dioxide', dtype=float),
                        column('total sulfur dioxide', dtype=float),
                        column('density', dtype=float),
                        column('pH', dtype=float),
                        column('sulphates', dtype=float),
                        column('alcohol', dtype=float),
                        column('quality', dtype=int)]))
    def test_df_has_twelve_columns(self, df):
        self.assertEqual(df.columns.__len__(), 12)

    @given(data_frames(index=range_indexes(min_size=5),
                       columns=[column('fixed acidity', dtype=float),
                        column('volatile acidity', dtype=float),
                        column('citric acid', dtype=float),
                        column('residual sugar', dtype=float),
                        column('chlorides', dtype=float),
                        column('free sulfur dioxide', dtype=float),
                        column('total sulfur dioxide', dtype=float),
                        column('density', dtype=float),
                        column('pH', dtype=float),
                        column('sulphates', dtype=float),
                        column('alcohol', dtype=float),
                        column('quality', dtype=int)]))
    def test_input_df_conversion(self, df):
        self.assertIsNotNone(src.it_ticket_analysis.train_tensorboard.df_to_dataset(df))


if __name__ == '__main__':
    unittest.main()
