from http import client
from processing import UneoProcessing
from ws_wapper import WSWapper
from tcp_wapper import TcpWapper
import asyncio
import time

class BedSensorController():
    def __init__(self) -> None:
        self.loop = asyncio.get_event_loop()

        self.tasks = []
        
        # state
        # object : UneoProcessing
        # connection : tcp connection
        self.overview = {}
        self.overview['168'] = {'state':'No signal', 'object': UneoProcessing(), 'device_command':None}
        self.overview['169'] = {'state':'No signal', 'object': UneoProcessing(), 'device_command':None}
        self.overview['170'] = {'state':'No signal', 'object': UneoProcessing(), 'device_command':None}
        self.overview['171'] = {'state':'No signal', 'object': UneoProcessing(), 'device_command':None}
        self.overview['172'] = {'state':'No signal', 'object': UneoProcessing(), 'device_command':None}

        self.ws_wapper = WSWapper(self.overview)
        self.tcp_wapper = TcpWapper(self.overview)
        
    def run(self):
        self.ws_wapper.run()
        self.tasks = self.tasks + self.tcp_wapper.get_tasks()
        
        try:
            self.loop.run_until_complete(asyncio.wait(self.tasks))
        except KeyboardInterrupt as e:
            for t in self.tasks:
                t.cancel()
            self.tcp_wapper.close()
        finally:
            self.loop.close()

if __name__ == '__main__':
    controller = BedSensorController()
    controller.run()