//
// Created by Iwan Paolucci on 04/05/2018.
//

#include "atracsysmarker.h"


AtracsysMarker::AtracsysMarker()
	:geometryId(-1)
{
}

AtracsysMarker::AtracsysMarker(size_t geometryId)
	:geometryId(geometryId),
transform{ { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 },{ 0, 0, 0, 1 } } }
         {

}

AtracsysMarker::~AtracsysMarker() = default;

size_t AtracsysMarker::getGeometryId() const {
    return geometryId;
}

size_t AtracsysMarker::getGeometryPresenceMask() const {
    return geometryPresenceMask;
}

void AtracsysMarker::setGeometryPresenceMask(size_t geometryPresenceMask) {
    AtracsysMarker::geometryPresenceMask = geometryPresenceMask;
}

float AtracsysMarker::getRegistrationError() const {
    return registrationError;
}

void AtracsysMarker::setRegistrationError(float registrationError) {
    AtracsysMarker::registrationError = registrationError;
}

const AtracsysMarker::Transform &AtracsysMarker::getTransform() const {
    return transform;
}

void AtracsysMarker::setTransform(const AtracsysMarker::Transform &transform) {
	this->transform = transform;
}

std::string AtracsysMarker::getName() const
{
	return name;
}

void AtracsysMarker::setName(const std::string& name)
{
	this->name = name;
}
