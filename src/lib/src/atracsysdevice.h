//
// Created by Iwan Paolucci on 04/05/2018.
//

#pragma once


#include <ftkTypes.h>
#include <ftkInterface.h>
#include <memory>

class AtracsysDevice {
public:
    explicit AtracsysDevice(const ftkLibrary library);
    virtual ~AtracsysDevice();

    void init();

    void setOnboardProcessing(bool enable);
    void setSendingImages(bool enable);

    ftkDeviceType getType() const;

    uint64 getSerialNumber() const;

private:
    ftkLibrary library;
    uint64 serialNumber;
    ftkDeviceType type;
    bool allowSimulator = true;
};


