
#ifndef FIT_TOP_HPP_
#define FIT_TOP_HPP_

#include <sferes/fit/fitness.hpp>
#include <meta-cmaes/global.hpp>
#include <meta-cmaes/mapelites_phenotype.hpp>

const size_t num_world_options = 10;

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/
namespace sferes
{

namespace fit
{

SFERES_FITNESS(FitTop, sferes::fit::Fitness)
{
public:
    size_t nb_evals = 0;
    /* current bottom-level map (new candidate to be added to _pop)*/
    template <typename MetaIndiv>
    void eval(MetaIndiv & indiv)
    {

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        this->_objs.resize(1);
        std::fill(this->_objs.begin(), this->_objs.end(), 0);
        this->_dead = false;
        _eval<MetaIndiv>(indiv);
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // std::cout << "Time difference = " <<     std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar &boost::serialization::make_nvp("_value", this->_value);
        ar &boost::serialization::make_nvp("_objs", this->_objs);
    }

    bool dead() { return _dead; }
    std::vector<double> weight() { return _ctrl; }

protected:
    bool _dead;
    std::vector<double> _ctrl;

    // descriptor work done here, in this case duty cycle
    template <typename MetaIndiv>
    void _eval(MetaIndiv & meta_indiv)
    {
        float avg_fitness = 0;
        std::vector<bottom_indiv_t> individuals = meta_indiv.sample_individuals();
        for (bottom_indiv_t &individual : individuals)
        {
            _eval_all(individual, avg_fitness);
        }
#ifdef EVAL_ENVIR
        this->_value = avg_fitness / (float)(individuals.size() * num_world_options); // no need to divide
#else
        this->_value = avg_fitness / (float)(individuals.size() * global::damage_sets.size()); // no need to divide
#endif
#ifdef PRINTING
        std::cout<< "Recovered performance: "<<this->_value << std::endl;
#endif
        this->_dead = false;
    }
#ifdef EVAL_ENVIR
    void _eval_all(const bottom_indiv_t &indiv, float &avg_fitness)
    {
#ifdef PRINTING
        std::cout << "start evaluating " << global::world_options.size() << " environments" << std::endl;
#endif
        for (size_t world_option = 0; world_option < global::world_options.size(); ++world_option)
        {
            _eval_single_envir(indiv, world_option, 0, avg_fitness);
        }
    }
#else
    void _eval_all(const bottom_indiv_t &indiv, float &avg_fitness)
    {
#ifdef PRINTING
        std::cout << "start evaluating " << global::damage_sets.size() << " damage sets" << std::endl;
#endif
        for (size_t i = 0; i < global::damage_sets.size(); ++i)
        {
            // initilisation of the simulation and the simulated robot, robot morphology currently set to raised.skel only
            _eval_single_envir(indiv, 0, i, avg_fitness);
        }
    }
#endif
    void _eval_single_envir(const bottom_indiv_t &indiv, size_t world_option, size_t damage_option, float &avg_fitness)
    {
        ++global::nb_evals;
        // copy of controller's parameters
        _ctrl.clear();

        for (size_t i = 0; i < 24; i++)
            _ctrl.push_back(indiv->gen().data(i));

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
            avg_fitness += 0;
            // do not update the descriptor !
        }
        else
        {
            // update the meta-fitness
            avg_fitness += fitness;
        }
    }
};
} // namespace fit
} // namespace sferes
#endif