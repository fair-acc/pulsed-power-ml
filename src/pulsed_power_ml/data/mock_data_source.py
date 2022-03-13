import time
from threading import Thread

import numpy as np
import pandas as pd
import zmq


class MockDataError(ValueError):
    """Error concerning MockDataSource"""
    pass


class MockDataSource:
    def __init__(self, conn_info, stream_type):
        """
        Set initial parameters

        Parameters
        ----------
        conn_info : ZMQConnectionInfo
        stream_type : str
        """

        ctx = zmq.Context()
        self.socket = ctx.socket(zmq.PUB)
        self.socket.bind(f'{conn_info.connection_string}:{conn_info.port}')

        self.data = self.get_data(stream_type)
        self.ptr = 0

    def run(self):
        """Execute in separate thread"""

        Thread(target=self._execute, daemon=True).start()

    def _execute(self):
        """Run ZMQ publisher in infinite while loop"""

        while True:
            start_row = ((self.ptr % 30) - 1) * 316
            stop_row = (self.ptr % 30) * 316

            self.socket.send(self.data[start_row:stop_row].to_numpy(dtype=np.float32).flatten())
            self.ptr += 1
            time.sleep(0.1)

    @staticmethod
    def get_data(stream_type):
        """
        Get the mock data for the ZMQ stream.

        Parameters
        ----------
        stream_type : str
            Identifying which data should be sent.
        Returns
        -------
        pd.DataFrame
            The mock data.
        """

        amplitude = np.sin(np.arange(0, 10000, 0.1))

        if stream_type in ('raw', 'raw_bp'):
            data = pd.DataFrame({'U': amplitude, 'I': amplitude / 2})
            return data
        elif stream_type == 'power':
            return pd.DataFrame({
                'P': amplitude,
                'Q': amplitude + 0.2,
                'S': amplitude + 0.4,
                'phi': amplitude + 0.6
            })
        elif stream_type == 'mains_freq':
            return pd.DataFrame({'mainsFreq': np.random.normal(loc=50, scale=0.01, size=10000)})
        elif stream_type == 'power_spec':
            return pd.DataFrame({'powerSpec': np.random.uniform(low=0, high=7, size=10000)})
        else:
            raise MockDataError(f'Unknown stream type: {stream_type}')


if __name__ == '__main__':
    from src.pulsed_power_ml.data.zmq_connection_info import ZMQConnectionInfo

    mds = MockDataSource(
        ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=1234,
            topic='',
            chan_cnt=1,
            data_type=np.float32
        ),
        'raw'
    )
    mds.run()
