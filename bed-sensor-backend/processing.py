import numpy as np
import asyncio
from Cubic_bezier import CubicBezier

class UneoProcessing:

    def __init__(self):
        self.isInitialized = False
        self.default_noise = np.zeros(80 * 3)
        self._sensor_data = np.zeros(80 * 3)
        self._alarm_score = np.zeros(80 * 3)

        self.cb = CubicBezier([0.0,0.33,0.66,1.0],
                                [0.0,0.8,0.8,1.0])

        asyncio.ensure_future(self.update_alarm_score())

    def set_default_noise(self, data):
        if not self.isInitialized:
            self.isInitialized = True
            self.default_noise = data

    @property
    def sensor_data(self):
        return self._sensor_data

    @sensor_data.setter
    def sensor_data(self, data):
        if not self.is_data_valid(data):
            sensor_data1 = data[6:86]
            sensor_data2 = data[6+110:86+110]
            sensor_data3 = data[6+220:86+220]
            sensor_data = np.array(sensor_data1 + sensor_data2 + sensor_data3)
            self.set_default_noise(sensor_data)
            sensor_data = (sensor_data - self.default_noise) / 255
            # self.bezier_curve(sensor_data)
            self.transfer2Cubic(sensor_data)
            sensor_data = np.clip(sensor_data, 0, 1)
            self._sensor_data = sensor_data

    @property
    def alarm_score(self):
        return self._alarm_score

    async def update_alarm_score(self):
        rising_interval = 0.05
        dropping_interval = 0.01
        while True:
            if self.isInitialized:
                self._alarm_score += [rising_interval if i > 0.005 else -dropping_interval for i in self.sensor_data]
                self._alarm_score = np.clip(self._alarm_score, 0, 1)
            await asyncio.sleep(1)
        
        
    def is_data_valid(self, data):

        prefix = [0x51, 0x4E, 0x45, 0xFF, 0xFF, 0xFF]
        postfix = [0x51, 0x4E, 0x45, 0xFF]

        sensor_data1 = data[0:110]
        sensor_data2 = data[110:220]
        sensor_data3 = data[220:330]

        prefix[0] = 0x51
        postfix[0] = 0x51
        valid1 = sensor_data1[0:6]== prefix and sensor_data1[106:110] == postfix

        prefix[0] = 0x52
        postfix[0] = 0x52
        valid2 = sensor_data2[0:6]== prefix and sensor_data2[106:110] == postfix

        prefix[0] = 0x53
        postfix[0] = 0x53
        valid3 = sensor_data3[0:6]== prefix and sensor_data3[106:110] == postfix

        return valid1 and valid2 and valid3

    def transfer2Cubic(self, data):
        data = [self.cb.position(d) for d in data]

    # # Bezier Curve
    # def one_bezier_curve(self, a,b,t):
    #     return (1-t)*a + t*b

    # #xs表示原始數據
    # #n表示階數
    # #k表示索引
    # def n_bezier_curve(self, xs, n, k, t):
    #     if n == 1:
    #         return self.one_bezier_curve(xs[k],xs[k+1],t)
    #     else:
    #         return (1-t)*self.n_bezier_curve(xs,n-1,k,t) + t*self.n_bezier_curve(xs,n-1,k+1,t)

    # def bezier_curve(self, data):
    #     ys = [0, 1.3, 0.8, 0.8, 1]
    #     ys = [i * 255 for i in ys]
    #     b_ys = []
    #     n = len(ys) - 1
    #     for d in data:
    #         b_ys.append(self.n_bezier_curve(ys,n,0,d))
    #     data = b_ys