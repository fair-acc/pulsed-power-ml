import sys

import numpy as np
import pandas as pd
from PyQt6.QtWidgets import QApplication

from pulsed_power_ml.data.mock_data_source import MockDataSource
from src.pulsed_power_ml.data.zmq_client import ZMQClient
from src.pulsed_power_ml.data.zmq_connection_info import ZMQConnectionInfo
from src.pulsed_power_ml.models.mock_model import MockModel
from src.pulsed_power_ml.visualization.non_intrusive_load_monitoring import NonIntrusiveLoadMonitoring


def get_conn_infos(nilm_window, classifier):
    """
    Get information on how to receive the data and how it should be processed.

    Parameters
    ----------
    nilm_window : NonIntrusiveLoadMonitoring
        The QWidget to be updated.
    classifier : MockModel
        The model to convert from power data to trigger data.

    Returns
    -------
    Dict[str, ZMQConnectionInfo]
    """

    def update_raw(data):
        nilm_window.updateDoubleYAxisPlots(
            pd.DataFrame(data, columns=['U', 'I']),
            'raw',
            {'U': 'uPlot', 'I': 'iPlot'}
        )

    def update_bp(data):
        nilm_window.updateDoubleYAxisPlots(
            pd.DataFrame(data, columns=['U', 'I']),
            'rawBp',
            {'U': 'uPlot', 'I': 'iPlot'}
        )

    def update_power_and_trigger(data):
        power_data = pd.DataFrame(data, columns=['P', 'Q', 'S', 'phi'])
        nilm_window.updateDoubleYAxisPlots(
            power_data,
            'power',
            {'P': 'pPlot', 'Q': 'qPlot', 'S': 'sPlot', 'phi': 'phiPlot'}
        )

        nilm_window.updateOneYAxisPlots(
            classifier.predict(power_data),
            'triggerEvents',
            {'S': 'sPlot', 'sEst': 'sEstPlot', 'sLED': 'sLEDPlot', 'sHalo': 'sHaloPlot', 'sFluo': 'sFluoPlot'}
        )

    def update_mains_freq(data):
        nilm_window.updateOneYAxisPlots(
            pd.DataFrame(data, columns=['mainsFreq']),
            'mainsFreq',
            {'mainsFreq': 'mfPlot'}
        )

    def update_power_spec(data):
        nilm_window.updateOneYAxisPlots(
            pd.DataFrame(data, columns=['powerSpec']),
            'powerSpec',
            {'powerSpec': 'psPlot'}
        )

    zmq_connection_infos = {
        'raw': ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=1,
            topic='',
            chan_cnt=2,
            data_type=np.float32,
            processing_function=update_raw
        ),
        'raw_bp': ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=2,
            topic='',
            chan_cnt=2,
            data_type=np.float32,
            processing_function=update_bp
        ),
        'power': ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=3,
            topic='',
            chan_cnt=4,
            data_type=np.float32,
            processing_function=update_power_and_trigger
        ),
        'mains_freq': ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=4,
            topic='',
            chan_cnt=1,
            data_type=np.float32,
            processing_function=update_mains_freq
        ),
        'power_spec': ZMQConnectionInfo(
            connection_string='tcp://127.0.0.1',
            port=5,
            topic='',
            chan_cnt=1,
            data_type=np.float32,
            processing_function=update_power_spec
        )
    }

    return zmq_connection_infos


def mock_example():
    """Working example using mocked data streams"""

    # Start App
    app = QApplication(sys.argv)
    window = NonIntrusiveLoadMonitoring()

    clf = MockModel()
    conn_infos = get_conn_infos(window, clf)

    mds = {}
    clients = {}
    for key, val in conn_infos.items():
        mds[key] = MockDataSource(val, key)
        mds[key].run()

        clients[key] = ZMQClient(val, val.processing_function)
        clients[key].run()

    window.show()
    sys.exit(app.exec())


if __name__ == '__main__':
    mock_example()
