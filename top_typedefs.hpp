#ifndef TOP_TYPEDEFS_HPP
#define TOP_TYPEDEFS_HPP


#include <sferes/gen/evo_float.hpp>
//#include <sferes/gen/evo_float.hpp>


#include <Eigen/Dense>
#include <rhex_dart/rhex_dart_simu.hpp>

//#include <meta-cmaes/fit_bottom.hpp>


#include <sferes/eval/eval.hpp>
#include <sferes/modif/dummy.hpp>


#include <sferes/stat/best_fit.hpp>

#include <meta-cmaes/bottom_typedefs.hpp>
#include <meta-cmaes/params.hpp>
#include <meta-cmaes/fit_top.hpp>
#include <meta-cmaes/eval_total.hpp>
#include <meta-cmaes/stat_maps.hpp>
#include <meta-cmaes/stat_pop.hpp>
#include <meta-cmaes/stat_database.hpp>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// TOP   ////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef sferes::gen::EvoFloat<NUM_GENES, CMAESParams> gen_t;



typedef sferes::eval::EvalTotal<CMAESParams> eval_t;
typedef sferes::fit::FitTop<CMAESParams>  fit_t;

 
typedef sferes::phen::Parameters<gen_t, fit_t, CMAESParams> meta_phen_t;
typedef MapElitesPhenotype<meta_phen_t> phen_t; 

typedef boost::fusion::vector<sferes::stat::Stat_Database<phen_t, CMAESParams>,sferes::stat::Stat_Pop<phen_t, CMAESParams>,sferes::stat::Stat_Maps<phen_t, CMAESParams>> stat_t;

typedef modif::Dummy<> modifier_t;










#endif