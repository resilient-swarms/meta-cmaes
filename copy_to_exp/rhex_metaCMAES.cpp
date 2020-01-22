

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

#ifdef GRAPHIC
#define NO_PARALLEL
#endif

#define FRICTION 1.0

//#define PRINTING
//#define CHECK_PARALLEL
#define PARALLEL_RUN

#include <boost/random.hpp>
#include <iostream>
#include <mutex>
#include <cstdlib>

#include <sferes/gen/evo_float.hpp>
//#include <sferes/gen/sampled.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/run.hpp>
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
#endif

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
