#pragma once

#include <iostream>
#include <exception>

namespace TagHierarchyUtil {
    struct StopTraversing : public std::exception
    {
        const char * what () const throw ()
        {
            return "C++ Exception";
        }
    };
}
