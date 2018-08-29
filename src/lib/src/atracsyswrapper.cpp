//
// Created by Iwan Paolucci on 29/08/2018.
//

#include <atracsyswrapper/atracsyswrapper.h>
#include "atracsyswrapperimpl.h"

std::unique_ptr<AtracsysWrapper> AtracsysWrapper::New() {
    return std::make_unique<AtracsysWrapperImpl>();
}
