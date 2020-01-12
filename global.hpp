
#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <rhex_dart/safety_measures.hpp>
#include <rhex_dart/descriptors.hpp>

namespace global
{
    std::shared_ptr<rhex_dart::Rhex> global_robot;
    std::vector<rhex_dart::RhexDamage> damages;

}; // namespace global


#endif 
