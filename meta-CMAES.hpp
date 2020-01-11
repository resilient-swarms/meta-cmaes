

#ifndef METAMAP_ELITE_HPP_
#define METAMAP_ELITE_HPP_

#include <algorithm>
#include <limits>
#include <array>

#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <Eigen/Dense>

#include <meta-cmaes/fit_bottom.hpp>

#include <meta-cmaes/fit_top.hpp>

#include <meta-cmaes/top_typedefs.hpp>

#include <sferes/ea/cmaes.hpp>
//#include <meta-cmaes/params.hpp>

namespace sferes
{

namespace ea
{
/* struct to store all of a bottom-level individual's information 
        -base-features
        -fitness
        -phenotype
*/

// we code meta-map-elites as simply map-elites with three differences:
// 1. the first features describe the top-level
// 2. sampling of parents is local to the current bottom-level map (switches every 5 generations)
// 3. genotype of top-level also evolves
// 4. keep track of existing solutions, refill them in the current map
const size_t MAP_REFRESH = 5;

// Main class
SFERES_EA(MetaCmaes, sferes::ea::Cmaes)
{
public:
    typedef boost::shared_ptr<phen_t> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef typename pop_t::iterator it_t;
    typedef typename std::vector<std::vector<indiv_t>> front_t;
    typedef boost::shared_ptr<phen_t> phen_ptr_t;

    const size_t num_maps = 5;


    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::dim;
    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::cmaes_t;
    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_evo;
    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_ar_funvals;
    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_cmaes_pop;
    using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_lambda;

    struct DataEntry
    {
        base_features_t base_features;
        float fitness; // bottom-level fitness
    };

    typedef std::vector<DataEntry> database_t;
    database_t database;

    MetaCmaes() 
    {
    }

    bottom_indiv_t entry_to_bottomindividual(const DataEntry &entry, const weight_t& weight)
    {
        
        // use weight and base features --> bottom-level features
        
        // create new individual
        base_phen_t individual = base_phen_t();
        //
        individual.fit().set_desc(individual.obtain_descriptor(weight, entry.base_features));
        individual.fit().set_value(entry.fitness);
        return individual;

    }

    /* fill map j with individuals */
    void fill_new_map(size_t j)
    {
        for (int i = 0; i < database.size(); ++i)
        {
            bottom_indiv_t individual = entry_to_bottomindividual(database[i], this->_pop[i]->gen().data());
            this->_pop[j]->_add_to_archive(individual);
        }
    }

    /* main loop here */
    void epoch()
    {
        // sample random individuals (new population of maps)
        _cmaes_pop = cmaes_SamplePopulation(&_evo);
        // copy pop
        for (size_t i = 0; i < this->_pop.size(); ++i)
        {
            for (size_t j = 0; j < this->_pop[i]->size(); ++j)
            {
                this->_pop[i]->gen().data(j, _cmaes_pop[i][j]);
                this->_pop[i]->develop();
            }
            fill_new_map(i); // after genotype is assigned, compute the resulting map accordingly
        }

        
        // eval
        this->_eval_pop(this->_pop, 0, this->_pop.size());
        this->apply_modifier();
        for (size_t i = 0; i < this->_pop.size(); ++i)
        {
            //warning: CMAES minimizes the fitness...
            _ar_funvals[i] = -this->_pop[i]->fit().value();
        }
        // update CMAES distribution
        cmaes_UpdateDistribution(&_evo, _ar_funvals);
    }

    void random_pop()
    {
        // do bottom-level random pop only, adding bottom-level individuals to the database
        database.resize(BottomParams::pop::init_size);
        BOOST_FOREACH (indiv_t indiv, database)
        {
            indiv = indiv_t();
            indiv->random();
        }
    }
};
} // namespace ea
} // namespace sferes

#endif