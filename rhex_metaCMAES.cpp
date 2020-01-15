
#include <boost/random.hpp>
#include <iostream>
#include <mutex>

#include <sferes/gen/evo_float.hpp>
//#include <sferes/gen/sampled.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/run.hpp>
#include <sferes/stat/pareto_front.hpp>

#include <rhex_dart/rhex_dart_simu.hpp>
#include <chrono>

#include <sferes/stc.hpp>

#include <meta-cmaes/meta-CMAES.hpp>

//#define GRAPHIC

#ifdef GRAPHIC
#define NO_PARALLEL
#endif

#define FRICTION 1.0

using namespace sferes;


int main(int argc, char **argv)
{
    typedef ea::MetaCmaes<phen_t, eval_t, stat_t, modifier_t, CMAESParams> ea_t;
    ea_t ea;
    // initialisation of the simulation and the simulated robot, robot morphology currently set to raised.skel only
    global::init_simu(std::string(std::getenv("RESIBOTS_DIR")) + "/share/rhex_models/SKEL/raised.skel");

    run_ea(argc, argv, ea);

    global::global_robot.reset();
    return 0;
}
