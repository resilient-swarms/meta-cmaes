

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

#include <meta-cmaes/cmaes.hpp>
//#include <sferes/ea/cmaes_interface.h> // to access parameter initialisation functions
//#include <meta-cmaes/params.hpp>

namespace sferes
{

namespace ea
{

// Main class
SFERES_EA(MetaCmaes, sferes::ea::Cmaes)
{
public:
        typedef boost::shared_ptr<phen_t> indiv_t;
        typedef typename std::vector<indiv_t> pop_t;

        MetaCmaes()
        {
        }

        void random_pop()
        {
#ifdef PRINTING
                std::cout << "initialise database " << std::endl;
#endif
                boost::shared_ptr<Phen> dummy_map(new Phen()); // create a meaningless map
                dummy_map->random_pop();                       // generate individuals which are then added to the databse
                
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
                        indiv->random();
                        indiv->develop();
                }
#ifdef PRINTING

                std::cout << "end CMAES random pop with population size " << this->_pop.size() << std::endl;
#endif
        }
        // for resuming; we don't need it as develop() takes care of initialising the maps again
        // furthermore: CMAES re-samples individuals and we halt based on top-level generations rather than
        // bottom-level generations
        // void _set_pop(const std::vector<boost::shared_ptr<Phen>> &pop)
        // {
        //         assert(!pop.empty());

        //         //        std::cout << this->res_dir() << " " << this->gen() << std::endl;

        //         //        std::string fname = ea.res_dir() + "/archive_" +
        //         //                boost::lexical_cast<std::string>(ea.gen()) +
        //         //                std::string(".dat");

        //         for (size_t h = 0; h < pop.size(); ++h)
        //         {
        //                 //            std::cout << "Fitness of ind " << h << " is " << pop[h]->fit().value() << std::endl;
        //                 //            std::cout << "Descriptor is " ; //<< pop[h]->fit().desc()[0] << std::endl;
        //                 //            for (size_t desc_index = 0; desc_index < pop[h]->fit().desc().size(); ++desc_index)
        //                 //                std::cout << pop[h]->fit().desc()[desc_index] << " ";
        //                 //            std::cout << std::endl;

        //                 //            pop[h]->develop();
        //                 //            pop[h]->fit().eval(*pop[h]);  // we need to evaluate the individuals to get the descriptor values

        //                 point_t p = get_point(pop[h]);

        //                 behav_index_t behav_pos;
        //                 for (size_t i = 0; i < Params::ea::behav_shape_size(); ++i)
        //                 {
        //                         behav_pos[i] = round(p[i] * behav_shape[i]);
        //                         behav_pos[i] = std::min(behav_pos[i], behav_shape[i] - 1);
        //                         assert(behav_pos[i] < behav_shape[i]);
        //                 }
        //                 _array(behav_pos) = pop[h];
        //         }
        // }
};
} // namespace ea
} // namespace sferes

#endif