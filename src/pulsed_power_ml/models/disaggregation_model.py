from typing import Dict

import numpy as np
import pandas as pd

from src.pulsed_power_ml.models.model_params import ModelParams


class DisaggregationModel:
    def __init__(self, params=None):
        """

        Parameters
        ----------
        params : ModelParams
        """

        if params is None:
            self.params = ModelParams()
        else:
            self.params = params

        self.iteration = 0

        self.rolling_frames = int(self.params.rolling_time * self.params.sampling_rate)
        self.diff_step_len = int(self.params.diff_step_time * self.params.sampling_rate)
        self.suppress_index = int(self.params.suppress_time * self.params.sampling_rate)
        self.params.activity_waiting_time = int(self.params.activity_waiting_time * self.params.sampling_rate)

        self.tuning_waiting_frames = [int(wt * self.params.sampling_rate) for wt in self.params.waiting_times]

        self.activity_detection_tuned = False
        self.tuning_checks_path = self.params.save_folder / f'{self.params.name}_tuning_checks.pkl'

        self.device_states = None

    def predict(self, new_data):

        return [np.append(n, [0]) for n in new_data]

    def init_for_curr(self):
        pass

    def preproc_data(self):
        pass

    def disaggregate_frame(self, stream_item, name=None):
        """

        Parameters
        ----------
        stream_item : pd.DataFrame
        name : str = None

        Returns
        -------
        Dict
        """

        pass
