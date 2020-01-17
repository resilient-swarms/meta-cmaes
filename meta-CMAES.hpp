

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
        // void set_pop(const pop_t &p)
        // {
        //         _load();
        //         this->_pop = p;
        //         for (size_t i = 0; i < this->_pop.size(); ++i)
        //                 this->_pop[i]->develop();
        //         stc::exact(this)->_set_pop(p);
        // }
};
} // namespace ea
} // namespace sferes

#endif