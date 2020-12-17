import unittest
import sys


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


if __name__ == '__main__':
    unittest.main()
