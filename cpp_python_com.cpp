#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <tchar.h>
#include <zmq.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <chrono>

using namespace std;

enum status {
	STATUS_RUN,
	STATUS_STOP,
};

vector<string> getFiles(const string& dir_name, const string& extension) {
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	vector<string> file_names;

	string search_name = dir_name + "\\*." + extension;

	hFind = FindFirstFile(search_name.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		throw runtime_error("file not found");
	}

	do {
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		}
		else {
			file_names.push_back(win32fd.cFileName);
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);

	return file_names;
}

static char *s_recv(void *socket) {
	char buffer[256];
	int size = zmq_recv(socket, buffer, 255, 0);
	if (size == -1) {
		return NULL;
	}
	if (size > 255) {
		size = 255;
	}
	buffer[size] = 0;

	return _strdup(buffer);
}

int main() {
	// Start Python process
	STARTUPINFO tStartupInfo = { 0 };
	PROCESS_INFORMATION tProcessInfomation = { 0 };

	TCHAR command[] = { _T("python classify.py") };
	BOOL bResult = CreateProcess(
		NULL
		, command
		, NULL
		, NULL
		, FALSE
		, 0
		, NULL
		, NULL
		, &tStartupInfo
		, &tProcessInfomation
	);

	// Image classification
	vector<string> files = getFiles("samples", "jpg");
	for (int i = 0; i < files.size(); i++) {
		auto start = chrono::system_clock::now();
		
		// ZMQ configuration
		zmq::context_t context_s(1);
		zmq::socket_t socket_s(context_s, ZMQ_REQ);
		socket_s.connect("tcp://localhost:11000");

		// Send command
		int status = STATUS_RUN;
		zmq::message_t cmd((void *)&status, sizeof(int), NULL);
		socket_s.send(cmd, ZMQ_SNDMORE);

		// Send file name
		zmq::message_t message(files[i].size());
		std::memcpy(message.data(), files[i].data(), files[i].size());
		socket_s.send(message);

		// ZMQ configuration
		zmq::context_t context_r(1);
		zmq::socket_t socket_r(context_r, ZMQ_REP);
		socket_r.bind("tcp://*:11001");
		zmq::message_t rcv_msg;

		// Receive results
		string class_name = s_recv(socket_r);
		socket_r.recv(&rcv_msg, 0);
		float score = *(float *)rcv_msg.data();

		auto end = chrono::system_clock::now();
		auto dur = end - start;
		auto msec = chrono::duration_cast<chrono::milliseconds>(dur).count();

		cout << files[i] << endl;
		cout << "  class : " << class_name << endl;
		cout << "  score : " << score << endl;
		cout << "  time  : " << msec << " [msec]" << endl << endl;
	}

	// ZMQ configuration
	zmq::context_t context_s(1);
	zmq::socket_t socket_s(context_s, ZMQ_REQ);
	socket_s.connect("tcp://localhost:11000");

	// Send stop command
	int status = STATUS_STOP;
	zmq::message_t cmd((void *)&status, sizeof(int), NULL);
	socket_s.send(cmd, ZMQ_SNDMORE);

	// dummy
	string dummy = "dummy";
	zmq::message_t message(dummy.size());
	std::memcpy(message.data(), dummy.data(), dummy.size());
	socket_s.send(message);
}