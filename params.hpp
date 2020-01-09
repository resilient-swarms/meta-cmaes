#include <boost/random.hpp>
#include <iostream>
#include <mutex>

#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/run.hpp>
#include <sferes/stat/pareto_front.hpp>



#include <meta-map-elites/phenotype.hpp>
#include <chrono>

#ifdef GRAPHIC
// #define NO_PARALLEL
#endif

#define NO_MPI

#ifndef NO_PARALLEL
#include <sferes/eval/parallel.hpp>
#ifndef NO_MPI
#include <sferes/eval/mpi.hpp>
#endif
#else
#include <sferes/eval/eval.hpp>
#endif



/* params for the bottom-level map */
struct BottomParams {

    // grid properties, discretise 3 dimensions into 10 bins each
    struct ea {
        SFERES_CONST size_t behav_dim = 3;
        SFERES_ARRAY(size_t, behav_shape, 10, 10, 10); // 125 cells for each bottom-level map
        SFERES_CONST float epsilon = 0.00;
    };

    // our values for each gait parameter can take on any one of these    ????????????????????
    struct sampled {
        SFERES_ARRAY(float, values, 0.00, 0.025, 0.05, 0.075, 0.10, 0.125, 0.15, 0.175,
                     0.20, 0.225, 0.25, 0.275, 0.30, 0.325, 0.35,
                     0.375, 0.40, 0.425, 0.45, 0.475, 0.50, 0.525,
                     0.55, 0.575, 0.60, 0.625, 0.65, 0.675, 0.70,
                     0.725, 0.75, 0.775, 0.80, 0.825, 0.85, 0.875,
                     0.90, 0.925, 0.95, 0.975, 1);

        SFERES_CONST float mutation_rate = 0.05f;
        SFERES_CONST float cross_rate = 0.00f;
        SFERES_CONST bool ordered = false;
    };

    struct pop {
        SFERES_CONST unsigned size = 200;
        SFERES_CONST unsigned init_size = 200;
        SFERES_CONST unsigned nb_gen = 100001;// not used
        SFERES_CONST int dump_period = 50;
        SFERES_CONST int initial_aleat = 1;
    };

    // parameter limits between 0 and 1
    struct parameters {
        SFERES_CONST float min = 0.0f;
        SFERES_CONST float max = 1.0f;
    };
};



/* params for the top-level map */
struct Params {

    // grid properties, discretise 3 dimensions into 10 bins each
    struct ea {
        SFERES_CONST size_t behav_dim = 2;
        SFERES_ARRAY(size_t, behav_shape, 4, 4); // 16 cells based on two meta-descriptors
        SFERES_CONST float epsilon = 0.00;
    };

    struct evo_float {
        // we choose the polynomial mutation type
        SFERES_CONST mutation_t mutation_type = polynomial;
        // we choose the polynomial cross-over type
        SFERES_CONST cross_over_t cross_over_type = sbx;
        // the mutation rate of the real-valued vector
        SFERES_CONST float mutation_rate = 0.1f;
        // the cross rate of the real-valued vector
        SFERES_CONST float cross_rate = 0.5f;
        // a parameter of the polynomial mutation
        SFERES_CONST float eta_m = 15.0f;
        // a parameter of the polynomial cross-over
        SFERES_CONST float eta_c = 10.0f;
    };

    // save map every 50 iterations
    struct pop {
        SFERES_CONST unsigned nb_gen = 100001;// used 
        SFERES_CONST int dump_period = 50;// used
        SFERES_CONST int size        = 0;//not used
        SFERES_CONST int initial_aleat = 1;
    };

    // parameter limits between 0 and 1
    struct parameters {
        SFERES_CONST float min = 0.0f;
        SFERES_CONST float max = 1.0f;
    };
};
