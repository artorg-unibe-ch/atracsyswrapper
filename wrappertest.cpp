//
// Created by Iwan Paolucci on 07/05/2018.
//

#include "lib/atracsyswrapper.h"
#include "connectionlistener.h"
#include <thread>
#include <chrono>

#include <igtl/igtlOSUtil.h>
#include <igtl/igtlServerSocket.h>
#include <igtl/igtlTrackingDataMessage.h>
#include <igtl/igtlMessageBase.h>
#include <conio.h>

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

    ConnectionListener cl;
    cl.init();

    wrapper.addGeometry("geometry/geometry002.ini", "Pointer");
	wrapper.addGeometry("geometry/geometry003.ini", "Ultrasound");
    wrapper.startTracking();

	bool run = true;
	while (run) {
		wrapper.getMarkerPositions();
		auto markers = wrapper.getMarkers();

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
			}
			
			element->SetMatrix(matrix);
			transMsg->AddTrackingDataElement(element);
		}

		transMsg->SetTimeStamp(igtl::TimeStamp::New());
		transMsg->Pack();
		cl.send(transMsg);
		std::this_thread::sleep_for(20ms);
		if(_kbhit()) {
			char key = _getch();
			if(key == 'q') {
				run = false;
			}
		}
	}

	wrapper.stopTrackking();
}
