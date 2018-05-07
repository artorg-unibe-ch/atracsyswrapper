//
// Created by Iwan Paolucci on 30/04/2018.
//

#include "atracsyswrapper.h"
#include "helpers.hpp"
#include "geometryHelper.hpp"
#include "atracsysmarker.h"

AtracsysWrapper::AtracsysWrapper()
        : library(nullptr),
          device(nullptr) {
}

AtracsysWrapper::~AtracsysWrapper() {
	ftkDeleteFrame(frame);

    if (FTK_OK != ftkClose(&library)) {
        checkError(library);
    }
}

bool AtracsysWrapper::init() {
    library = ftkInit();
    if (library == nullptr) {
        return false;
    }

    device = std::make_unique<AtracsysDevice>(library);
    if (device->getType() == DEV_SPRYTRACK_180) {
        device->setOnboardProcessing(true);
        device->setSendingImages(false);
    }

    return true;
}

bool AtracsysWrapper::addGeometry(const std::string &filename, const std::string& geometryId) {
    ftkGeometry geometry{};
    bool success = false;
    switch (loadGeometry(library, device->getSerialNumber(), filename, geometry)) {
        case 1:            //cout << "Loaded from installation directory." << endl;
        case 0:
            if (ftkSetGeometry(library, device->getSerialNumber(), &geometry) != 0) {
                //checkError(*library);
            }
            geometries[geometryId] = geometry;
            markers[geometry.geometryId] = AtracsysMarker(geometry.geometryId);
			markers[geometry.geometryId].setName(geometryId);
            success = true;
            break;
        default:
            success = false;
            // error cannnot laod geometry file
    }
    return success;
}

bool AtracsysWrapper::startTracking() {
    frame = ftkCreateFrame();
    if ( frame == nullptr)
    {
        return false;       //error( "Cannot create frame instance" );
    }

    ftkError err( ftkSetFrameOptions( false, false, 16u, 16u, 0u, 16u, frame ) );

    if ( err != FTK_OK )
    {
        ftkDeleteFrame( frame );
//        checkError( *library );
    }
    return true;
}

bool AtracsysWrapper::stopTrackking() {
    return false;
}

void AtracsysWrapper::getMarkerPositions() {
    ftkError err = ftkGetLastFrame(library, device->getSerialNumber(), frame, 100u );
    if ( err != FTK_OK )
    {
        std::cout << "could not load frame" << std::endl;
        return;
    }

    if ( frame->markersCount == 0u )
    {
        std::cout << "no markers" << std::endl;
        return;
    }

    if ( frame->markersStat == QS_ERR_OVERFLOW )
    {
        std::cout << "marker overflow" << std::endl;
        return;
    }

    for ( int i = 0u; i < frame->markersCount; ++i )
    {
        ftkMarker marker = frame->markers[i];
		AtracsysMarker& atrMarker = markers[marker.geometryId];
        atrMarker.setGeometryPresenceMask(marker.geometryPresenceMask);
        atrMarker.setRegistrationError(marker.registrationErrorMM);
		AtracsysMarker::Transform transform = { { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 },{ 0, 0, 0, 1 } } };

		for(int i = 0; i < 3; ++i) {
			transform[i][0] = marker.rotation[i][0];
			transform[i][1] = marker.rotation[i][1];
			transform[i][2] = marker.rotation[i][2];
		}

		transform[0][3] = marker.translationMM[0];
		transform[1][3] = marker.translationMM[1];
		transform[2][3] = marker.translationMM[2];
        atrMarker.setTransform(transform);
    }
}

const std::map<size_t, AtracsysMarker>& AtracsysWrapper::getMarkers() const
{
	return markers;
}
