import pandas as pd


class MockModel:
    @staticmethod
    def predict(data):
        """
        Mock method to convert power data to trigger data

        Parameters
        ----------
        data : pd.DataFrame

        Returns
        -------
        pd.DataFrame
        """

        result = data.rename(columns={'P': 'S', 'Q': 'sEst', 'S': 'sLED', 'phi': 'sHalo'})
        result['sFluo'] = result['sHalo'] + 0.2
        return result
