"""
    Codestyle analogous to PyQt6 not PEP8
"""
import sys
from functools import partial
from typing import Dict

import numpy as np
import zmq
from PyQt6.QtCore import QRunnable, pyqtSlot
from PyQt6.QtCore import QThreadPool
from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import QApplication, QTabWidget, QGridLayout, QVBoxLayout, QWidget
from pyqtgraph import PlotWidget, PlotItem, ViewBox, mkPen, AxisItem, DateAxisItem

from pulsed_power_ml.data.signal_communicate import SignalCommunicate
from src.pulsed_power_ml.data.buffers.power_buffer import PowerBuffer
from src.pulsed_power_ml.data.buffers.raw_buffer import RawBuffer
from src.pulsed_power_ml.data.zmq_connection_info import ZMQConnectionInfo

from src.pulsed_power_ml.data.buffers.trigger_buffer import TriggerBuffer

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
        Initialise the runner function with passed args, kwargs.
        """

        context = zmq.Context()
        poller = zmq.Poller()

        subscriber = context.socket(zmq.SUB)
        subscriber.connect(f'{self.conn_info.connection_string}:{self.conn_info.port}')
        subscriber.subscribe(self.conn_info.topic)

        poller.register(subscriber, zmq.POLLIN)

        buffer = self.conn_info.buffer_object()

        if self.conn_info.trigger_buffer != (None, ):
            trigger_buffer = self.conn_info.trigger_buffer()
        else:
            trigger_buffer = None

        while True:
            message = subscriber.recv_multipart()
            msg = np.frombuffer(message[0], self.conn_info.data_type)
            parsed_msg = np.array_split(msg, len(msg) // self.conn_info.chan_cnt)

            buffer.update_data(parsed_msg)
            if (buffer.data is not None) & buffer.do_update_vis:
                self.conn_info.processing_function.emit(buffer.data)

            if trigger_buffer is not None:
                trigger_buffer.update_data(parsed_msg)
                if (trigger_buffer.data is not None) & trigger_buffer.do_update_vis:
                    self.conn_info.trigger_proc_fun.emit(trigger_buffer.data)

        print('Stopping ZMQClient.')
        subscriber.close()
        context.term()


class NonIntrusiveLoadMonitoring(QWidget):
    def __init__(self,
                 raw_ax_lims=(-400, 400, -3, 3),
                 *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.threadManager = QThreadPool()
        print(f'Multithreading with maximum {self.threadManager.maxThreadCount()} threads')

        self.setWindowTitle('Non-Intrusive Load Monitoring')
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

            'P': '#025b94',
            'Q': '#c4380d',
            'S': '#F6BE00',
            'phi': '#228b22',

            'powerReal': '#000000',
            'powerEst': '#025b94',
            'ledBulb': '#F6BE00',
            'halogenBulb': '#228b22',
            'fluorescentTube': '#c4380d'
        }

        self.plotData = {
            'rawBp': self._createBpPlot('BANDPASS FILTERED', axis_limits=raw_ax_lims),
            'power': self._createPowerPlot('POWER'),
            'triggerEvents': self._createTriggerEventsPlot('TRIGGER EVENTS'),
        }

        # Create the tab widget with two tabs
        tabs = QTabWidget()
        tabs.addTab(self.loadMonitoringUI(), 'Load Monitoring')
        layout.addWidget(tabs)

        self.signalComm = SignalCommunicate()
        self.connectSignals()

        self.getData()

    def connectSignals(self):
        """Connect signals for data update"""

        self.signalComm.requestBpGraphUpdate.connect(
            partial(self.updateDoubleYAxisPlots, plotName='rawBp', columnPlotMap={'U': 'uPlot', 'I': 'iPlot'})
        )

        self.signalComm.requestPowerGraphUpdate.connect(
            partial(self.updateDoubleYAxisPlots, plotName='power',
                    columnPlotMap={'P': 'pPlot', 'Q': 'qPlot', 'S': 'sPlot', 'phi': 'phiPlot'})
        )

        self.signalComm.requestTriggerGraphUpdate.connect(
            partial(self.updateSingleYAxisPlots, plotName='triggerEvents',
                    columnPlotMap={'S': 'sPlot', 'sEst': 'sEstPlot', 'sLED': 'sLEDPlot', 'sHalo': 'sHaloPlot',
                                   'sFluo': 'sFluoPlot'})
        )

    def getData(self):
        """Start workers with ZMQ listeners"""

        zmqConfig = {
            'raw_bp': ZMQConnectionInfo(
                connection_string='tcp://10.0.0.2',
                port=5007,
                topic='',
                chan_cnt=2,
                data_type=np.float32,
                buffer_object=partial(RawBuffer, update_rate=50, data_points_to_show=60),
                processing_function=self.signalComm.requestBpGraphUpdate
            ),
            'power': ZMQConnectionInfo(
                connection_string='tcp://10.0.0.2',
                port=5008,
                topic='',
                chan_cnt=4,
                data_type=np.float32,
                buffer_object=partial(PowerBuffer, update_rate=10, data_points_to_show=6_000),
                processing_function=self.signalComm.requestPowerGraphUpdate,
                trigger_buffer=partial(TriggerBuffer, update_rate=10, data_points_to_show=6_000),
                trigger_proc_fun=self.signalComm.requestTriggerGraphUpdate
            ),
        }

        for v in zmqConfig.values():
            worker = Worker(v)
            self.threadManager.start(worker)

    def loadMonitoringUI(self):
        """Create the first tab for the dashboard."""

        generalTab = QWidget()
        layout = QGridLayout()

        layout.addWidget(self.plotData['rawBp']['plotWidget'], 0, 0)
        layout.addWidget(self.plotData['power']['plotWidget'], 1, 0)
        layout.addWidget(self.plotData['triggerEvents']['plotWidget'], 2, 0)
        generalTab.setLayout(layout)
        return generalTab

    def _createBpPlot(self, title, axis_limits):
        """
        Create the layout for the bandpass filtered plot

        Parameters
        ----------
        title : str

        Returns
        -------
        Dict
            Info about the plot
        """

        uiBp = PlotItem(name='bpPlot', title=title)
        plotWidget = self.plotHelper(plotItem=uiBp)

        uVBox = uiBp.getViewBox()
        iVBox = ViewBox()

        # LEFT y-axis
        uiBp.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        uiBp.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['U']))
        uiBp.setLabel('left', 'U', units='V', color=self.colors['U'], **{'font-size': self.fontSize})

        # x-axis
        uiBp.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        uiBp.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        uiBp.setLabel('bottom', 'time', units='ms', color=self.colors['time'], **{'font-size': self.fontSize})

        # RIGHT y-axis
        yRight = AxisItem('right')
        yRight.setTickFont(self.plotOptions['visTickFont'])
        yRight.setPen(self.plotOptions['axisPen'](color=self.colors['I']))
        yRight.setLabel('I', units='A', color=self.colors['I'], **{'font-size': self.fontSize})

        uiBp.layout.addItem(yRight, 2, 2)
        uiBp.scene().addItem(iVBox)

        # Link right y-axis to ViewBox for I values
        yRight.linkToView(iVBox)
        # Link x-axis for both Views
        iVBox.setXLink(uiBp)

        # Set Y axis ranges
        uVBox.setYRange(min=axis_limits[0], max=axis_limits[1])
        iVBox.setYRange(min=axis_limits[2], max=axis_limits[3])

        # Create plots
        uPlot = uiBp.plot(pen=self.plotOptions['linePen'](self.colors['U']))
        iPlot = uiBp.plot(pen=self.plotOptions['linePen'](self.colors['I']))

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

    def _createPowerPlot(self, title):
        """
        Create the layout for the power plot

        Parameters
        ----------
        title : str

        Returns
        -------
        Dict
            Info about the plot
        """

        powPlot = PlotItem(
            name='powPlot', title=title,
            axisItems={'bottom': DateAxisItem()}
        )
        plotWidget = self.plotHelper(plotItem=powPlot)

        pVBox = powPlot.getViewBox()
        phiVBox = ViewBox()

        # LEFT y-axis
        powPlot.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        powPlot.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['P']))
        powPlot.setLabel('left', 'P (W) '
                                 f'<font color={self.colors["Q"]}>Q (Var)</font> '
                                 f'<font color={self.colors["S"]}>S (VA)</font>',
                         color=self.colors['P'], **{'font-size': self.fontSize})

        # x-axis
        powPlot.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        powPlot.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        powPlot.setLabel('bottom', 'time', units='s', color=self.colors['time'], **{'font-size': self.fontSize})

        # RIGHT y-axis
        ax_phi = AxisItem('right')
        ax_phi.setTickFont(self.plotOptions['visTickFont'])
        ax_phi.setPen(self.plotOptions['axisPen'](color=self.colors['phi']))
        ax_phi.setLabel(u'\u03c6', units='deg', color=self.colors['phi'], **{'font-size': self.fontSize})

        powPlot.layout.addItem(ax_phi, 2, 2)
        powPlot.scene().addItem(phiVBox)

        # Link right y-axis to ViewBox for I values
        ax_phi.linkToView(phiVBox)
        # Link x-axis for both Views
        phiVBox.setXLink(powPlot)

        # Create plots
        pPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['P']))
        qPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['Q']))
        sPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['S']))
        phiPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['phi']))

        # Add Downsampling
        pPlot.setDownsampling(auto=True, method='mean')
        qPlot.setDownsampling(auto=True, method='mean')
        sPlot.setDownsampling(auto=True, method='mean')
        phiPlot.setDownsampling(auto=True, method='mean')

        # Add plot to ViewBox
        pVBox.addItem(pPlot)
        pVBox.addItem(qPlot)
        pVBox.addItem(sPlot)
        phiVBox.addItem(phiPlot)

        return {
            'plotWidget': plotWidget,
            'y2VBox': phiVBox,
            'pPlot': pPlot,
            'qPlot': qPlot,
            'sPlot': sPlot,
            'phiPlot': phiPlot
        }

    def _createTriggerEventsPlot(self, title):
        """
        Create the layout for the plot showing the classified devices

        Parameters
        ----------
        title : str

        Returns
        -------
        Dict
            Info about the plot
        """

        triggerEvents = PlotItem(name='triggerEventsPlot', title=title, axisItems={'bottom': DateAxisItem()})
        plotWidget = self.plotHelper(plotItem=triggerEvents)

        teVBox = triggerEvents.getViewBox()

        # LEFT y-axis
        triggerEvents.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        triggerEvents.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['powerReal']))
        triggerEvents.setLabel('left', 'Leistung (real) '
                                       f'<font color={self.colors["powerEst"]}>Leistung (est)</font> '
                                       f'<font color={self.colors["ledBulb"]}>LED</font> '
                                       f'<font color={self.colors["halogenBulb"]}>Halogen</font> '
                                       f'<font color={self.colors["fluorescentTube"]}>Röhre</font>',
                               color=self.colors['powerReal'], **{'font-size': self.fontSize})

        # x-axis
        triggerEvents.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        triggerEvents.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        triggerEvents.setLabel('bottom', 'time', units='s', color=self.colors['time'], **{'font-size': self.fontSize})

        # Create plots
        sPlot = triggerEvents.plot(pen=self.plotOptions['linePen'](self.colors['powerReal']))
        sEstPlot = triggerEvents.plot(pen=self.plotOptions['linePen'](self.colors['powerEst']))
        sLEDPlot = triggerEvents.plot(pen=self.plotOptions['linePen'](self.colors['ledBulb']))
        sHaloPlot = triggerEvents.plot(pen=self.plotOptions['linePen'](self.colors['halogenBulb']))
        sFluoPlot = triggerEvents.plot(pen=self.plotOptions['linePen'](self.colors['fluorescentTube']))

        # Add Downsampling
        sPlot.setDownsampling(auto=True, method='mean')
        sEstPlot.setDownsampling(auto=True, method='mean')
        sLEDPlot.setDownsampling(auto=True, method='mean')
        sHaloPlot.setDownsampling(auto=True, method='mean')
        sFluoPlot.setDownsampling(auto=True, method='mean')

        # Add plots to ViewBox
        teVBox.addItem(sPlot)
        teVBox.addItem(sEstPlot)
        teVBox.addItem(sLEDPlot)
        teVBox.addItem(sHaloPlot)
        teVBox.addItem(sFluoPlot)

        return {
            'plotWidget': plotWidget,
            'sPlot': sPlot,
            'sEstPlot': sEstPlot,
            'sLEDPlot': sLEDPlot,
            'sHaloPlot': sHaloPlot,
            'sFluoPlot': sFluoPlot,
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
            self.plotData[plotName][plot].setData(x=data.index, y=data[col].tolist(), _callSync='on')

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
    window = NonIntrusiveLoadMonitoring()
    window.show()
    sys.exit(app.exec())
