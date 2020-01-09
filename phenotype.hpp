

#include <Eigen>
#include <rhex_dart/rhex_dart_simu.hpp>
#include <modules/map_elites/map_elites.hpp>

#define NUM_BASE_FEATURES 12  // number of base features
#define NUM_TOP_CELLS 15      // number of cells in the meta-map
#define NUM_BOTTOM_FEATURES 3 // number of features for bottom level maps

/* base-features */
typedef Matrix<float, NUM_BASE_FEATURES, 1> base_features_t;

/* weights to construct bottom-level map features from base_features */
typedef Matrix<float, NUM_BOTTOM_FEATURES, NUM_BASE_FEATURES> weight_t;

/* bottom-level map features */
typedef Matrix<float, NUM_BOTTOM_FEATURES, 1> bottom_features_t;

// bottom-level typedefs
typedef eval::Eval<BottomParams> bottom_eval_t;

typedef gen::Sampled<24, BottomParams> bottom_gen_t; // 24 parameters for our controller
typedef FitBottom<BottomParams> bottom_fit_t;
typedef phen::Parameters<bottom_gen_t, bottom_fit_t, BottomParams> bottom_phen_t;

#ifdef BINARY
typedef sferes::stat::MapBinary<bottom_phen_t, Params> bottommap_stat_t;
#else
typedef sferes::stat::Map<bottom_phen_t, Params> bottommap_stat_t;
#endif

typedef boost::fusion::vector<bottommap_stat_t, sferes::stat::MapProgress<bottom_phen_t, BottomParams>> bottom_stat_t;
typedef modif::Dummy<> bottom_modifier_t;
typedef ea::MapElites<bottom_phen_t, bottom_eval_t, bottom_stat_t, bottom_modifier_t, BottomParams> bottom_ea_t;

typedef boost::shared_ptr<bottom_phen_t> bottom_indiv_t;

/* struct to store all of a bottom-level individual's information 
        -base-features
        -fitness
        -phenotype
*/
struct DataEntry
{
    base_features_t base_features;
    bottom_indiv_t individual; // bottom-level genotype, bottom-level fitness
};

/* phenotype has members _gen and _fit, the genotype and fitmap, 
                        and is responsible for individuals random init, mutation, development */

class MapElitesPhenotype : public EvoFloat, public MapElites
{

    MapElitesPhenotype(){}; // will create random genotype and no fitness
    MapElitesPhenotype(const DataEntry& entry, const std::vector)
    { // create specific genotype, with given descriptor and fitness

        std::vector<float> desc = obtain_descriptor(entry.gen().data()  ,entry.base_features);
        this->fit().set_desc(desc);
        this->fit().set_obj(0, fitness);
    }

    /* the included descriptors determine the base-features */
    void get_base_features(base_features_t &base_features)
    {

        std::vector<double> results;
        rhex_dart::get_descriptor<DutyCycle, std::vector<double>>(results);

        for (size_t i = 0; i < results.size(); ++i)
        {
            base_features(i, 0) = results[i];
        }

        std::vector<double> results2;
        rhex_dart::get_descriptor<DutyCycle, std::vector<double>>(results2);

        for (size_t i = 0; i < results2.size(); ++i)
        {
            base_features(6 + i, 0) = results2[i];
        }
        return base_features;
    }

    /* after the base-features are obtained, a bottom-level map processes the features into its own */
    std::vector<float> descriptor obtain_descriptor(const weight_t &W, const base_features_t &b)
    {

        bottom_features_t D = W * b;
        return {D(0, 0), D(1, 0), D(2, 0)};
    }

    /*  get the weights (genotype)    */
    void get_weights(weight_t &W)
    {
        std::vector<float> weights = gen()->data(); // get the genotype
        float *v = &param[0];
        W(va_list);
    #ifdef PRINTING
        for(float w : weights)
        {
            std::cout<<w<<,;
        }
        std::cout<<\n W << std::endl;
    #endif
    }

    std::vector<float> get_desc()
    {
        //get the base_features
        base_features_t b;
        get_base_features(b);

        //get the weights
        weight_t W;
        get_weights(W);

        //obtain descriptor
        return obtain_descriptor(W, b);
    }
};
