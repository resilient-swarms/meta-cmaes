
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


#define GRAPHIC

#ifdef GRAPHIC
	#define NO_PARALLEL
#endif



#define FRICTION 1.0


using namespace sferes;



void init_simu(std::string robot_file, std::vector<rhex_dart::RhexDamage> damages = std::vector<rhex_dart::RhexDamage>())
{
    global::global_robot = std::make_shared<rhex_dart::Rhex>(robot_file, "Rhex", false, damages);
}


int main(int argc, char** argv)
{
    typedef ea::MetaCmaes<phen_t, eval_t, stat_t, modifier_t, CMAESParams> ea_t;
    ea_t ea;
    // initilisation of the simulation and the simulated robot, robot morphology currently set to raised.skel only
    init_simu(std::string(std::getenv("RESIBOTS_DIR")) + "/share/rhex_models/SKEL/raised.skel", global::damages);

    run_ea(argc, argv, ea);

    global::global_robot.reset();
    return 0;
}
