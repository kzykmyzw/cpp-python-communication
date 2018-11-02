from keras.applications.vgg16 import VGG16, preprocess_input, decode_predictions
from keras.preprocessing import image
import numpy as np
import sys
import zmq
import cv2
import struct
import time

STATUS_RUN = 0
STATUS_STOP = 1

# VGG16
model = VGG16(weights='imagenet')

start = time.time()
while (1):
    # ZMQ configuration
    conn_str = "tcp://*:11000"
    ctx = zmq.Context()
    sock = ctx.socket(zmq.REP)
    sock.bind(conn_str)

    # Receive command
    status, filename = sock.recv_multipart()
    if struct.unpack('i', status)[0] == STATUS_STOP:
        print('Python has stopped.')
        break

    # Receive filename
    img = image.load_img('samples/' + filename.decode('UTF-8'), target_size=(224, 224))
    x = image.img_to_array(img)
    x = np.expand_dims(x, axis=0)
    preds = model.predict(preprocess_input(x))
    results = decode_predictions(preds, top=1)[0]

    # ZMQ configuration
    conn_str = "tcp://localhost:11001"
    ctx = zmq.Context()
    sock = ctx.socket(zmq.REQ)
    sock.connect(conn_str)

    # Send results
    data = [np.array([results[0][1].encode('UTF-8')]), np.array([results[0][2]])];
    sock.send_multipart(data)