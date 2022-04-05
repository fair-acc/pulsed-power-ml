from dataclasses import dataclass
from pathlib import Path
from typing import List


@dataclass
class ModelParams:
    name: str = 'disaggregation_model'
    save_folder: Path = Path('./')
    tuning_save_interval_length: int = 50,
    verbose: int = 0,

    # preprocessing parameters
    sampling_rate: int = 1_000
    rolling_time: float = 0.02
    diff_step_time: float = 0.02

    # activity detection tuning parameters
    first_check_counts: int = 3
    quantile_resolution: int = 2_000
    ignore_first_quantiles: int = 200
    quantile_detection_threshold: float = 0.33
    waiting_times: List[float] = None
    power_grid_scale: float = 1.0

    # activity detection parameters
    threshold_p: float = 0.06
    threshold_q: float = 0.105
    threshold_s: float = 0.06
    threshold_phi: float = 0.05
    activity_waiting_time: float = 0.1

    activity_alpha: float = 0.08  # depends on SR
    smooth_threshold: float = 0.001
    suppress_start: bool = True
    suppress_time: float = 0.3

    # model weights
    model_weights = None

    def __post_init__(self):
        if self.waiting_times is None:
            self.waiting_times = [0.05, 0.1, 0.15]
