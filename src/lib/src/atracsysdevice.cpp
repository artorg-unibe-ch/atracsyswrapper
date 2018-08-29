//
// Created by Iwan Paolucci on 04/05/2018.
//

#include "atracsysdevice.h"
#include "helpers.hpp"

static const unsigned ENABLE_ONBOARD_PROCESSING_OPTION = 6000;
static const unsigned SENDING_IMAGES_OPTION = 6003;


AtracsysDevice::AtracsysDevice(ftkLibrary library)
        : library(library) {
    DeviceData device( retrieveLastDevice( library ) );
    type = device.Type;
    serialNumber = device.SerialNumber;
}

AtracsysDevice::~AtracsysDevice() = default;


void AtracsysDevice::setOnboardProcessing(bool enable) {
    if ( ftkSetInt32( library, serialNumber, ENABLE_ONBOARD_PROCESSING_OPTION, 1 ) != FTK_OK )
    {
        //error( "Cannot process data directly on the SpryTrack." );
    }

}

void AtracsysDevice::setSendingImages(bool enable) {
    if ( ftkSetInt32( library, serialNumber, SENDING_IMAGES_OPTION, 0 ) != FTK_OK )
    {
        //error( "Cannot disable images sending on the SpryTrack." );
    }
}

ftkDeviceType AtracsysDevice::getType() const {
    return type;
}

uint64 AtracsysDevice::getSerialNumber() const {
    return serialNumber;
}

void AtracsysDevice::init() {

}
