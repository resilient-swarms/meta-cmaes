
#ifndef FIT_TOP_HPP_
#define FIT_TOP_HPP_

#include <sferes/fit/fitness.hpp>
#include <meta-cmaes/global.hpp>
#include <meta-cmaes/mapelites_phenotype.hpp>
#include <meta-cmaes/eval_meta.hpp>


// typedef
    


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
    
    /* current bottom-level map (new candidate to be added to _pop)*/
    template <typename MetaIndiv>
    void eval(MetaIndiv & indiv)
    {

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        this->_objs.resize(1);
        std::fill(this->_objs.begin(), this->_objs.end(), 0);
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

    bool dead() { return false; }
#ifdef EVAL_ENVIR
    static float _eval_all(const base_phen_t &indiv)
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
    static float _eval_all(const base_phen_t &indiv)
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
    std::tuple<float,size_t> avg_value(float val, size_t num_individuals)
    {

#ifdef EVAL_ENVIR
        _nb_evals = num_individuals * global::world_options.size(); // no need to divide
#else
        _nb_evals = num_individuals * global::damage_sets.size(); // no need to divide
#endif
        val = val / (float)(_nb_evals);
        return std::tuple<float,size_t>{val,_nb_evals};
    }

    inline void set_fitness(float fFitness)
    {
#if CONTROL()
        this->_objs.resize(1);
        this->_objs[0] = fFitness;
#endif
        this->_value = fFitness;
    }

    size_t nb_evals()
    {
        return _nb_evals;
    }

protected:
    size_t _nb_evals = 0;
    // descriptor work done here, in this case duty cycle
    template <typename MetaIndiv>
    void _eval(MetaIndiv & meta_indiv)
    {
        float avg_fitness = 0.0f;
#ifdef PARALLEL_RUN
        typedef sferes::eval::_eval_parallel_meta<MetaIndiv, sferes::fit::FitTop<CMAESParams>> top_eval_helper_t;
#else
        typedef sferes::eval::_eval_serial_meta<MetaIndiv, sferes::fit::FitTop<CMAESParams>> top_eval_helper_t;
#endif
        auto helper = top_eval_helper_t(meta_indiv); //allow parallelisation over individuals (_parallel_eval_meta)
        set_fitness(helper.value);
        _nb_evals = helper.nb_evals;
#ifdef PRINTING
        std::cout << "recovered performance " << this->_value << std::endl;
#endif
    }
    static float _eval_single_envir(const base_phen_t &indiv, size_t world_option, size_t damage_option)
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
};
} // namespace fit
} // namespace sferes
#endif