import pandas as pd
from PyQt6.QtCore import QObject, pyqtSignal


class SignalCommunicate(QObject):
    """Signal Object for data traffic"""

    requestRawGraphUpdate = pyqtSignal(pd.DataFrame)
    requestBpGraphUpdate = pyqtSignal(pd.DataFrame)
    requestPowerGraphUpdate = pyqtSignal(pd.DataFrame)
    requestMainsFreqGraphUpdate = pyqtSignal(pd.DataFrame)
    requestTriggerGraphUpdate = pyqtSignal(pd.DataFrame)
