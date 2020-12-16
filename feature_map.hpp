#ifndef FEATURE_MAP_HPP
#define FEATURE_MAP_HPP

#include <meta-cmaes/feature_vector_typedefs.hpp>

#if FEATUREMAP == NONLINEAR
struct NonLinearFeatureMap
{
    weight_t W;
    NonLinearFeatureMap() {}
    NonLinearFeatureMap(const std::vector<float> &weights)
    {
        genotype_to_mat(weights);
    }
    static NonLinearFeatureMap random()
    {
        NonLinearFeatureMap fm = NonLinearFeatureMap();
        weight1_t W1 = weight1_t::Random();       //random numbers between (-1,1)
        //W1 = (W1 + weight1_t::Constant(1.)) / 2.; // add 1 to the matrix to have values between 0 and 2; divide by 2 --> [0,1]
        fm.W.W1 = W1;

        weight2_t W2 = weight2_t::Random();       //random numbers between (-1,1)
        //W2 = (W2 + weight2_t::Constant(1.)) / 2.; // add 1 to the matrix to have values between 0 and 2; divide by 2 --> [0,1]
        fm.W.W2 = W2;

        boost::random::mt19937 gen;
        //boost::random::uniform_real_distribution<> dist(0, 1);
	boost::random::uniform_real_distribution<> dist(-1, 1);
        fm.W.B1 = dist(gen);
        fm.W.B2 = dist(gen);
        return fm;
    }
    bottom_features_t out(const base_features_t &b)
    {

        hidden_t h = (W.W1 * b + hidden_t::Constant(W.B1)).unaryExpr(std::ptr_fun(NonLinearFeatureMap::sigmoidInner));
        bottom_features_t f = (W.W2 * h + bottom_features_t::Constant(W.B2)).unaryExpr(std::ptr_fun(NonLinearFeatureMap::sigmoidOuter));
        return f;
    }

    static float sigmoidInner(float x)
    {
	// from range [0,NUM_BASE_FEATURES + 1] to [-20,20]
        //std::cout << "raw " << x << std::endl;
        return normSigmoid(x,(float)(NUM_BASE_FEATURES+1.0));
    }

    static float sigmoidOuter(float x)
    {
	// from range [0,NUM_HIDDEN + 1] to [-20,20]
        //std::cout << "raw " << x << std::endl;
        return normSigmoid(x,(float)(NUM_HIDDEN+1.0));
    }
    static float normSigmoid(float x, float norm)
    {

	//x = -10.0f + 30.0f * x/norm;
	//if(x > 20.0f || x < -10.0f)
	//{
	//    std::cout << "WARNING: normalisation not in [-10,20]; if performing random_pop(), OK; otherwise you have a problem" << std::endl;
	//}
	x = 30.0f * x/norm;// [-30,30]
	if(x > 30.0f || x < -30.0f)
	{
	    std::cout << "WARNING: normalisation not in [-30,30]; if performing random_pop(), OK; otherwise you have a problem" << std::endl;
	}
        //std::cout << "normalised " << x << std::endl;
        x = 1.f / (1.f + std::exp(-x));
        //std::cout << "transformed " << x << std::endl;
	return x;
    }

    void genotype_to_mat(const std::vector<float> &weights)
    {
        size_t count = 0;
#ifdef PRINTING
        std::cout << "before conversion " << std::endl;
#endif
        for (size_t j = 0; j < NUM_HIDDEN; ++j)
        {
            for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
            {
                W.W1(j, k) = weights[count]; // put it available for the MapElites parent class

#ifdef PRINTING
                std::cout << weights[count] << std::endl;
                std::cout << W.W1(j, k) << "," << std::endl;
#endif
                ++count;
            }
        }
        W.B1 = weights[count];
        ++count;
        for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
        {
            for (size_t k = 0; k < NUM_HIDDEN; ++k)
            {
                W.W2(j, k) = weights[count]; // put it available for the MapElites parent class

#ifdef PRINTING
                std::cout << weights[count] << std::endl;
                std::cout << W.W2(j, k) << "," << std::endl;
#endif
                ++count;
            }
        }
        W.B2 = weights[count];
        ++count;

#ifdef PRINTING
        std::cout << "after conversion " << std::endl;
        std::cout << "W1=" << W.W1 << "\nB1" << W.B1 << "\nW2=" << W.W2 << "\nB2" << W.B2 << std::endl;
#endif
    }
    void print_weights(std::ostream &os)
    {

        os << W.W1 << "\n"
           << W.B1 << "\n"
           << W.W2 << "\n"
           << W.B2 << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
    void print_weights(std::ofstream &os)
    {

        os << W.W1 << "\n"
           << W.B1 << "\n"
           << W.W2 << "\n"
           << W.B2 << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
};
typedef NonLinearFeatureMap feature_map_t;
#elif FEATUREMAP == SELECTION
struct FeatureSelectionMap
{
    weight_t W;
    bottom_indices_t max_indices = bottom_indices_t::Constant(0);
    FeatureSelectionMap() {}
    FeatureSelectionMap(const std::vector<float> &weights)
    {
        genotype_to_mat(weights);
    }
    static FeatureSelectionMap random()
    {
        FeatureSelectionMap fm = FeatureSelectionMap();
        weight_t W = weight_t::Random();       //random numbers between (-1,1)
        W = (W + weight_t::Constant(1.)) / 2.; // add 1 to the matrix to have values between 0 and 2; divide by 2 --> [0,1]
        fm.W = W;
        for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
        {
            float max = -INFINITY;
            size_t maxInd = 0;
            for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
            {
                if (fm.W(j, k) > max)
                {
                    maxInd = k;
                    max = fm.W(j, k);
                }
#ifdef PRINTING
                std::cout << fm.W(j, k) << "," << std::endl;
#endif
            }
            fm.max_indices[j] = maxInd;
        }
        return fm;
    }
    bottom_features_t out(const base_features_t &b)
    {
        bottom_features_t f;

        for (size_t i = 0; i < max_indices.size(); ++i)
        {
            f[i] = b[max_indices[i]];
        }
        return f;
    }

    void genotype_to_mat(const std::vector<float> &weights)
    {
        size_t count = 0;
#ifdef PRINTING
        std::cout << "before conversion " << std::endl;
#endif
        for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
        {
            float max = -INFINITY;
            size_t maxInd = 0;
            for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
            {
                W(j, k) = weights[count]; // put it available for the MapElites parent class
                if (W(j, k) > max)
                {
                    maxInd = k;
                    max = W(j, k);
                }
#ifdef PRINTING
                std::cout << weights[count] << std::endl;
                std::cout << W(j, k) << "," << std::endl;
#endif
                ++count;
            }
            max_indices[j] = maxInd;
        }
#ifdef PRINTING
        std::cout << "after conversion " << std::endl;
        std::cout << W << std::endl;
#endif
    }

    void print_weights(std::ostream &os)
    {

        os << W << "\n"
           << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
    void print_weights(std::ofstream &os)
    {

        os << W << "\n"
           << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
};

typedef FeatureSelectionMap feature_map_t;
#else
struct LinearFeatureMap
{
    weight_t W;
    static constexpr float min = 0.20f, max =0.800f;
    LinearFeatureMap() {}
    LinearFeatureMap(const std::vector<float> &weights)
    {
        genotype_to_mat(weights);
    }
    static LinearFeatureMap random()
    {
        LinearFeatureMap fm = LinearFeatureMap();
        weight_t W = weight_t::Random();       //random numbers between (-1,1)
        W = (W + weight_t::Constant(1.)) / 2.; // add 1 to the matrix to have values between 0 and 2; divide by 2 --> [0,1]
        size_t count = 0;
#ifdef PRINTING
        std::cout << "before conversion " << std::endl;
#endif
        for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
        {
            float sum = W.block<1, NUM_BASE_FEATURES>(j, 0).sum();
            for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
            {
                W(j, k) = W(j, k) / sum; // put it available for the MapElites parent class

#ifdef PRINTING
                std::cout << "sum " << sum << std::endl;
                std::cout << W(j, k) << "," << std::endl;
#endif
                ++count;
            }
        }
        fm.W = W;
        return fm;
    }
    static float normalise(float x)
    {
	x =  (x - min)/(max - min);// from [0.125,0.875] to [0,1]
	return std::min(std::max(x, 0.0f), 1.0f);
    }
    
    bottom_features_t out(const base_features_t &b)
    {
        bottom_features_t f;
        return (W * b).unaryExpr(std::ptr_fun(LinearFeatureMap::normalise));
    }

    void genotype_to_mat(const std::vector<float> &weights)
    {
        size_t count = 0;
#ifdef PRINTING
        std::cout << "before conversion " << std::endl;
#endif
        for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
        {
            float sum = std::accumulate(weights.begin() + j * NUM_BASE_FEATURES, weights.begin() + (j + 1) * NUM_BASE_FEATURES, 0.0);
            for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
            {
                W(j, k) = weights[count] / sum; // put it available for the MapElites parent class

#ifdef PRINTING
                std::cout << "sum " << sum << std::endl;
                std::cout << weights[count] << std::endl;
                std::cout << W(j, k) << "," << std::endl;
#endif
                ++count;
            }
        }
#ifdef PRINTING
        std::cout << "after conversion " << std::endl;
        std::cout << W << std::endl;
#endif
    }

    void print_weights(std::ostream &os)
    {

        os << W << "\n"
           << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
    void print_weights(std::ofstream &os)
    {

        os << W << "\n"
           << std::endl;
        os << "END WEIGHTS" << std::endl;
    }
};
typedef LinearFeatureMap feature_map_t;

#endif

#endif
