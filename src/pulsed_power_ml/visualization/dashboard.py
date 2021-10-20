"""
This module contains all functions necessary to
"""
from functools import partial
import datetime as dt

import numpy as np
import holoviews as hv
import holoviews.plotting.bokeh
import pandas as pd

from holoviews.streams import Pipe, Buffer

renderer = hv.renderer('bokeh')


def create_template_df(n_dev: int) -> pd.DataFrame:
    """
    Return a template data frame to be used as the first input for the power versus time visualization

    Parameters
    ----------
    n_dev : int
        Number of devices that should be displayed.

    Returns
    -------
    template_df : pd.DataFrame,
        Data frame with the structure of real data: columns = [timestamp, dev_0, dev_1, ...]

    Raises
    ------
    AssertionError:
        If n_dev < 1 or n_dev is not integer.
    """
    # Check parameters
    assert type(n_dev) == int, 'n_dev needs to be of type int'
    assert n_dev > 0, 'n_dev needs to be greater then 0'

    # Create template df
    data = {'timestamp': [dt.datetime.now()]}

    for dev_number in range(n_dev):
        data[f'dev_{dev_number}'] = [0]

    template_df = pd.DataFrame(data=data)

    return template_df


def power_stack(data: pd.DataFrame) -> hv.element:
    """
    Function to create the stacked power versus time plot.

    Parameters
    ----------
    data : pd.DataFrame
        Data frame containing the power values. Columns: timestamp, dev_0, dev_1, ...

    Returns
    -------
    area_element : hv.element
        Stacked area plot: power (one color per device) versus time.
    """
    # Unpivot data table
    melted_data = pd.melt(data, 'timestamp', var_name='device', value_name='Power')

    # Create area plots
    areas = hv.Dataset(melted_data).to(hv.Area, 'timestamp', 'Power')

    # Stack areas
    area_element = hv.Area.stack(areas.overlay())

    return area_element


def create_power_stack_display(n_dev: int, window_len: int = 300) -> (holoviews.core.spaces.DynamicMap,
                                                                      hv.streams.Buffer):
    """
    Instantiates the dynamic map which displays the stacked power versus time and creates a data stream to which
    data points can be sent which will then be displayed.

    Parameters
    ----------
    n_dev : int
        Number of devices present in the data
    window_len: int
        Number of data points, which should be displayed at the same time.

    Returns
    -------
    power_stack_dmap : holoviews.core.spaces.DynamicMap
        Holoviews dynamic map containing the stacked area element.
    df_stream : hv.streams.Buffer
        Data stream which accepts data to be visualized.
    """
    # Create template_df
    template_df = create_template_df(n_dev)

    # Instantiate data stream to power stack display
    power_stream = Buffer(data=template_df,
                          length=window_len,
                          index=False)

    # Create power stack dmap (needs to be instantiated with some valid data)
    power_stack_dmap = hv.DynamicMap(callback=power_stack,
                                     streams=[power_stream])

    # Customize visualization
    power_stack_dmap.opts(title='Power versus time',
                          width=1000,
                          height=500,
                          show_grid=True,
                          legend_position='top_left')

    return power_stack_dmap, power_stream


def start_display_server(n_dev: int, window_len: int = 300) -> hv.streams.Buffer:
    """
    Start the display server. Dashboard can be accessed via the web browser. URL is displayed in the command line.

    Parameters
    ----------
    n_dev : int
        Number of device to visualize.

    window_len : int
        Maximum number of data point to be visualzed at the same time.

    Returns
    -------
    power_data_sink : hv.streams.Buffer
        A buffer object which serves as data sink for power data.

    state_data_sink : hv.streams.Buffer
        A buffer object which serves as data sink for state data.

    Raises
    ------
    AssertionError:
        If n_dev or window_len are not of type int or less then 1.
    """
    # Check parameter
    assert type(n_dev) == int, 'n_dev must be of type integer'
    assert n_dev > 0, 'n_dev must be greater then 0'
    assert type(window_len) == int, 'window_len must be of type integer'
    assert window_len > 0, 'window_len must be greater then 0'

    # crate layout and stream for the power plot
    power_stack_dmap, power_stream = create_power_stack_display(n_dev=n_dev, window_len=window_len)

    # ToDo: create layout and stream for the state plot
    state_stream = Buffer(pd.DataFrame())

    # Start server
    layout = power_stack_dmap

    doc = renderer.server_doc(layout)
    doc.add_periodic_callback(partial(update_data, power_stream, state_stream, n_dev),
                              1.0)

    # Return buffer objects
    return power_stream


def update_data(power_stream: hv.streams.Buffer, state_stream: hv.streams.Buffer, n_dev: int) -> None:
    """
    This function requests a new datapoint for power and state data and then pushes these to the respective streams.

    Parameters
    ----------
    power_stream : hv.streams.Buffer
        Stream for power data. Expects a data frame with 'timestamp' column and 'dev_i' columns containing power values.
    state_stream : hv.streams.Buffer
        Stream for state data. Expects a data frame with 'timestamp' column and 'dev_i' columns containing state values.
    n_dev : int
        Number of devices in data.
    """
    # Get new data points
    # ToDo: Replace with real data sources as soon as possible.
    power_data = next(dummy_data_gen(n_dev=n_dev, data_type='power'))
    # state_data = next(dummy_data_gen(n_dev=n_dev, data_type='power'))

    # Push data to streams
    power_stream.send(power_data)
    # state_stream.send(state_data)

    return


def dummy_data_gen(n_dev: int, data_type: str) -> pd.DataFrame:
    """
    Generator of dummy data with *n_dev* devices.

    Parameters
    ----------
    n_dev : int
        Number of devices.
    data_type : str
        Either 'power' or 'state'

    Returns
    -------
    dummy_df : pd.DataFrame
        Data frame containing one datapoint with one arbitrary value for each device.
    """
    data = {'timestamp': [dt.datetime.now()]}
    for i in range(n_dev):
        if data_type == 'power':
            data[f'dev_{i}'] = max(1 + 0.1 * np.random.randn(), 0)
        else:
            data[f'dev_{i}'] = np.random.randint(0, 2)

    dummy_df = pd.DataFrame(data=data)

    yield dummy_df


stream = start_display_server(n_dev=8)
