

#define META_CMAES 0
#define RANDOM_WEIGHT 1
#define CONDITION_DUTY_CYCLE 2
#define CONDITION_BODY_ORIENTATION 3
#define CONDITION_LINEAR_VELOCITY 4

//#define EXPERIMENT_TYPE CONDITION_DUTY_CYCLE

#define META() EXPERIMENT_TYPE == META_CMAES
#define CONTROL() EXPERIMENT_TYPE > META_CMAES
#define GLOBAL_WEIGHT() EXPERIMENT_TYPE == RANDOM_WEIGHT
#define NO_WEIGHT() EXPERIMENT_TYPE > RANDOM_WEIGHT
#define WEIGHT() EXPERIMENT_TYPE == RANDOM_WEIGHT || EXPERIMENT_TYPE == META_CMAES
#define DUTY_C() EXPERIMENT_TYPE == CONDITION_DUTY_CYCLE
#define BO_C() EXPERIMENT_TYPE == CONDITION_BODY_ORIENTATION
#define LV_C() EXPERIMENT_TYPE == CONDITION_LINEAR_VELOCITY

#define ENVIR_TESTS()  EVAL_ENVIR==1 && (TEST || META())
#define DAMAGE_TESTS()  EVAL_ENVIR==0 && (TEST || META())

#ifdef GRAPHIC
#define NO_PARALLEL
#endif

#define FRICTION 1.0

#define TAKE_COMPLEMENT // whether or not to test for generalisation to unseen world/damage conditions

//#define PRINTING
//#define CHECK_PARALLEL
#ifndef TEST
#define PARALLEL_RUN
#endif

#include <boost/random.hpp>
#include <iostream>
#include <mutex>
#include <cstdlib>

#include <sferes/gen/evo_float.hpp>
//#include <sferes/gen/sampled.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>

#include <sferes/stat/pareto_front.hpp>

#include <rhex_dart/rhex_dart_simu.hpp>
#include <chrono>

#include <sferes/stc.hpp>
#include <meta-cmaes/global.hpp>

#if META()
#include <meta-cmaes/meta-CMAES.hpp>
#include <meta-cmaes/stat_maps.hpp>
#include <meta-cmaes/stat_pop.hpp>
#else
#include <meta-cmaes/control_typedefs.hpp>
#include <meta-cmaes/stat_map.hpp>
#include <sferes/ea/ea.hpp>
#include <modules/map_elites/map_elites.hpp>
typedef boost::fusion::vector<sferes::stat::Map<phen_t, BottomParams>> stat_t;

typedef modif::Dummy<> modifier_t;
typedef sferes::ea::MapElites<phen_t, eval_t, stat_t, modifier_t, BottomParams> ea_t;
#endif



#include <sferes/run.hpp>

//#define GRAPHIC

using namespace sferes;






int main(int argc, char **argv)
{
    std::srand(atoi(argv[1])); //use experiment number as seed for random generator. mostly for Eigen
    ea_t ea;
#ifdef PARALLEL_RUN
    sferes::eval::init_shared_mem();
#endif
    // initialisation of the simulation and the simulated robot, robot morphology currently set to raised.skel only
    global::init_simu(std::string(argv[1]), std::string(std::getenv("RESIBOTS_DIR")) + "/share/rhex_models/SKEL/raised.skel");

    run_ea(argc, argv, ea);

    global::global_robot.reset();
    return 0;
}
