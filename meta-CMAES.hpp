

#ifndef METAMAP_ELITE_HPP_
#define METAMAP_ELITE_HPP_

#include <algorithm>
#include <limits>
#include <array>

#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>




#include <meta-cmaes/fit_top.hpp>

#include <meta-cmaes/top_typedefs.hpp>

#include <meta-cmaes/global.hpp>

#include <sferes/ea/cmaes.hpp>
#include <sferes/ea/cmaes_interface.h>  // to access parameter initialisation functions
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

    const size_t num_maps = 5;


    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::dim;
    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::cmaes_t;
    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_evo;
    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_ar_funvals;
    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_cmaes_pop;
    // using sferes::ea::Cmaes<phen_t,eval_t, stat_t, modifier_t,CMAESParams>::_lambda;


   

    MetaCmaes() 
    {
    }





    // /* main loop here */
    // void epoch()
    // {
    //     // sample random individuals (new population of maps)
    //     _cmaes_pop = cmaes_SamplePopulation(&_evo);
    //     // copy pop
    //     for (size_t i = 0; i < this->_pop.size(); ++i)
    //     {
    //         for (size_t j = 0; j < this->_pop[i]->size(); ++j)
    //         {
    //             this->_pop[i]->gen().data(j, _cmaes_pop[i][j]);
    //             this->_pop[i]->develop();
    //         }
    //         fill_new_map(i); // after genotype is assigned, compute the resulting map accordingly
    //     }

        
    //     // eval
    //     this->_eval_pop(this->_pop, 0, this->_pop.size());
    //     this->apply_modifier();
    //     for (size_t i = 0; i < this->_pop.size(); ++i)
    //     {
    //         //warning: CMAES minimizes the fitness...
    //         _ar_funvals[i] = -this->_pop[i]->fit().value();
    //     }
    //     // update CMAES distribution
    //     cmaes_UpdateDistribution(&_evo, _ar_funvals);
    // }

    void random_pop()
    {
        // do bottom-level random pop only, adding bottom-level individuals to the database
        weight_t w = weight_t::Random(); // we do not care about the weights here
        for(size_t i=0; i< BottomParams::pop::init_size; ++i)
        {
            base_phen_t indiv = base_phen_t();
            
            indiv.random();
            indiv.develop();
            indiv.fit() = FitBottom(w);
            indiv.fit().eval<base_phen_t>(indiv);
        }


        
    }
    // for resuming; we don't need it as develop() takes care of initialising the maps again 
    // furthermore: CMAES re-samples individuals and we halt based on top-level generations rather than 
    // bottom-level generations
    // void _set_pop(const std::vector<boost::shared_ptr<Phen> >& pop) {

    // }
};
} // namespace ea
} // namespace sferes

#endif