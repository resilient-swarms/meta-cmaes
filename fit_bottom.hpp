

#ifndef FITBOTTOM_HPP
#define FITBOTTOM_HPP

#include <meta-cmaes/feature_vector_typedefs.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>

#if CONTROL()

#include <modules/map_elites/fit_map.hpp>
#endif

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/

namespace sferes
{
namespace fit
{
#if CONTROL()

SFERES_FITNESS(FitBottom, sferes::fit::FitMap)
{
#else
class FitBottom
{
#endif

public:
    FitBottom() : _dead(false){};
#if META()
    weight_t W;

    FitBottom(const weight_t &w) : W(w), _dead(false)
    {
    }
#endif
    inline void set_fitness(float fFitness)
    {
#if CONTROL()
        this->_objs.resize(1);
        this->_objs[0] = fFitness;
#endif
        this->_value = fFitness;
    }
    inline void set_dead(bool dead)
    {
        this->_dead = dead;
    }

    template <typename Indiv>
    void eval(Indiv & indiv)
    {

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        this->_value = 0;
        this->_dead = false;
        _eval<Indiv>(indiv);
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // std::cout << "Time difference = " <<     std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

    // override the function so that we can write behaviour descriptor values along with the fitness value
    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP(this->_value);
        ar &BOOST_SERIALIZATION_NVP(this->_desc);
    }

    bool dead() { return _dead; }
    std::vector<double> ctrl() { return _ctrl; }
    
    std::vector<float> get_desc(simulator_t & simu, base_features_t & b)
    {
#ifdef PRINTING

#endif

#if NO_WEIGHT()
        std::vector<double> vec;
#if DUTY_C()
        simu.get_descriptor<rhex_dart::descriptors::DutyCycle, std::vector<double>>(vec);
#elif BO_C()
        simu.get_descriptor<rhex_dart::descriptors::BodyOrientation, std::vector<double>>(vec);
#elif LV_C()
        Eigen::Vector3d velocities;
        simu.get_descriptor<rhex_dart::descriptors::AvgCOMVelocities, Eigen::Vector3d>(velocities);
        vec.resize(3);                                  // cf. skeleton : .54 .39 .139
        vec[0] = std::min(1.0, std::max(0.0, velocities[0] / (1.6 * global::BODY_LENGTH)));                                // [0, 1.6] body lengths (moving backwards is unlikely; .54 is body length)
        vec[1] = std::min(1.0, std::max(0.0, (velocities[1] + 0.80 * global::BODY_WIDTH) / (1.60 * global::BODY_WIDTH)));  // [-0.80,0.80] body widths, body cannot suddenly rotate heavily
        vec[2] = std::min(1.0, std::max(0.0, (velocities[2] + 0.30 * global::BODY_HEIGHT) / (0.60 * global::BODY_HEIGHT))); // [-0.30,0.30] body heights; body usually tilts backwards

#else
#error "Please give a viable condition in {0,1,2,3,4}"
#endif
        return std::vector<float>(vec.begin(), vec.end());
#else
        //get the base_features
        get_base_features(b, simu);
        // get descriptor
        return get_desc(b);
#endif
    }

#if WEIGHT()
    std::vector<float> get_desc(const base_features_t &b)
    {

#if META()
        bottom_features_t D = W * b;
#elif GLOBAL_WEIGHT()
        bottom_features_t D = global::W * b;
#endif
        std::vector<float> vec(D.data(), D.data() + D.rows() * D.cols());
#ifdef PRINTING
        std::cout << " getting descriptor " << std::endl;
#if META()
        std::cout << " w =  " << W << std::endl;
#elif GLOBAL_WEIGHT()
        std::cout << " w =  " << global::W << std::endl;
#endif
        std::cout << " b = " << b << std::endl;
        std::cout << " D = " << D << std::endl;
#endif
        return vec;
    }
#endif
#if META()
    mode::mode_t mode() const
    {
        return _mode;
    }
    void set_mode(mode::mode_t m)
    {
        _mode = m;
    }

    void set_desc(const std::vector<float> &d)
    {
        _desc = d;
    }

    void set_value(float v)
    {
        _value = v;
    }
    std::vector<float> desc()
    {
        return _desc;
    }

    float value()
    {
        return _value;
    }
#endif

protected:
    std::vector<double> _ctrl;
    bool _dead;

#if META() // these are already defined by FitMap
    mode::mode_t _mode;
    float _value = 0.0f;
    std::vector<float> _desc;
#endif

    // descriptor work done here, in this case duty cycle
    template <typename Indiv>
    void _eval(Indiv & indiv)
    {
        // copy of controller's parameters
        _ctrl.clear();

        for (size_t i = 0; i < 24; i++)
            _ctrl.push_back(indiv.data(i));

        // launching the simulation
        auto robot = global::global_robot->clone();

        simulator_t simu(_ctrl, robot);
        simu.run(BottomParams::simu::time); // run simulation for 5 seconds

        set_fitness(simu.covered_distance());

        std::vector<float> desc;

        float dead = -1000.0f;
        // these assume a behaviour descriptor of size 6.
        if (dead > this->_value)
        {
            // this means that something bad happened in the simulation
            // we kill this individual
            this->_dead = true; // no need to do anything
                                // desc.resize(6);
                                // desc[0] = 0;
                                // desc[1] = 0;
                                // desc[2] = 0;
                                // desc[3] = 0;
                                // desc[4] = 0;
                                // desc[5] = 0;
                                // this->_value = -1000.0f;// note this causes troubles;
                                // -> due to optimisation (presumably) the code is evaluated within if first, therefore the above condition seems to always be true
        }
        else
        {
            // convert to final descriptor
            base_features_t b;
            this->_desc = get_desc(simu, b);
#ifdef PRINTING
            std::cout << " fitness is " << this->_value << std::endl;
#endif
            this->_dead = false;
        }
    }

    /* the included descriptors determine the base-features */
    void get_base_features(base_features_t & base_features, simulator_t & simu)
    {

        std::vector<double> results;
        simu.get_descriptor<rhex_dart::descriptors::DutyCycle, std::vector<double>>(results);

        for (size_t i = 0; i < results.size(); ++i)
        {
            base_features(i, 0) = results[i];
        }

        simu.get_descriptor<rhex_dart::descriptors::BodyOrientation, std::vector<double>>(results);
        for (size_t i = 0; i < results.size(); ++i)
        {
            base_features(i + 6, 0) = results[i];
        }
        Eigen::Vector3d velocities;
        simu.get_descriptor<rhex_dart::descriptors::AvgCOMVelocities, Eigen::Vector3d>(velocities);                                      // cf. skeleton : .54 .39 .139
        base_features(12, 0) = std::min(1.0, std::max(0.0, velocities[0] / (2.0 * global::BODY_LENGTH)));                                // [0, 2] body lengths (moving backwards is unlikely; .54 is body length)
        base_features(13, 0) = std::min(1.0, std::max(0.0, (velocities[1] + 0.80 * global::BODY_WIDTH) / (1.60 * global::BODY_WIDTH)));  // [-0.80,0.80] body widths, body cannot suddenly rotate heavily
        base_features(14, 0) = std::min(1.0, std::max(0.0, (velocities[2] + 1.0 * global::BODY_HEIGHT) / (1.60 * global::BODY_HEIGHT))); // [-1,0.60] body heights; body usually tilts backwards
    }
};
} // namespace fit
} // namespace sferes

#endif