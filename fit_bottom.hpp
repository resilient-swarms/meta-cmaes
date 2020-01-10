
#ifndef FIT_BOTTOM_HPP_
#define FIT_BOTTOM_HPP_


#include <modules/map_elites/fit_map.hpp>
#include <rhex_dart/safety_measures.hpp>
#include <rhex_dart/descriptors.hpp>

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/

FIT_MAP(FitBottom)
{
public:
    size_t nb_evals = 0;
    template <typename Indiv>
    void eval(Indiv & indiv)
    {

        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        this->_objs.resize(1);
        std::fill(this->_objs.begin(), this->_objs.end(), 0);
        this->_dead = false;
        _eval(indiv);
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // std::cout << "Time difference = " <<     std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    }

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        dbg::trace trace("fit", DBG_HERE);

        ar &boost::serialization::make_nvp("_value", this->_value);
        ar &boost::serialization::make_nvp("_objs", this->_objs);
    }

    bool dead() { return _dead; }
    std::vector<double> ctrl() { return _ctrl; }

protected:
    bool _dead;
    std::vector<double> _ctrl;

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
        using safe_t = boost::fusion::vector<rhex_dart::safety_measures::BodyColliding, rhex_dart::safety_measures::MaxHeight, rhex_dart::safety_measures::TurnOver>;
        using desc_t = boost::fusion::vector<rhex_dart::descriptors::BodyOrientation>;

        rhex_dart::RhexDARTSimu<rhex_dart::safety<safe_t>, rhex_dart::desc<desc_t>> simu(_ctrl, robot);
        simu.run(5); // run simulation for 5 seconds

        this->_value = simu.covered_distance();

        std::vector<float> desc;

        // these assume a behaviour descriptor of size 6.
        if (this->_value < -1000)
        {
            // this means that something bad happened in the simulation
            // we kill this individual
            this->_dead = true;
            desc.resize(3);
            desc[0] = 0;
            desc[1] = 0;
            desc[2] = 0;
            this->_value = -1000;
        }
        else
        {
            desc.resize(3);
            std::vector<double> v;
            desc = indiv.get_desc(simu);

                        //simu.get_descriptor<rhex_dart::descriptors::BodyOrientation>(v);
            // desc[0] = v[0];
            // desc[1] = v[1];
            // desc[2] = v[2];

        }

        this->set_desc(desc);
    }
};

#endif