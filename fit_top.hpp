
#ifndef FIT_TOP_HPP_
#define FIT_TOP_HPP_

#include <modules/map_elites/fit_map.hpp>


const size_t num_world_options = 10;

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/

FIT_MAP(FitTop)
{
public:
    /* current bottom-level map (new candidate to be added to _pop)*/
    
    indiv_t current_map;
    template <typename MetaIndiv>
    void eval(MetaIndiv & indiv)
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
    std::vector<double> weight() { return _ctrl; }

protected:
    bool _dead;
    std::vector<double> _ctrl;
    float num_bd_calcs;
    size_t nb_evals = 0;
    
    // sampling without replacement (see https://stackoverflow.com/questions/28287138/c-randomly-sample-k-numbers-from-range-0n-1-n-k-without-replacement)
    std::unordered_set<size_t> pickSet(size_t N, size_t k, std::mt19937& gen)
    {
        std::uniform_int_distribution<> dis(1, N);
        std::unordered_set<size_t elems;

        while (elems.size() < k) {
            elems.insert(dis(gen));
        }

        return elems;
    }
    std::vector<size_t> pick(size_t N, size_t k) {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::unordered_set<size_t> elems = pickSet(N, k, gen);
        return  std::vector<int> result(elems.begin(), elems.end());
    }

    // descriptor work done here, in this case duty cycle
    template <typename MetaIndiv>
    void _eval(MetaIndiv & meta_indiv)
    {
        float avg_fitness = 0;
        num_bd_calcs = 0.0;
        std::vector<float> avg_bd = {0.0, 0.0, 0.0};
        size_t num_individuals = std::max(1, (size_t)0.10 * current_map->_pop.size());
        std::vector<size_t> individuals = pick(num_individuals, current_map->_pop.size());
        for(size_t individual: individuals)
        {
            Indiv indiv = current_map->_pop[index];
            for (size_t world_option = 0; world_option < num_world_options; ++world_option)
            {
                eval_single_envir<Indiv>(indiv, world_option, avg_fitness, avg_bd);
            }
        }

        avg_bd[0] /= num_bd_calcs;
        avg_bd[1] /= num_bd_calcs;
        avg_bd[2] /= num_bd_calcs;
        this->set_desc(avg_bd);
        this->_value = avg_fitness / num_calcs; // no need to divide
        this->_dead = false;
        nb_evals = individuals.size() * num_world_options;
    }

    void _eval_single_envir(Indiv & indiv, size_t world_option, float &avg_fitness, std::vector<float> &avg_bd)
    {
        // copy of controller's parameters
        _ctrl.clear();

        for (size_t i = 0; i < 24; i++)
            _ctrl.push_back(indiv.data(i));

        // launching the simulation
        auto robot = global::global_robot->clone();
        using safe_t = boost::fusion::vector<rhex_dart::safety_measures::BodyColliding, rhex_dart::safety_measures::MaxHeight, rhex_dart::safety_measures::TurnOver>;
        using desc_t = boost::fusion::vector<rhex_dart::descriptors::BodyOrientation>;

        rhex_dart::RhexDARTSimu<rhex_dart::safety<safe_t>, rhex_dart::desc<desc_t>> simu(_ctrl, robot, world_option);
        simu.run(5); // run simulation for 5 seconds

        float fitness = simu.covered_distance();

        // these assume a behaviour descriptor of size 6.
        if (fitness < -1000)
        {
            // this means that something bad happened in the simulation
            // we do not kill the individual in the meta-map, but set fitness to zero and bd does not contribute
            avg_fitness += 0;
            // do not update the descriptor !
        }
        else
        {
            desc.resize(3);
            std::vector<double> v;
            simu.get_descriptor<rhex_dart::descriptors::BodyOrientation>(v);
            desc[0] = safety();
            desc[1] = power_consumption();
            desc[2] = 0.0;
            // update the meta-fitness
            avg_fitness += fitness;
            // update the meta-descriptor
            bd[0] += desc[0];
            bd[1] += desc[1];
            bd[2] += desc[2];
            ++num_bd_calcs;
        }
    }

    size_t nb_evals()
    {
        return nb_evals;
    }
};

#endif