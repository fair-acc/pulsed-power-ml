import unittest
import sys


class BasicDevEnvironmentTests(unittest.TestCase):
    """Basic tests
    """
    def test_happy_case(self):
        """ Happy Case
        """
        self.assertTrue(False)

    def test_python_version(self):
        """ Test python version

        Tests if python version is 3.7
        """
        self.assertEqual(3, sys.version_info.major)
        self.assertEqual(7, sys.version_info.minor)


if __name__ == '__main__':
    unittest.main()
