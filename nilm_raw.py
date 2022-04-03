"""
    Codestyle according to PyQt6 not PEP8
"""
import sys
from functools import partial
from typing import Dict, Tuple

import numpy as np
import zmq
from PyQt6.QtCore import QRunnable, pyqtSlot
from PyQt6.QtCore import QThreadPool
from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import QApplication, QTabWidget, QGridLayout, QVBoxLayout, QWidget
from pyqtgraph import PlotWidget, PlotItem, ViewBox, mkPen, AxisItem, DateAxisItem

from pulsed_power_ml.data.signal_communicate import SignalCommunicate
from src.pulsed_power_ml.data.buffers.mains_freq_buffer import MainsFreqBuffer
from src.pulsed_power_ml.data.buffers.raw_buffer import RawBuffer
from src.pulsed_power_ml.data.zmq_connection_info import ZMQConnectionInfo


class Worker(QRunnable):
    """
    Worker thread
    Inherits from QRunnable to handle worker thread setup, signals and wrap-up.
    """

    def __init__(self, conn_info):
        """

        Parameters
        ----------
        conn_info : ZMQConnectionInfo
        """
        super().__init__()
        self.conn_info = conn_info

    @pyqtSlot()
    def run(self):
        """
        Listen to ZMQ publisher send data to databuffer
        """

        context = zmq.Context()
        poller = zmq.Poller()

        subscriber = context.socket(zmq.SUB)
        subscriber.connect(f'{self.conn_info.connection_string}:{self.conn_info.port}')
        subscriber.subscribe(self.conn_info.topic)

        poller.register(subscriber, zmq.POLLIN)

        buffer = self.conn_info.buffer_object()

        while True:
            message = subscriber.recv_multipart()
            msg = np.frombuffer(message[0], self.conn_info.data_type)
            parsed_msg = np.array_split(msg, len(msg) // self.conn_info.chan_cnt)

            buffer.update_data(parsed_msg)
            if (buffer.data is not None) & buffer.do_update_vis:
                self.conn_info.processing_function.emit(buffer.data)

        print('Stopping ZMQClient.')
        subscriber.close()
        context.term()


class NonIntrusiveLoadMonitoringRaw(QWidget):
    def __init__(self,
                 raw_ax_lims=(-400, 400, -3, 3),
                 mf_offset=0.05,
                 *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.threadManager = QThreadPool()

        self.setWindowTitle('Non-Intrusive Load Monitoring - Raw data')
        self.resize(1200, 800)

        # Create a top-level layout
        layout = QVBoxLayout()
        self.setLayout(layout)

        # Plot information
        invTickFont = QFont()
        invTickFont.setPixelSize(1)
        visTickFont = QFont()
        visTickFont.setPixelSize(20)

        self.plotOptions = {
            'background': 'w',
            'foreground': 'k',
            'invTickFont': invTickFont,
            'visTickFont': visTickFont,
            'axisPen': partial(mkPen, width=1),
            'linePen': partial(mkPen, width=1)
        }

        self.plotHelper = partial(PlotWidget,
                                  background=self.plotOptions['background'],
                                  foreground=self.plotOptions['foreground'])

        self.fontSize = '15pt'
        self.colors = {
            'time': '#000000',
            'U': '#025b94',
            'I': '#c4380d',

            'mainsFreq': '#025b94'
        }

        self.plotData = {
            'raw': self._createRawPlot('RAW DATA', axis_limits=raw_ax_lims),
            'mainsFreq': self._createMainsFreqPlot('MAINS FREQUENCY', offset=mf_offset),
        }

        tabs = QTabWidget()
        tabs.addTab(self.rawDataUI(), 'Load Monitoring')
        layout.addWidget(tabs)

        self.signalComm = SignalCommunicate()
        self.connectSignals()

        self.getData()

    def connectSignals(self):
        """Connect signals for data update"""

        self.signalComm.requestRawGraphUpdate.connect(
            partial(self.updateDoubleYAxisPlots, plotName='raw', columnPlotMap={'U': 'uPlot', 'I': 'iPlot'})
        )

        self.signalComm.requestMainsFreqGraphUpdate.connect(
            partial(self.updateSingleYAxisPlots, plotName='mainsFreq', columnPlotMap={'mainsFreq': 'mfPlot'})
        )

    def getData(self):
        """Start workers with ZMQ listeners"""

        zmqConfig = {
            'raw': ZMQConnectionInfo(
                connection_string='tcp://10.0.0.2',
                port=5006,
                topic='',
                chan_cnt=2,
                data_type=np.float32,
                buffer_object=partial(RawBuffer, update_rate=50, data_points_to_show=12_000),
                processing_function=self.signalComm.requestRawGraphUpdate
            ),
            'mainsFreq': ZMQConnectionInfo(
                connection_string='tcp://10.0.0.2',
                port=5009,
                topic='',
                chan_cnt=1,
                data_type=np.float32,
                buffer_object=partial(MainsFreqBuffer, update_rate=50, data_points_to_show=1_000),
                processing_function=self.signalComm.requestMainsFreqGraphUpdate
            ),
        }

        for v in zmqConfig.values():
            worker = Worker(v)
            self.threadManager.start(worker)

    def rawDataUI(self):
        """Create the raw data interface."""

        generalTab = QWidget()
        layout = QGridLayout()

        layout.addWidget(self.plotData['raw']['plotWidget'], 0, 0)
        layout.addWidget(self.plotData['mainsFreq']['plotWidget'], 1, 0)
        generalTab.setLayout(layout)

        return generalTab

    def _createRawPlot(self, title, axis_limits):
        """
        Create the layout for the raw plot

        Parameters
        ----------
        title : str
        axis_limits : Tuple[int]
            Limits for the axis in order [x_min, x_max, y_min, y_max]

        Returns
        -------
        Dict
            Info about the plot
        """

        uiRaw = PlotItem(name='rawPlot', title=title)
        plotWidget = self.plotHelper(plotItem=uiRaw)

        uVBox = uiRaw.getViewBox()
        iVBox = ViewBox()

        # LEFT y-axis
        uiRaw.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        uiRaw.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['U']))
        uiRaw.setLabel('left', 'U', units='V', color=self.colors['U'], **{'font-size': self.fontSize})

        # x-axis
        uiRaw.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        uiRaw.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        uiRaw.setLabel('bottom', 'time', units='ms', color=self.colors['time'], **{'font-size': self.fontSize})

        # RIGHT y-axis
        yRight = AxisItem('right')
        yRight.setTickFont(self.plotOptions['visTickFont'])
        yRight.setPen(self.plotOptions['axisPen'](color=self.colors['I']))
        yRight.setLabel('I', units='A', color=self.colors['I'], **{'font-size': self.fontSize})

        uiRaw.layout.addItem(yRight, 2, 2)
        uiRaw.scene().addItem(iVBox)

        # Link right y-axis to ViewBox for I values
        yRight.linkToView(iVBox)
        # Link x-axis for both Views
        iVBox.setXLink(uiRaw)

        # Set Y axis ranges
        uVBox.setYRange(min=axis_limits[0], max=axis_limits[1])
        iVBox.setYRange(min=axis_limits[2], max=axis_limits[3])

        # Create plots
        uPlot = uiRaw.plot(pen=self.plotOptions['linePen'](self.colors['U']))
        iPlot = uiRaw.plot(pen=self.plotOptions['linePen'](self.colors['I']))

        # Downsample data
        uPlot.setDownsampling(auto=True, method='mean')
        iPlot.setDownsampling(auto=True, method='mean')

        # Add plots to ViewBox
        uVBox.addItem(uPlot)
        iVBox.addItem(iPlot)

        return {
            'plotWidget': plotWidget,
            'y2VBox': iVBox,
            'uPlot': uPlot,
            'iPlot': iPlot
        }

    def _createMainsFreqPlot(self, title, offset):
        """
        Create the layout for the mains frequency plot

        Parameters
        ----------
        title : str
        offset : float

        Returns
        -------
        Dict
            Info about the plot
        """

        mainsFreq = PlotItem(name='mainsFreqPlot', title=title,
                             labels={'left': {'axis': 'left'}, 'bottom': {'axis': 'bottom'}},
                             axisItems={'bottom': DateAxisItem(orientation='bottom')})
        plotWidget = self.plotHelper(plotItem=mainsFreq)

        mfVBox = mainsFreq.getViewBox()

        # LEFT y-axis
        mainsFreq.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        mainsFreq.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['mainsFreq']))
        mainsFreq.setLabel('left', 'MainsFreq', units='Hz', color=self.colors['mainsFreq'],
                           **{'font-size': self.fontSize})
        mfVBox.setYRange(min=50 - offset, max=50 + offset)

        # x-axis
        mainsFreq.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        mainsFreq.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        mainsFreq.setLabel('bottom', 'time', units='s', color=self.colors['time'], **{'font-size': self.fontSize})

        # Create plots
        mfPlot = mainsFreq.plot(pen=self.plotOptions['linePen'](self.colors['mainsFreq']))

        # Downsample data
        mfPlot.setDownsampling(auto=True, method='mean')

        # Add plots to ViewBox
        mfVBox.addItem(mfPlot)

        return {
            'plotWidget': plotWidget,
            'mfPlot': mfPlot
        }

    def updateDoubleYAxisPlots(self, data, plotName, columnPlotMap):
        """
        Updating method for all plots with two Y axes.

        Parameters
        ----------
        data : pd.DataFrame
        plotName : str
        columnPlotMap : Dict[str, str]
        """

        def u():
            uVBox = self.plotData[plotName]['plotWidget'].getPlotItem().getViewBox()
            iVBox = self.plotData[plotName]['y2VBox']
            iVBox.setGeometry(uVBox.sceneBoundingRect())
            iVBox.linkedViewChanged(uVBox, iVBox.XAxis)

        u()
        # self.plotData[plotName]['plotWidget'].getPlotItem().getViewBox() \
        #     .sigResized.connect(u)

        for col, plot in columnPlotMap.items():
            self.plotData[plotName][plot].setData(x=data.index, y=data[col].tolist())

    def updateSingleYAxisPlots(self, data, plotName, columnPlotMap):
        """
        Updating method for all plots with one Y axis.

        Parameters
        ----------
        data : pd.DataFrame
        plotName : str
        columnPlotMap : Dict[str, str]
        """

        for col, plot in columnPlotMap.items():
            self.plotData[plotName][plot].setData(x=data.index, y=data[col].tolist())


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NonIntrusiveLoadMonitoringRaw(
        raw_ax_lims=(-400, 400, -3, 3),
        mf_offset=0.05
    )
    window.show()
    sys.exit(app.exec())
