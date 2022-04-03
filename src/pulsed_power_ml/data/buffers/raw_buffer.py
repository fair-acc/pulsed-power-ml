import numpy as np
import pandas as pd


class RawBuffer:
    def __init__(self, update_rate, data_points_to_show):
        """
        Initialize data buffer for raw data

        Parameters
        ----------
        update_rate : int
        data_points_to_show : int
        """

        self.update_rate = update_rate
        self.data_points_to_show = data_points_to_show

        self.data_counter = -1
        self.do_update_vis = False

        self.buffer = []
        self.data = pd.DataFrame()

        self.index = pd.Float64Index(np.arange(start=0, stop=60, step=60 / self.data_points_to_show))

    def update_data(self, new_data):
        """
        Update the data for visualization

        Parameters
        ----------
        new_data : np.array
        """

        self.buffer.extend(new_data)

        if len(self.buffer) >= self.data_points_to_show:
            self.buffer = self.buffer[-self.data_points_to_show:]

            self.data = pd.DataFrame(self.buffer, columns=['U', 'I'], index=self.index)

            self.data_counter = (self.data_counter + 1) % self.update_rate
            self.do_update_vis = self.data_counter == 0
