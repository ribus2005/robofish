import time
import logging
import argparse

import queue
import threading
import cv2
import numpy as np


class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")

formatter = logging.Formatter("%(name)s %(asctime)s %(levelname)s %(message)s")

class SensorCam(Sensor): 
    
    def __init__(self,name,resolution):
        self.logger = logging.getLogger('camlogger')
        self.logger.setLevel(logging.INFO)
        camhandler = logging.FileHandler("log/camlog.txt", mode='w')
        camhandler.setFormatter(formatter)
        self.logger.addHandler(camhandler)
        self.logger.info(f"starting SensorCam module for {name} with {resolution}")
        self.most_recent_frame = np.zeros((resolution[0],resolution[1],3))
        self.resolution = resolution
        self.name = name
        self.ok = True
        if resolution[0] <= 1 or resolution[1] <= 1:
            self.ok = False 
            self.logger.critical(f"incorrect resolution {resolution} for camera {self.name}")
        self.cam = cv2.VideoCapture(name)
        if self.cam.isOpened() == False:
            self.ok = False
            self.logger.critical(f"unable to open camera {self.name}")
        self.cam.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
        self.cam.set(cv2.CAP_PROP_FRAME_HEIGHT, resolution[1])
        self.shutdown = False
        self.thread = threading.Thread(target=self.run)
        self.thread.start()
    def run(self):
        while True:
            if self.shutdown or not self.ok:
                break
            try: 
                ret, frame = self.cam.read()
            except Exception:
                self.ok = False
            if ret:
                frame = cv2.resize(frame,(self.resolution[0],self.resolution[1]),interpolation = cv2.INTER_LINEAR)
                self.most_recent_frame = frame
            else:
                self.ok = False
                self.logger.error(f'unable to read feed from {self.name}')
                break
                
    def get(self):
        return (self.ok, self.most_recent_frame)
        
    def __del__(self):
        self.shutdown = True
        self.thread.join()
        self.cam.release()
        self.logger.info(f"finished reading from camera {self.name}")

sensornum = 0
sensorhandler = logging.FileHandler("log/sensorlog.txt", mode='w')
sensorhandler.setFormatter(formatter)

class SensorX(Sensor):
    '''Sensor X'''
    def __init__(self, delay: float):
        self._delay = delay 
        self._data = 0
    
    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data

class CringeX():
    
    def __init__(self,frequency):
        global sensorhandler
        global sensornum
        self.logger = logging.getLogger('sensorlogger')
        self.logger.setLevel(logging.INFO)
        self.logger.addHandler(sensorhandler)
        self.number = sensornum
        sensornum += 1
        self.logger.info(f"starting SensorX module number {self.number}")
        self.ok = True
        self.shutdown = False
        if frequency <= 0: 
            self.logger.critical(f"incorrect frequency {frequency} for SensorX {self.number}")
            self.ok = False
        else:
            self.delay = 1.0 / frequency
            self.sensor = SensorX(self.delay)
        self.value = 0
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def run(self):
        while True:
            if self.shutdown or not self.ok:
                break
            self.value = self.sensor.get()
           

    def get(self):
        return self.value

    def __del__(self):
        self.shutdown = True
        self.thread.join()
        self.logger.info(f"finished work for sensor {self.number}")

windownum = 0

class WindowImage():
    
    def __init__(self,frequency):
        global windownum
        self.logger = logging.getLogger('windowlogger')
        self.logger.setLevel(logging.INFO)
        winhandler = logging.FileHandler("log/windowlog.txt", mode='w')
        winhandler.setFormatter(formatter)
        self.logger.addHandler(winhandler)
        self.logger.info(f"starting WindowImage module with frequency {frequency}")
        self.ok = True
        if frequency <= 0:
            self.logger.critical(f"incorrect frequency {frequency} for window {windownum}")
            self.ok = False
            return
        else:
            self.delay = 1.0 / frequency
        cv2.namedWindow('window '+ str(windownum), cv2.WINDOW_AUTOSIZE)
        self.name = 'window '+ str(windownum)
        windownum += 1
        self.last_time = time.time()

    def show(self,image,values=[]):
        if self.delay - (time.time() - self.last_time) > 0: 
            time.sleep(self.delay - (time.time() - self.last_time))
        self.last_time = time.time()
        frame = cv2.rectangle(image,(image.shape[1] - 280,image.shape[0] - 40*len(values) - 30),(image.shape[1],image.shape[0]),(255,255,255),-1) 
        for i in range(len(values)):
            frame = cv2.putText(frame,f"sensor {i}: {values[i]}",(image.shape[1] - 250,image.shape[0] - (i+1) * 40),cv2.FONT_HERSHEY_SIMPLEX,1,(0,0,0),2,cv2.LINE_AA)
        cv2.imshow(self.name,frame)
                
    def __del__(self):
        if cv2.getWindowProperty(self.name, 0):
            cv2.destroyWindow(self.name)
        self.logger.info(f'finished showing for {self.name}')

parser = argparse.ArgumentParser()
parser.add_argument("--name",const = 0,nargs='?')
parser.add_argument("--width",const = 500,nargs='?')
parser.add_argument("--height",const = 500,nargs='?')
parser.add_argument("--fps",const = 30,nargs='?')
args = parser.parse_args()
print(args.name)
print(args.width)
print(args.height)
print(args.fps)

riba = SensorCam(int(args.name),(int(args.width),int(args.height)))
pivo = WindowImage(int(args.fps))
kamen1 = CringeX(100)
kamen2 = CringeX(10)
kamen3 = CringeX(1)
while True:
    val1 = kamen1.get()
    val2 = kamen2.get()
    val3 = kamen3.get()
    ok, pic = riba.get()
    if not ok or not pivo.ok or not kamen1.ok or not kamen2.ok or not kamen3.ok or cv2.waitKey(1) == ord('q'):
        break
    pivo.show(pic,[val1,val2,val3])
pivo.__del__()
riba.__del__()
kamen1.__del__()
kamen2.__del__()
kamen3.__del__()