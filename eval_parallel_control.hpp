
#ifndef EVAL_PARALLEL_CONTROL_HPP
#define EVAL_PARALLEL_CONTROL_HPP

#include <sferes/eval/eval.hpp>
#include <meta-cmaes/eval_parallel.hpp>
namespace sferes
{
    namespace eval
    {
        SFERES_EVAL(EvalParallelControl, Eval)
        {
            public :
                template <typename Phen>
                void eval(std::vector<boost::shared_ptr<Phen>> & pop, size_t begin, size_t end,
                                const typename Phen::fit_t &fit_proto){
                            dbg::trace trace("eval", DBG_HERE);
                assert(pop.size());
                assert(begin < pop.size());
                assert(end <= pop.size());
        #ifdef ANALYSIS
                throw std::runtime_error("cannot use parallel while doing analysis");
        #endif
                /* if you want timer */
                //auto t1 = std::chrono::system_clock::now();
                /* initialise the fitmap */
                for (size_t i = begin; i < end; ++i)
                {
                    pop[i]->fit() = fit_proto;
                }
                auto helper = _eval_parallel_individuals<Phen, typename Phen::fit_t>();
                helper._pop = pop;
                helper.run();
                this->_nb_evals += (end - begin);
            } // namespace eval
        };    // namespace sferes
    }
}
#endif