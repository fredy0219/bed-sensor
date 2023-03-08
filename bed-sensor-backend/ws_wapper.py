import socketio
import eventlet
import threading
from flask import Flask
import time
from collections import deque

class WSWapper():
    def __init__(self, overview) -> None:
        self.sio = socketio.Server(cors_allowed_origins='*')
        self.app = Flask(__name__)
        self.app.logger.disabled = True

        self.device_command = deque()

        self.overview = overview

    def setup(self):
        self.call_backs()
        self._bedOverviewData = None

        wst = threading.Thread(target=self.serve_app, args=(self.sio, self.app))
        wst.daemon = True
        wst.start()

    def serve_app(self, _sio, _app):
        self.app = socketio.Middleware(_sio, _app)
        eventlet.wsgi.server(eventlet.listen(('localhost', 5000)), self.app)

    def call_backs(self):
        @self.sio.event
        def connect(sid, data):
            print("CONNECTION!!!")
        
        @self.sio.event
        def disconnect(sid):
            pass
        
        @self.sio.event
        def askBedDetail(sid, data):
            message = {
                'bedId' : data['bedId'],
                'data' : list(self.overview[data['bedId']]['object'].sensor_data)
            }
            self.sio.emit('getBedDetail', message)

        # System level react
        @self.sio.event
        def askBedOverview(sid, data):
            self.sio.emit('getBedOverview', [
                { 'number': 168, 'state': 'No signal' },
                { 'number': 169, 'state': 'On bed' },
                { 'number': 123, 'state': 'On bed' },
                { 'number': 171, 'state': 'Reposition' },
                { 'number': 172, 'state': 'Off' }
            ])
        
        # Device level react
        @self.sio.event
        def connectBedSensor(sid, data):
            # data['command'] : 'ON', 'OFF'
            print(data)
            if data['bedId'] and data['switch']:
                self.overview[data['bedId']]['device_command'] = data['switch']

    def set_overview_data(self, bedOverviewData):
        self._bedOverviewData = bedOverviewData

    def run(self):
        self.setup()

    # def get_device_command(self):
    #     if len(self.device_command) > 0:
    #         return self.device_command.popleft()
    #     return None

    
if __name__ == '__main__':
    wapper = WSWapper()
    wapper.run()

    while True:
        time.sleep(1);