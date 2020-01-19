#ifndef EVAL_BOTTOM_HPP
#define EVAL_BOTTOM_HPP


#include <cmath>
#include <sferes/eval/eval.hpp>
#include <vector>
#include <meta-cmaes/feature_vector_typedefs.hpp>

namespace sferes
{
namespace eval
{

template <typename Fit>
class EvalBottom
{
public:
      
      EvalBottom() : _nb_evals(0) {}
      template <typename Indiv_t>
      float eval(std::vector<boost::shared_ptr<Indiv_t>> &pop, weight_t &W)
      {
            //dbg::trace trace("eval", DBG_HERE);
            assert(pop.size());
            _nb_evals = 0;
            float value = 0.0f;
            for (size_t i = 0; i < pop.size(); ++i)
            {
                pop[i]->fit() = Fit(W);
                value +=  eval_individual<Indiv_t>(pop[i]);
            }
            
#ifdef PRINTING
            std::cout << "number of evaluations is now " << _nb_evals << std::endl;
#endif
            return value;
      }
      unsigned nb_evals() const { return _nb_evals; }

protected:
      unsigned _nb_evals;
      template <typename Indiv>
      float eval_individual(boost::shared_ptr<Indiv> &ind)
      {
            
            ind->develop();
            ind->fit().eval<Indiv>(ind);
            ++_nb_evals;
            return ind->fit().value();
      }
};

}
}


#endif