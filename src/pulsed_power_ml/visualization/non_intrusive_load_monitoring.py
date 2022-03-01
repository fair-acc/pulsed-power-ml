"""
    Codestyle analogous to PyQt6 not PEP8
"""
import sys
from functools import partial
from typing import Dict

from PyQt6.QtCore import QThreadPool
from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import QApplication, QTabWidget, QGridLayout, QVBoxLayout, QWidget
from pyqtgraph import PlotWidget, PlotItem, ViewBox, mkPen, AxisItem, DateAxisItem


class NonIntrusiveLoadMonitoring(QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.threadManager = QThreadPool()

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

            'mainsFreq': '#025b94',

            'powerSpec': '#025b94',
            'limitCurve': '#c4380d',

            'powerReal': '#000000',
            'powerEst': '#025b94',
            'ledBulb': '#F6BE00',
            'halogenBulb': '#228b22',
            'fluorescentTube': '#c4380d'
        }

        self.plotData = {
            'raw': self._createRawPlot('RAW DATA'),
            'rawBp': self._createBpPlot('BANDPASS FILTERED'),
            'power': self._createPowerPlot('POWER', downsampleFactor=1),
            'mainsFreq': self._createMainsFreqPlot('MAINS FREQUENCY', offset=0.05),
            'powerSpec': self._createPowerSpecPlot('POWER SPECTRUM'),

            'triggerEvents': self._createTriggerEventsPlot('TRIGGER EVENTS')
        }

        # Create the tab widget with two tabs
        tabs = QTabWidget()
        tabs.addTab(self.loadMonitoringUI(), 'Load Monitoring')
        tabs.addTab(self.powerDisaggregationUI(), 'Power Disaggregation')
        layout.addWidget(tabs)

    def loadMonitoringUI(self):
        """Create the first tab for the dashboard."""

        generalTab = QWidget()
        layout = QGridLayout()

        layout.addWidget(self.plotData['raw']['plotWidget'], 0, 0)
        layout.addWidget(self.plotData['rawBp']['plotWidget'], 0, 1)
        layout.addWidget(self.plotData['power']['plotWidget'], 1, 0)
        layout.addWidget(self.plotData['mainsFreq']['plotWidget'], 1, 1)
        layout.addWidget(self.plotData['powerSpec']['plotWidget'], 2, 0, 1, 2)
        generalTab.setLayout(layout)
        return generalTab

    def powerDisaggregationUI(self):
        """Create the second tab with prediction"""

        networkTab = QWidget()
        layout = QGridLayout()
        layout.addWidget(self.plotData['triggerEvents']['plotWidget'])
        networkTab.setLayout(layout)
        return networkTab

    def _createRawPlot(self, title):
        """
        Create the layout for the raw plot

        Parameters
        ----------
        title : str

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
        yRight.setTickFont(self.plotOptions['invTickFont'])
        yRight.setPen(self.plotOptions['axisPen'](color=self.colors['I']))

        uiRaw.layout.addItem(yRight, 2, 2)
        uiRaw.scene().addItem(iVBox)

        # Link right y-axis to ViewBox for I values
        yRight.linkToView(iVBox)
        # Link x-axis for both Views
        iVBox.setXLink(uiRaw)

        # Set Y axis ranges
        uVBox.setYRange(min=-2, max=2)
        iVBox.setYRange(min=-2, max=2)

        # Create plots
        uPlot = uiRaw.plot(pen=self.plotOptions['linePen'](self.colors['U']))
        iPlot = uiRaw.plot(pen=self.plotOptions['linePen'](self.colors['I']))

        # Add plots to ViewBox
        uVBox.addItem(uPlot)
        iVBox.addItem(iPlot)

        return {
            'plotWidget': plotWidget,
            'y2VBox': iVBox,
            'uPlot': uPlot,
            'iPlot': iPlot
        }

    def _createBpPlot(self, title):
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
        uiBp.getAxis('left').setTickFont(self.plotOptions['invTickFont'])
        uiBp.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['U']))

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
        uVBox.setYRange(min=-2, max=2)
        iVBox.setYRange(min=-2, max=2)

        # Create plots
        uPlot = uiBp.plot(pen=self.plotOptions['linePen'](self.colors['U']))
        iPlot = uiBp.plot(pen=self.plotOptions['linePen'](self.colors['I']))

        # Add plots to ViewBox
        uVBox.addItem(uPlot)
        iVBox.addItem(iPlot)

        return {
            'plotWidget': plotWidget,
            'y2VBox': iVBox,
            'uPlot': uPlot,
            'iPlot': iPlot
        }

    def _createPowerPlot(self, title, downsampleFactor):
        """
        Create the layout for the power plot

        Parameters
        ----------
        title : str
        downsampleFactor : int

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

        # TODO: Remove invTickFont and set "showAxes(showValues=('left', False))" etc.

        powPlot.layout.addItem(ax_phi, 2, 2)
        powPlot.scene().addItem(phiVBox)
        # powPlot.showAxes(True, showValues=(True, False, False, True))

        # Link right y-axis to ViewBox for I values
        ax_phi.linkToView(phiVBox)
        # Link x-axis for both Views
        phiVBox.setXLink(powPlot)

        # Set Y axis ranges
        pVBox.setYRange(min=-2, max=2)
        phiVBox.setYRange(min=-2, max=2)

        # Create plots
        pPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['P']))
        qPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['Q']))
        sPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['S']))
        phiPlot = powPlot.plot(pen=self.plotOptions['linePen'](self.colors['phi']))

        # Add Downsampling
        pPlot.setDownsampling(ds=downsampleFactor, auto=False, method='mean')
        qPlot.setDownsampling(ds=downsampleFactor, auto=False, method='mean')
        sPlot.setDownsampling(ds=downsampleFactor, auto=False, method='mean')
        phiPlot.setDownsampling(ds=downsampleFactor, auto=False, method='mean')

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
                             labels={'right': {'axis': 'right'}, 'bottom': {'axis': 'bottom'}},
                             axisItems={'bottom': DateAxisItem()})
        plotWidget = self.plotHelper(plotItem=mainsFreq)

        mfVBox = mainsFreq.getViewBox()

        # LEFT y-axis
        mainsFreq.showAxis('left', show=False)

        # x-axis
        mainsFreq.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        mainsFreq.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        mainsFreq.setLabel('bottom', 'time', units='s', color=self.colors['time'], **{'font-size': self.fontSize})

        # RIGHT y-axis
        mainsFreq.getAxis('right').setTickFont(self.plotOptions['visTickFont'])
        mainsFreq.getAxis('right').setPen(self.plotOptions['axisPen'](color=self.colors['mainsFreq']))
        mainsFreq.setLabel('right', 'MainsFreq', units='Hz', color=self.colors['mainsFreq'],
                           **{'font-size': self.fontSize})
        mfVBox.setYRange(min=50 - offset, max=50 + offset)

        # Create plots
        mfPlot = mainsFreq.plot(pen=self.plotOptions['linePen'](self.colors['mainsFreq']))

        # Add plots to ViewBox
        mfVBox.addItem(mfPlot)

        return {
            'plotWidget': plotWidget,
            'mfPlot': mfPlot
        }

    def _createPowerSpecPlot(self, title):
        """
        Create the layout for the power spec plot

        Parameters
        ----------
        title : str

        Returns
        -------
        Dict
            Info about the plot
        """

        powerSpec = PlotItem(name='powerSpecPlot', title=title)
        plotWidget = self.plotHelper(plotItem=powerSpec)

        psVBox = powerSpec.getViewBox()

        # LEFT y-axis
        powerSpec.getAxis('left').setTickFont(self.plotOptions['visTickFont'])
        powerSpec.getAxis('left').setPen(self.plotOptions['axisPen'](color=self.colors['powerSpec']))
        powerSpec.setLabel('left', 'PowerSpec', units='dB', color=self.colors['powerSpec'],
                           **{'font-size': self.fontSize})

        # x-axis
        powerSpec.getAxis('bottom').setTickFont(self.plotOptions['visTickFont'])
        powerSpec.getAxis('bottom').setPen(self.plotOptions['axisPen'](color=self.colors['time']))
        powerSpec.setLabel('bottom', 'time', units='Hz', color=self.colors['time'], **{'font-size': self.fontSize})
        psVBox.setXRange(min=0, max=7)

        # Create plots
        psPlot = powerSpec.plot(pen=self.plotOptions['linePen'](self.colors['mainsFreq']))
        limitCurvePlot = powerSpec.plot(pen=self.plotOptions['linePen'](self.colors['limitCurve']))

        # Add plots to ViewBox
        psVBox.addItem(psPlot)
        psVBox.addItem(limitCurvePlot)

        return {
            'plotWidget': plotWidget,
            'psPlot': psPlot,
            'limitCurvePlot': limitCurvePlot
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
        self.plotData[plotName]['plotWidget'].getPlotItem().getViewBox() \
            .sigResized.connect(u)

        for col, plot in columnPlotMap.items():
            self.plotData[plotName][plot].setData(data[col].tolist())

    def updateOneYAxisPlots(self, data, plotName, columnPlotMap):
        """
        Updating method for all plots with one Y axis.

        Parameters
        ----------
        data : pd.DataFrame
        plotName : str
        columnPlotMap : Dict[str, str]
        """

        for col, plot in columnPlotMap.items():
            self.plotData[plotName][plot].setData(data[col].tolist())


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = NonIntrusiveLoadMonitoring()
    window.show()
    sys.exit(app.exec())
