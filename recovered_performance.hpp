
#ifndef RECOVERED_PERFORMANCE_HPP
#define RECOVERED_PERFORMANCE_HPP

#include <meta-cmaes/global.hpp>

namespace sferes
{

namespace fit
{

template<typename Phen>
struct RecoveredPerformance
{
    static float _eval_single_envir(const Phen &indiv, size_t world_option, size_t damage_option)
    {
        // copy of controller's parameters
        std::vector<double> _ctrl;
        _ctrl.clear();

        for (size_t i = 0; i < 24; i++)
            _ctrl.push_back(indiv.gen().data(i));

#ifdef EVAL_ENVIR
        // launching the simulation
        auto robot = global::global_robot->clone();
        simulator_t simu(_ctrl, robot, world_option);
#else
        auto robot = global::damaged_robots[damage_option]->clone();
        simulator_t simu(_ctrl, robot, world_option, 1.0, global::damage_sets[damage_option]);
#endif

        simu.run(BottomParams::simu::time); // run simulation for the same amount of time as the bottom level, to keep function evals comparable
        float fitness = simu.covered_distance();

        // these assume a behaviour descriptor of size 6.
        if (fitness < -1000)
        {
            // this means that something bad happened in the simulation
            // we do not kill the individual in the meta-map, but set fitness to zero and bd does not contribute
            return 0.0; // will not count towards the sum
            // do not update the descriptor !
        }
        else
        {
            // update the meta-fitness
            return fitness;
        }
    }

#ifdef EVAL_ENVIR
    static float _eval_all(const Phen &indiv)
    {
#ifdef PRINTING
        std::cout << "start evaluating " << global::world_options.size() << " environments" << std::endl;
#endif
        float val = 0.0f;
        for (size_t world_option : global::world_options)
        {
            val += _eval_single_envir(indiv, world_option, 0);
        }
        return val;
    }
#else
    static float _eval_all(const Phen &indiv)
    {
#ifdef PRINTING
        std::cout << "start evaluating " << global::damage_sets.size() << " damage sets" << std::endl;
#endif
        float val = 0.0f;
        for (size_t i = 0; i < global::damage_sets.size(); ++i)
        {
            // initilisation of the simulation and the simulated robot, robot morphology currently set to raised.skel only
            val += _eval_single_envir(indiv, 0, i);
        }
        return val;
    }
#endif
};

}
}


#endif