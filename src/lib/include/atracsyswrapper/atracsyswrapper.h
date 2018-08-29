//
// Created by Iwan Paolucci on 29/08/2018.
//
#pragma once

#include <string>
#include <map>
#include <atracsyswrapper/atracsysmarker.h>

class AtracsysWrapper {
public:
    AtracsysWrapper() = default;;
    virtual ~AtracsysWrapper() = default;;

    virtual bool init() = 0;

    virtual bool addGeometry(const std::string &filename, const std::string& geometryId) = 0;

    virtual bool startTracking() = 0;
    virtual bool stopTrackking() = 0;

    virtual void getMarkerPositions() = 0;

    static std::unique_ptr<AtracsysWrapper> New();

    virtual const std::map<size_t, AtracsysMarker>& getMarkers() const = 0;
};
