//
// Created by Iwan Paolucci on 04/05/2018.
//

#pragma once

#include <ftkTypes.h>
#include <cstddef>
#include <array>

class AtracsysMarker {
public:
	AtracsysMarker();
    explicit AtracsysMarker(size_t geometryId);
    virtual ~AtracsysMarker();

    typedef std::array<std::array<float, 4>, 4> Transform;

    size_t getGeometryId() const;

    size_t getGeometryPresenceMask() const;
    void setGeometryPresenceMask(size_t geometryPresenceMask);

    float getRegistrationError() const;
    void setRegistrationError(float registrationError);

	const Transform &getTransform() const;
	void setTransform(const Transform &transform);


	std::string getName() const;
	void setName(const std::string& name);
private:
	std::string name;
    size_t geometryId;
    size_t geometryPresenceMask;
	Transform transform;
    float registrationError;
};


