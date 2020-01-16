
#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <rhex_dart/safety_measures.hpp>
#include <rhex_dart/descriptors.hpp>
#include <rhex_dart/rhex.hpp>

namespace global
{
std::shared_ptr<rhex_dart::Rhex> global_robot;
#ifdef EVAL_ENVIR
    std::vector<size_t> world_options;
#else
    std::vector<std::shared_ptr<rhex_dart::Rhex>> damaged_robots;
    std::vector<std::vector<rhex_dart::RhexDamage>> damage_sets =   {
        {
            rhex_dart::RhexDamage("leg_removal", "15")
        }
    };

#endif
void init_simu(std::string robot_file)
{
    global::global_robot = std::make_shared<rhex_dart::Rhex>(robot_file, "Rhex", false, std::vector<rhex_dart::RhexDamage>()); // we repeat this creation process for damages
#ifdef EVAL_ENVIR

    
#else
    for (size_t i = 0; i < global::damage_sets.size(); ++i)
    {
        global::damaged_robots.push_back(std::make_shared<rhex_dart::Rhex>(robot_file, "Rhex", false, global::damage_sets[i])); // we repeat this creation process for damages
    }
#endif
}
}; // namespace global

#endif