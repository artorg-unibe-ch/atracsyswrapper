//
// Created by Iwan Paolucci on 07/05/2018.
//

#include "lib/atracsyswrapper.h"
#include <thread>
#include <chrono>

#include <igtl/igtlOSUtil.h>
#include <igtl/igtlServerSocket.h>
#include <igtl/igtlTrackingDataMessage.h>
#include <igtl/igtlMessageBase.h>

using namespace std::chrono_literals;

void printMarkers(std::map<size_t, AtracsysMarker> markers)
{
	for(auto entry : markers) {
		std::cout << std::endl;
		std::cout.precision(2u);
		std::cout << "geometry " << entry.first << std::endl;
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				std::cout << entry.second.getTransform()[i][j] << "\t";
			}
			std::cout << std::endl;
		}
		std::cout.precision(3u);
		std::cout << "error " << entry.second.getRegistrationError() << std::endl;
	}
}

int main(int argc, char **argv) {
    AtracsysWrapper wrapper;
    wrapper.init();

	igtl::ServerSocket::Pointer serverSocket;
	serverSocket = igtl::ServerSocket::New();
	int r = serverSocket->CreateServer(22222);

	igtl::Socket::Pointer socket;

    wrapper.addGeometry("geometry/geometry002.ini", "Pointer");
	wrapper.addGeometry("geometry/geometry003.ini", "Ultrasound");
    wrapper.startTracking();

	socket = serverSocket->WaitForConnection(10000);
	if (!socket.IsNotNull()) {
		std::cout << "No connection" << std::endl;
		return 0;
	}	

	for (int i = 0; i < 1000; ++i) {
		
		std::cout << "connected" << std::endl;
		wrapper.getMarkerPositions();
		auto markers = wrapper.getMarkers();

		//printMarkers(markers);

		igtl::TrackingDataMessage::Pointer transMsg;
		transMsg = igtl::TrackingDataMessage::New();

		for (auto entry : markers) {
			auto element = igtl::TrackingDataElement::New();
			element->SetName(entry.second.getName().c_str());
			
			igtl::Matrix4x4 matrix;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					matrix[i][j] = entry.second.getTransform()[i][j];
				}
				std::cout << std::endl;
			}
			
			element->SetMatrix(matrix);
			transMsg->AddTrackingDataElement(element);
		}

		transMsg->SetTimeStamp(igtl::TimeStamp::New());
		transMsg->Pack();
		//transMsg->Print(std::cout);
		socket->Send(transMsg->GetPackPointer(), transMsg->GetPackSize());

		std::this_thread::sleep_for(50ms);
	}
	serverSocket->CloseSocket();
	wrapper.stopTrackking();
}
