#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include "computer/computer.hpp"

namespace ss
{
    //Abstração do tipo byte
    typedef unsigned char byte;

    // Abstração para lista de computadores
    typedef std::vector<ss::computer> computers;
}

#endif //TYPES_HPP