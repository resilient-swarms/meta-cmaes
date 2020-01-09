#include <boost/shared_ptr.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

// eval class which allows us to:
// 1. 
// 2.  count the total number of function evaluations

namespace sferes {
  namespace eval {
    const size_t bottom_epochs = 10;
    SFERES_EVAL(EvalTotal) {
    public:
      EvalTotal() : _nb_evals(0) {}
      template<typename Phen>
      void eval(std::vector<boost::shared_ptr<Phen> >& pop, size_t begin, size_t end,
                const typename Phen::fit_t& fit_proto) {
        dbg::trace trace("eval", DBG_HERE);
        assert(pop.size());
        assert(begin < pop.size());
        assert(end <= pop.size());
        for (size_t i = begin; i < end; ++i) {

          pop[i]->fit() = fit_proto;
          for (int k=0; k < bottom_epochs; ++k)   // run the map for a while to develop new individuals
          {
            this->_pop[i]->epoch();
          }
          _nb_evals+= this->_pop[i]->_pop.size() * 2;//batch size times 2 
          pop[i]->develop();
          pop[i]->fit().eval(*pop[i]);// evaluate its recovered performance
          _nb_evals+=pop[i]->fit().nb_evals();// counts the number of individuals*environments in fit_top
        }
      }
      unsigned nb_evals() const { return _nb_evals; }
    protected:
      unsigned _nb_evals;
    };
  }
}