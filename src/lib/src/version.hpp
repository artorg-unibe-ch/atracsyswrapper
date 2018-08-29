#ifndef version_hpp
#define version_hpp

#include <string>

namespace samples2cams
{
    class AtracsysGitVersion
    {
    public:
        static const unsigned Major;
        static const unsigned Minor;
        static const unsigned Revision;
        static const unsigned Build;
        static const unsigned TimeStamp;
        static const std::string SoftwareVersion;
        static const std::string SoftwareVersionFull;
    };
}

#endif
