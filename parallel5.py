import time
import argparse

import multiprocessing as mp
import cv2 as cv
from ultralytics import YOLO

class worker:
    def __init__(self,input_queue,output_list):
        self.input_queue = input_queue
        self.output_list = output_list
        self.model = YOLO("yolov8s-pose.pt",verbose=False).to('cpu')
        self.process = mp.Process(target=self.run)
        self.process.start()
    def run(self):
        while True:
            riba = time.time()
            num, img = self.input_queue.get()
            #print(time.time()-riba,end='\r')
            res = self.model(img,verbose=False)[0].plot()
            self.output_list.append((num,res))
    def stop(self):
        self.process.terminate()
    def __del__(self):
        self.stop()

def VideoToYolo(input, output=None, num_workers=1):
    global input_queue
    global output_list
    manager = mp.Manager()
    output_list = manager.list([])
    read_frame = 0
    current_frame = 0
    input_queue = mp.Queue(num_workers*2)
    output_queue = mp.Queue()
    cap = cv.VideoCapture(input)
    frame_width = int(cap.get(3)) 
    frame_height = int(cap.get(4))
    fps = cap.get(cv.CAP_PROP_FPS)
    result = cv.VideoWriter(output, cv.VideoWriter_fourcc(*'MJPG'), fps, (frame_width,frame_height))

    workers = []
    for i in range(num_workers):
        workers.append(worker(input_queue,output_list))

    while cap.isOpened():
        while not input_queue.full():
            ret, frame = cap.read()
            if ret:
                input_queue.put((read_frame,frame))
                read_frame += 1
            else:
                break
        if output_list:
            for index, element in enumerate(output_list):
                if element[0] == current_frame:
                    _, res = output_list.pop(index)
                    result.write(res)
                    print(f'written {current_frame}',end='\r')
                    current_frame += 1
        if not ret and current_frame == read_frame:
            print('no more frames i guess')
            break
    result.release()
    cap.release()
    
    for riba in workers:
        riba.stop()
 
parser = argparse.ArgumentParser()
parser.add_argument("--input",nargs=1)
parser.add_argument("--output",nargs=1)
parser.add_argument('--workers',nargs=1)
args = parser.parse_args()


InputPath = args.input[0]
OutputPath = args.output[0]
NumWorkers = int(args.workers[0])

if __name__ == '__main__':
    start = time.time()
    VideoToYolo(InputPath,OutputPath,num_workers=NumWorkers)
    print('время обработки видео ', time.time() - start)
