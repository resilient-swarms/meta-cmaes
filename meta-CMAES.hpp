

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

#ifdef BINARY
#include <modules/map_elites/stat_map_binary.hpp>
#else
#include <modules/map_elites/stat_map.hpp>
#endif

#include <modules/map_elites/stat_progress.hpp>

#include <modules/map_elites/map_elites.hpp>

#include <meta-cmaes/fit_bottom.hpp>

#include <meta-cmaes/fit_top.hpp>


#include <meta-cmaes/phenotype.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
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
struct DataEntry
{
    base_features_t base_features;
    bottom_indiv_t individual; // bottom-level genotype, bottom-level fitness
};

// we code meta-map-elites as simply map-elites with three differences:
// 1. the first features describe the top-level
// 2. sampling of parents is local to the current bottom-level map (switches every 5 generations)
// 3. genotype of top-level also evolves
// 4. keep track of existing solutions, refill them in the current map
const size_t MAP_REFRESH = 5;

// Main class
SFERES_EA(MetaCmaes, Cmaes)
{
public:

    typedef boost::shared_ptr<MapElitesPhenotype> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef typename pop_t::iterator it_t;
    typedef typename std::vector<std::vector<indiv_t>> front_t;
    typedef boost::shared_ptr<MapElitesPhenotype> phen_ptr_t;
    static const size_t behav_dim = Params::ea::behav_dim;

    typedef std::array<float, behav_dim> point_t;
    typedef boost::multi_array<phen_ptr_t, behav_dim> array_t;
    typedef std::array<typename array_t::index, behav_dim> behav_index_t;

    behav_index_t behav_shape;

    const size_t num_maps = 5;
    

    std::vector<DataEntry> database_t;
    datahase_t data;

    MetaCmaes()
    {
    }


    bottom_indiv_t entry_to_bottomindividual(const DataEntry &entry)
    {
        bottom_indiv_t individual;
        individual->get_weights();
    }
 

    /* fill map j with individuals */
    void fill_new_map(size_t j)
    {
        for (int i = 0; i < database.size(); ++i)
        {
            bottom_indiv_t individual = entry_to_bottomindividual(database[i],this->_pop[i]->gen().data());
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
          for (size_t j = 0; j < this->_pop[i]->size(); ++j) {
            this->_pop[i]->gen().data(j, _cmaes_pop[i][j]);
            this->_pop[i]->develop();
          }
          fill_new_map(i); // after genotype is assigned, compute the resulting map accordingly
        }

        // evaluate the population of maps
        pop_t ptmp;
        ptmp.push_back(current_map);
        // eval
        this->_eval_pop(this->_pop, 0, this->_pop.size());
        this->apply_modifier();
        for (size_t i = 0; i < this->_pop.size(); ++i) {
          //warning: CMAES minimizes the fitness...
          _ar_funvals[i] = - this->_pop[i]->fit().value();
        }
        // update CMAES distribution
        cmaes_UpdateDistribution(&_evo, _ar_funvals);
    }


    void random_pop()
    {
        // do bottom-level random pop only, adding bottom-level individuals to the database
        database.resize(Params::pop::init_size);
        BOOST_FOREACH (indiv_t &indiv, database)
        {
            indiv = indiv_t(new indiv_t());
            indiv->random();
        }
    }
};
} // namespace ea
} // namespace sferes

#endif