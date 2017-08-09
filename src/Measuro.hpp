#ifndef MEASURO_HPP
#define MEASURO_HPP

#include <string>

namespace measuro
{
    static void version(unsigned int & major, unsigned int & minor, unsigned int & release)
    {
        major = @LIB_VERSION_MAJOR@;
        minor = @LIB_VERSION_MINOR@;
        release = @LIB_VERSION_REL@;
    }

    static void version_text(std::string & version_str)
    {
        version_str = "@LIB_VERSION_MAJOR@.@LIB_VERSION_MINOR@-@LIB_VERSION_REL@";
    }
}

#endif
