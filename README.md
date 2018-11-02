# C++ <-> Python using ZeroMQ
Interprocess communication between C++ and Python using ZeroMQ


## Setup

### Prerequisites
- Windows
- Visual Studio
- Python3.6
- OpenCV
- Tensorflow and Keras

### Getting Started
#### For C++
1. Clone [ZeroMQ](https://github.com/zeromq/libzmq)
1. Use CMake to generate VS solution file
1. Build the solution file to generate libs and dlls
1. Download ZeroMQ C++ wrapper [cppzmq](https://github.com/zeromq/cppzmq) (zmq.hpp is needed)
#### For Python
Install zmq
```
pip install zmq
```

## Run
1. Compile `cpp_python_com.cpp` to generate an execution file
1. Put images in `sample` directory
1. Run the execution file
1. `classify.py` returns VGG16 classification results for the images
