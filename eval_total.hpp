
#ifndef EVAL_TOTAL_HPP
#define EVAL_TOTAL_HPP

#include <boost/shared_ptr.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>
#include <sferes/eval/eval.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
#include <meta-cmaes/top_typedefs.hpp>
#include <sferes/phen/parameters.hpp>

// eval class which allows us to:
// 1.  develop the individual maps for a few generations after which they are evaluated by their recovered performance
// 2.  count the total number of function evaluations (including those for the recovered performance calc.)

namespace sferes
{
      namespace eval
      {
	          struct EvalStats
                  {
                        float best_metafitness;
                        float sd_metafitness;
                        float avg_metafitness;
                        float metagenotype_diversity;
			template<typename MetaIndiv>
                        void get_diversity(std::vector<boost::shared_ptr<MetaIndiv>> &pop)
                        {
                              metagenotype_diversity = 0.0f;
                              for (size_t i = 0; i < pop.size(); ++i)
                              {
                                    for (size_t j = i + 1; j < pop.size(); ++j)
                                    {
                                          metagenotype_diversity += pop[i]->dist(pop[j]);
                                    }
                              }
                        }
			template<typename MetaIndiv>
                        void set_stats(std::vector<boost::shared_ptr<MetaIndiv>> &pop)
                        {
                              float best_metafitness = -INFINITY;
                              float sd_metafitness = 0;
                              float avg_metafitness = 0;
                              for (size_t i = 0; i < pop.size(); ++i)
                              {
                                    float newfit = pop[i]->fit().value();
                                    if(newfit > best_metafitness )
                                    {
                                          best_metafitness = newfit;
                                    }
                                    avg_metafitness += newfit;
                              }
                              avg_metafitness/=(float) pop.size();
                              for (size_t i = 0; i < pop.size(); ++i)
                              {
                                    float deviation = (pop[i]->fit().value() - avg_metafitness);
                                    sd_metafitness+=(deviation*deviation);
                              }
                              sd_metafitness = std::sqrt(sd_metafitness/(float)pop.size());
                              get_diversity<MetaIndiv>(pop);
                        }
                  } eval_stats;
	    ParameterControl<EvalStats,phen_t,BottomParams,CMAESParams> *param_ctrl;
            SFERES_EVAL(EvalTotal, Eval)
            {
            public:
                  EvalTotal() {}

  

                  template <typename MetaIndiv>
                  void eval(std::vector<boost::shared_ptr<MetaIndiv>> & pop, size_t begin, size_t end,
                            const typename MetaIndiv::fit_t &fit_proto)
                  {
                        //dbg::trace trace("eval", DBG_HERE);
                        assert(pop.size());
                        assert(begin < pop.size());
                        assert(end <= pop.size());

                        for (size_t i = begin; i < end; ++i)
                        {
                              param_ctrl->phenotype = *pop[i];
                              param_ctrl->eval_stats = eval_stats;
                              size_t bot_epochs = param_ctrl->get_bottom_epochs();
#ifdef PRINTING
                              std::cout << "running the map for " + std::to_string(bot_epochs) + " epochs" << std::endl;
                              std::cout << typeid(pop[i]).name() << std::endl;
#endif
                              //pop[i]->develop();// already developed into a map, see cmaes.hpp
                              pop[i]->do_epochs(bot_epochs);
#ifdef PRINTING
                              std::cout << "evaluating meta-individual  " + std::to_string(i) << std::endl;
#endif
                              global::nb_evals += pop[i]->eval_individuals.nb_evals;
                              pop[i]->fit().percentage = param_ctrl->get_percentage_evaluated();
                              pop[i]->fit().eval<MetaIndiv>(*pop[i]); // evaluate its recovered performance
                              global::nb_evals += pop[i]->fit().nb_evals();
                        }
#ifdef PRINTING
                        std::cout << "number of evaluations is now " << _nb_evals << std::endl;
#endif
                  }
                  unsigned nb_evals() const { return _nb_evals; }

            protected:
                  unsigned _nb_evals;
                  unsigned _generations;
            };
      } // namespace eval
} // namespace sferes

#endif
