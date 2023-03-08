# -- coding: utf-8 --**
from collections import deque
import json
import socket
import select
from collections import deque
import asyncio

# TCP HOST setting
HOST_IP = '192.168.0.102'
HOST_SEND_PORT = 8000
HOST_RECV_PORT = 7000

# TCP Client info
CLIENT_IP = '192.168.0.100'
CLIENT_PORT = 9999

class TcpWapper():
    def __init__(self, overview) -> None:
        
        self.recv_sockets = []
        self.send_sockets = []

        self.connection_id_list = {}
        self.overview = overview

        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server.bind((HOST_IP, HOST_RECV_PORT))
        self.server.listen(5)
        self.recv_sockets.append(self.server)

        self.tasks = []
        self.tasks.append(asyncio.ensure_future(self.service_select()))

        self.devic_dict = {}
        self.device_command = deque()

    def get_tasks(self):
        return self.tasks

    async def service_select(self):

        empty = []
        while True:
            readable, writable, exceptional = select.select(self.recv_sockets, self.send_sockets, empty)
            for r in readable:
                if r is self.server:
                    print("new connection from tcp")
                    connection, client_address = r.accept()
                    connection.setblocking(0)
                    self.recv_sockets.append(connection)
                    self.send_sockets.append(connection)
                else:
                    data_buffer = b''
                    continue_recv = True # if no ENDOFJSON flag read, keep recv
                    able_to_combine = False # if STARTOFJSON read, start combine data
                    packet_count = 0
                    try:
                        while continue_recv:
                            data = r.recv(1024)
                            
                            # recv 0 length of byte means lost connection
                            # remvoe connection
                            if len(data) == 0:
                                id = self.connection_id_list[r]
                                self.overview[str(id)]['connection'] = None
                                self.connection_id_list.pop(r)
                                self.recv_sockets.remove(r)
                                self.send_sockets.remove(r)
                                r.close()
                                break;
                            
                            if data[:11] == b'STARTOFJSON':
                                able_to_combine = True

                            if data[-9:] == b'ENDOFJSON':
                                continue_recv = False

                            if able_to_combine:
                                packet_count += 1
                                data_buffer += data
                            
                            if not continue_recv:
                                data_buffer = data_buffer[11:-9]
                                self.socket_data_handler(data_buffer, r)
                        
                    except Exception as e:
                        # pass
                        print(e)

                for w in writable:
                    try:
                        id = self.connection_id_list[w]
                        if self.overview[id]['device_command']:
                            message = {
                                'id': id,
                                'command': self.overview[id]['device_command']
                            }
                            message_b = json.dumps(message)
                            w.send(message_b.encode())
                            self.overview[id]['device_command'] = None
                    except Exception as e:
                        print(e)
                    

            await asyncio.sleep(0.1)

            # for w in writable:

    def socket_data_handler(self, data, connection):
        # print(data)
        try:
            j_data = json.loads(data)
            # print(j_data)
            if j_data['command'] == 'register':
                self.overview[str(j_data['id'])]['connection'] = connection
                self.connection_id_list[connection] = str(j_data['id']);

            elif j_data['command'] == 'streaming':
                if len(j_data['data']) == 330:
                    self.overview[str(j_data['id'])]['object'].sensor_data =  j_data['data']
        
        except Exception as e:
            print(e)


    def add_device_command(self, command):
        self.device_command.append(command)
        if self.send_commands[command['bedId']]:
            self.send_commands[command['bedId']]['command'] = command['switch']

    # def device_command_handler(self):
    #     if(len(self.device_command)):
    #         command = self.device_command.popleft()
    #         if self.connections[command['bedId']]:
    #             self.send_sockets.append()

    def close(self):
        for r in self.recv_sockets:
            r.close()
            
        

if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    tcp_wapper = TcpWapper(loop)
    tcp_wapper.run()
    loop.run_forever()
    