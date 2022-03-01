from dataclasses import dataclass
from typing import Callable, List


@dataclass
class ZMQConnectionInfo:
    connection_string: str
    port: int
    topic: str
    chan_cnt: int
    data_type: type
    processing_function: Callable[[List], None] = None
