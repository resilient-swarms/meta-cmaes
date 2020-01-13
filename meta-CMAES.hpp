

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
#include <sferes/ea/cmaes_interface.h> // to access parameter initialisation functions
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
#ifdef PRINTING
                std::cout << "initialise database " << std::endl;
#endif
                boost::shared_ptr<Phen> dummy_map(new Phen()); // create a meaningless map
                dummy_map->develop();
                for (size_t i = 0; i < BottomParams::pop::init_size; ++i)
                {
                        base_phen_t indiv = base_phen_t();

                        indiv.random();
                        indiv.develop();
                        indiv.fit() = FitBottom(dummy_map->W);
                        indiv.fit().eval<base_phen_t>(indiv);
                }
#ifdef PRINTING

                for (size_t k = 0; k < global::database.size(); ++k)
                {
                        std::cout << global::database[k].base_features << "\t" << global::database[k].fitness << std::endl;
                }
                std::cout << "CMAES random pop " << std::endl;
#endif
                this->_pop.resize(CMAESParams::pop::size);
                BOOST_FOREACH (boost::shared_ptr<Phen> &indiv, this->_pop)
                {
                        indiv = boost::shared_ptr<Phen>(new Phen());
                }
#ifdef PRINTING

                std::cout << "end CMAES random pop with population size " << this->_pop.size() << std::endl;
#endif
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