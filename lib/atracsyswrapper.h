//
// Created by Iwan Paolucci on 30/04/2018.
//

#pragma once

#include <memory>
#include <string>
#include <ftkInterface.h>
#include <map>
#include "helpers.hpp"
#include "atracsysdevice.h"
#include "atracsysmarker.h"

class AtracsysWrapper {
public:
    AtracsysWrapper();

    virtual ~AtracsysWrapper();

    bool init();

    bool addGeometry(const std::string &filename, const std::string& geometryId);

    bool startTracking();
    bool stopTrackking();

    void getMarkerPositions();

	const std::map<size_t, AtracsysMarker>& getMarkers() const;
private:
    ftkLibrary library;
    std::unique_ptr<AtracsysDevice> device;
    std::map<std::string, ftkGeometry> geometries;
    std::map<size_t, AtracsysMarker> markers;
    ftkFrameQuery* frame;
};


