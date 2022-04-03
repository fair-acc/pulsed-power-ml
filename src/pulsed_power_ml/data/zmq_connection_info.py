from dataclasses import dataclass
from functools import partial
from typing import Union

from src.pulsed_power_ml.data.buffers.power_buffer import PowerBuffer
from src.pulsed_power_ml.data.buffers.mains_freq_buffer import MainsFreqBuffer
from src.pulsed_power_ml.data.buffers.raw_buffer import RawBuffer
from pulsed_power_ml.data.signal_communicate import SignalCommunicate


@dataclass
class ZMQConnectionInfo:
    connection_string: str
    port: int
    topic: str
    chan_cnt: int
    data_type: type
    buffer_object: Union[
        partial[RawBuffer],
        partial[PowerBuffer],
        partial[MainsFreqBuffer],
        None
    ]
    processing_function: Union[
        SignalCommunicate.requestBpGraphUpdate,
        SignalCommunicate.requestRawGraphUpdate,
        SignalCommunicate.requestPowerGraphUpdate
    ]
