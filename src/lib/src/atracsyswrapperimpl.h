//
// Created by Iwan Paolucci on 30/04/2018.
//

#pragma once

#include <memory>
#include <string>
#include <ftkInterface.h>
#include <map>
#include <atracsyswrapper/atracsyswrapper.h>
#include "atracsysdevice.h"
#include "atracsyswrapper/atracsysmarker.h"

class AtracsysWrapperImpl : public AtracsysWrapper {
public:
    AtracsysWrapperImpl();

    ~AtracsysWrapperImpl() override;

    bool init() override;

    bool addGeometry(const std::string &filename, const std::string& geometryId) override;

    bool startTracking() override;
    bool stopTrackking() override;

    void getMarkerPositions() override;

	const std::map<size_t, AtracsysMarker>& getMarkers() const;
private:
    ftkLibrary library;
    std::unique_ptr<AtracsysDevice> device;
    std::map<std::string, ftkGeometry> geometries;
    std::map<size_t, AtracsysMarker> markers;
    ftkFrameQuery* frame;
};


