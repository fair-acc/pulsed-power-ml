import time
from datetime import datetime
from math import degrees

import numpy as np
import pandas as pd


class PowerBuffer:
    def __init__(self, update_rate, data_points_to_show):
        """
        Initialize data buffer for raw data

        Parameters
        ----------
        update_rate : int
        data_points_to_show : int
        """

        self.time_first_recv = round(time.time(), 3)

        self.update_rate = update_rate
        self.data_points_to_show = data_points_to_show

        self.prep_downsampling = 100
        self.target_sample_rate = 10

        self.data_counter = -1
        self.do_update_vis = False

        self.buffer = []
        self.data = pd.DataFrame()

        self.index = None
        self.first_index = None
        self.last_index = None

        self.base_date = datetime(1970, 1, 1, 0, 0, 0)

    def update_data(self, new_data):
        """
        Update the data for visualization

        Parameters
        ----------
        new_data : np.array
        """

        reduced_data = new_data[::self.prep_downsampling]
        self.buffer.extend(reduced_data)

        if len(self.buffer) >= self.data_points_to_show:
            self.buffer = self.buffer[-self.data_points_to_show:]
            index_shift = len(reduced_data)
        else:
            index_shift = 0

        if self.first_index is None:
            self.first_index = self.time_first_recv
        else:
            self.first_index += index_shift / self.target_sample_rate

        self.last_index = self.first_index + len(self.buffer) / self.target_sample_rate

        self.index = pd.Float64Index(
            np.arange(start=self.first_index, stop=self.last_index, step=1 / self.target_sample_rate)
        )[:len(self.buffer)]

        self.data_counter = (self.data_counter + 1) % self.update_rate
        self.do_update_vis = self.data_counter == 0

        self.data = pd.DataFrame(self.buffer, columns=['P', 'Q', 'S', 'phi'], index=self.index)
        self.data['phi'] = self.data['phi'].apply(lambda x: degrees(x))
