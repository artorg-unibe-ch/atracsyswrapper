//
// Created by Iwan Paolucci on 30/04/2018.
//

#include "atracsyswrapperimpl.h"
#include "helpers.hpp"
#include "geometryHelper.hpp"
#include "atracsyswrapper/atracsysmarker.h"

AtracsysWrapperImpl::AtracsysWrapperImpl()
        : AtracsysWrapper(),
        library(nullptr),
          device(nullptr) {
}

AtracsysWrapperImpl::~AtracsysWrapperImpl() {
	ftkDeleteFrame(frame);

    if (FTK_OK != ftkClose(&library)) {
        checkError(library);
    }
}

bool AtracsysWrapperImpl::init() {
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

bool AtracsysWrapperImpl::addGeometry(const std::string &filename, const std::string& geometryId) {
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

bool AtracsysWrapperImpl::startTracking() {
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

bool AtracsysWrapperImpl::stopTrackking() {
    return false;
}

void AtracsysWrapperImpl::getMarkerPositions() {
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

    for ( uint32 i = 0; i < frame->markersCount; ++i )
    {
        ftkMarker marker = frame->markers[i];
		AtracsysMarker& atrMarker = markers[marker.geometryId];
        atrMarker.setGeometryPresenceMask(marker.geometryPresenceMask);
        atrMarker.setRegistrationError(marker.registrationErrorMM);
		AtracsysMarker::Transform transform = { { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 },{ 0, 0, 0, 1 } } };

		for(int k = 0; k < 3; ++k) {
			transform[k][0] = marker.rotation[k][0];
			transform[k][1] = marker.rotation[k][1];
			transform[k][2] = marker.rotation[k][2];
		}

		transform[0][3] = marker.translationMM[0];
		transform[1][3] = marker.translationMM[1];
		transform[2][3] = marker.translationMM[2];
        atrMarker.setTransform(transform);
    }
}

const std::map<size_t, AtracsysMarker>& AtracsysWrapperImpl::getMarkers() const
{
	return markers;
}
