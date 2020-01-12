
#ifndef FIT_TOP_HPP_
#define FIT_TOP_HPP_

#include <sferes/fit/fitness.hpp>
#include <meta-cmaes/global.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>

const size_t num_world_options = 10;

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/
namespace sferes{

namespace fit{

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
        dbg::trace trace("fit", DBG_HERE);

        ar &boost::serialization::make_nvp("_value", this->_value);
        ar &boost::serialization::make_nvp("_objs", this->_objs);
    }

    bool dead() { return _dead; }
    std::vector<double> weight() { return _ctrl; }

protected:
    bool _dead;
    std::vector<double> _ctrl;
    
    // sampling without replacement (see https://stackoverflow.com/questions/28287138/c-randomly-sample-k-numbers-from-range-0n-1-n-k-without-replacement)
    std::unordered_set<size_t> pickSet(size_t N, size_t k, std::mt19937& gen)
    {
        std::uniform_int_distribution<> dis(1, N);
        std::unordered_set<size_t> elems;

        while (elems.size() < k) {
            elems.insert(dis(gen));
        }

        return elems;
    }
    std::vector<size_t> pick(size_t N, size_t k) {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::unordered_set<size_t> elems = pickSet(N, k, gen);
        std::vector<size_t> result(elems.begin(), elems.end());
        return result;
    }

    // descriptor work done here, in this case duty cycle
    template <typename MetaIndiv>
    void _eval(MetaIndiv & meta_indiv)
    {
        float avg_fitness = 0;
        size_t num_individuals = std::max(1, (int)std::round(0.10 * meta_indiv._pop.size()));
        std::vector<size_t> individuals = pick(num_individuals, meta_indiv._pop.size());
        for(size_t individual: individuals)
        {
            bottom_indiv_t indiv = meta_indiv._pop[individual];
            for (size_t world_option = 0; world_option < num_world_options; ++world_option)
            {
                _eval_single_envir(indiv, world_option, avg_fitness);
            }
        }
        this->_value = avg_fitness / (float) (num_individuals*num_world_options); // no need to divide
        this->_dead = false;
        nb_evals = individuals.size() * num_world_options;
    }

    void _eval_single_envir(bottom_indiv_t indiv, size_t world_option, float &avg_fitness)
    {
        // copy of controller's parameters
        _ctrl.clear();

        for (size_t i = 0; i < 24; i++)
            _ctrl.push_back(indiv->gen().data(i));

        // launching the simulation
        auto robot = global::global_robot->clone();

        simulator_t simu(_ctrl, robot, world_option);
        simu.run(5); // run simulation for 5 seconds

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
}
}
#endif