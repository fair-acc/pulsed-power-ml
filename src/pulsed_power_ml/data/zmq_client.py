from threading import Thread
from typing import List

import numpy as np
import zmq

from src.pulsed_power_ml.data.zmq_connection_info import ZMQConnectionInfo


def chunks(lst, n):
    """
    Yield successive n-sized chunks from lst.

    Parameters
    ----------
    lst : List
        Some list.
    n : int
        Size of the chunks.

    Yields
    ------
    List
        The successive n-sized chunks.
    """

    for i in range(0, len(lst), n):
        yield lst[i:i + n]


class ZMQClient:
    def __init__(self, conn_info, proc_fun):
        """
        Set base info for ZMQClient
        
        Parameters
        ----------
        conn_info : ZMQConnectionInfo
        proc_fun : Callable
        """

        self.conn_info = conn_info
        self.proc_fun = proc_fun

    def run(self):
        """Run the ZMQClient in separate Thread"""

        Thread(target=self._execute, daemon=True).start()

    def _execute(self):
        """Subscribe to the given ZMQSocket"""

        context = zmq.Context()
        poller = zmq.Poller()

        subscriber = context.socket(zmq.SUB)
        subscriber.connect(f'{self.conn_info.connection_string}:{self.conn_info.port}')
        subscriber.subscribe(self.conn_info.topic)

        poller.register(subscriber, zmq.POLLIN)

        while True:
            message = subscriber.recv_multipart()
            parsed_msg = list(chunks(np.frombuffer(message[0], self.conn_info.data_type), self.conn_info.chan_cnt))
            self.proc_fun(parsed_msg)

        print('Stopping ZMQClient.')
        subscriber.close()
        context.term()
