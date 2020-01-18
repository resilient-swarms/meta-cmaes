#ifndef CONTROL_TYPEDEFS_HPP
#define CONTROL_TYPEDEFS_HPP

#include <sferes/fit/fitness.hpp>
#include <meta-cmaes/params.hpp>
#include <boost/shared_ptr.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/eval/eval.hpp>
#include <meta-cmaes/fit_bottom.hpp>
#include <modules/map_elites/stat_map.hpp>
#include <modules/map_elites/map_elites.hpp>




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// TOP   ////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// now that FitBottom is defined, define the rest of the bottom level
typedef sferes::fit::FitBottom<BottomParams> fit_t;
typedef sferes::phen::Parameters<bottom_gen_t, fit_t, BottomParams> phen_t;
typedef boost::shared_ptr<phen_t> bottom_indiv_t;
typedef sferes::eval::Eval<BottomParams> eval_t;


typedef boost::fusion::vector<sferes::stat::Map<phen_t, BottomParams>> stat_t;

typedef modif::Dummy<> modifier_t;
typedef sferes::ea::MapElites<phen_t, eval_t, stat_t, modifier_t, BottomParams> ea_t;









#endif