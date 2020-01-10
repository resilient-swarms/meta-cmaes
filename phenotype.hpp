


#include <Eigen/Dense>
#include <rhex_dart/rhex_dart_simu.hpp>
#include <modules/map_elites/map_elites.hpp>
#include <meta-cmaes/fit_bottom.hpp>
#include <meta-cmaes/top_typedefs.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>

#include <sferes/gen/cmaes.hpp>







/* phenotype has members _gen and _fit, the genotype and fitmap, 
                        and is responsible for individuals random init, mutation, development */

class MapElitesPhenotype : public gen_t, public MapElites
{

    MapElitesPhenotype(){}; // will create random genotype and no fitness

    /* the included descriptors determine the base-features */
    void get_base_features(base_features_t &base_features, rhex_dart::RhexDARTSimu<rhex_dart::safety<safe_t>, rhex_dart::desc<desc_t>>& simu)
    {

        std::vector<double> results;
        simu.get_descriptor<rhex_dart::descriptors::, std::vector<double>>(results);

        for (size_t i = 0; i < results.size(); ++i)
        {
            base_features(i, 0) = results[i];
        }

        std::vector<double> results2;
        simu.get_descriptor<rhex_dart::descriptors::BodyOrientation, std::vector<double>>(results2);

        for (size_t i = 0; i < results2.size(); ++i)
        {
            base_features(6 + i, 0) = results2[i];
        }
    }

    /* after the base-features are obtained, a bottom-level map processes the features into its own */
    std::vector<float> obtain_descriptor(const weight_t &W, const base_features_t &b)
    {

        bottom_features_t D = W * b;
        std::vector<float> vec(D.data(), D.data() + D.rows() * D.cols());


        return vec;
    }

    /*  get the weights (genotype)    */
    void get_weights(weight_t &W)
    {
        std::vector<float> weights = gen()->data(); // get the genotype
        float *v = &weights[0];
        W(v, weights.size());
    #ifdef PRINTING
        for(float w : weights)
        {
            std::cout<<w<<,;
        }
        std::cout<<\n W << std::endl;
    #endif
    }

    std::vector<float> get_desc(rhex_dart::RhexDARTSimu<rhex_dart::safety<safe_t>, rhex_dart::desc<desc_t>>& simu)
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
