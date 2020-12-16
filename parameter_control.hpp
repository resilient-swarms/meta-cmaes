
#ifndef PARAMETER_CONTROL_HPP
#define PARAMETER_CONTROL_HPP

#include <string>
#include <meta-cmaes/global.hpp>
#include <meta-cmaes/params.hpp>
#include <meta-cmaes/parameter_controller.hpp>

template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct ParameterControl
{
    float bottom_epochs_factor = 1.0f;
    float percentage_evaluated_factor = 1.0f;
    float mutation_rate_factor = 1.0f;
    MetaPhen phenotype;
    EvalStats eval_stats;
    size_t evaluations = 0;
    ParameterControl(float bf, float pf, float mf = 1.0f)
    {
        this->bottom_epochs_factor = bf;
        this->percentage_evaluated_factor = pf;
        this->mutation_rate_factor = mf;
    }
    /* update the parameter control algorithm */
    virtual void update()
    {
    }
    /* get the number of bottom epochs per meta-individual */
    virtual int get_bottom_epochs()
    {
        int bot_epochs = (int)std::round(this->bottom_epochs_factor * B_Pars::bottom_epochs);
        std::cout << "bottom epochs " << bot_epochs << std::endl;
        return bot_epochs;
    }
    /* get the percentage of solutions to be evaluated for meta-fitness computation */
    virtual float get_percentage_evaluated()
    {
        float percentage = this->percentage_evaluated_factor * C_Pars::percentage_evaluated;
        std::cout << "percentage = " << percentage << std::endl;
        return percentage;
    }

    /* get the mutation rate */
    virtual float get_mutation_rate()
    {
        float mutation_rate = this->mutation_rate_factor * B_Pars::sampled::mutation_rate;
        if(global::nb_evals > this->evaluations)
        {
            std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        }
        this->evaluations = global::nb_evals;
        return mutation_rate;
    }

    virtual void set_stats(EvalStats &eval_stats)
    {
        this->eval_stats = eval_stats;
    }
};

template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct EpochAnnealing : public ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochAnnealing(float bf, float pf) : ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>(bf, pf)
    {
    }

    virtual int get_bottom_epochs()
    {
        float ratio = (float)(C_Pars::pop::max_evals - global::nb_evals) / (float)C_Pars::pop::max_evals;
        int bot_epochs = (int)std::round(this->min_bottom_epochs + ratio * (this->bottom_epochs_factor * B_Pars::bottom_epochs - this->min_bottom_epochs)); //
        std::cout << "bottom epochs " << bot_epochs << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return bot_epochs;
    }
};

template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct EpochEndogenous : public ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochEndogenous(float bf, float pf) : ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>(bf, pf)
    {
    }

    virtual int get_bottom_epochs()
    {
        size_t last = NUM_GENES - 1;
        float last_gene = this->phenotype.gen().data()[last];                                                                                                   // in [0,1]
        int bot_epochs = (int)std::round(this->min_bottom_epochs + last_gene * (this->bottom_epochs_factor * B_Pars::bottom_epochs - this->min_bottom_epochs)); //
        std::cout << "bottom epochs " << bot_epochs << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return bot_epochs;
    }
};

template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct MutationAnnealing : public ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>
{
    float min_mutation_rate = 0.001f;
    MutationAnnealing(float bf, float pf, float mf) : ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>(bf, pf, mf)
    {
    }

    virtual float get_mutation_rate()
    {
        float ratio = (float)(C_Pars::pop::max_evals - global::nb_evals) / (float)C_Pars::pop::max_evals;
        float mutation_rate = this->min_mutation_rate + ratio * (this->mutation_rate_factor * B_Pars::sampled::mutation_rate - this->min_mutation_rate); //
        if(global::nb_evals > this->evaluations)
        {
            std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        }
        this->evaluations = global::nb_evals;
        return mutation_rate;
    }
};

template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct MutationEndogenous : public ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>
{
    float min_mutation_rate = 0.001f;
    MutationEndogenous(float bf, float pf, float mf) : ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>(bf, pf, mf)
    {
    }

    virtual float get_mutation_rate()
    {
        size_t last = NUM_GENES - 1;
        float last_gene = this->phenotype.gen().data()[last];                                                                                                // in [0,1]
        float mutation_rate = this->min_mutation_rate + last_gene * (this->mutation_rate_factor * B_Pars::sampled::mutation_rate - this->min_mutation_rate); //
        if(global::nb_evals > this->evaluations)
        {
            std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        }
        this->evaluations = global::nb_evals;
        return mutation_rate;
    }
};
template <typename EvalStats, typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct RL : public ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>
{
    float f_last;
    size_t stagnation = 0;
    size_t num_params;
    RLController controller;
    float mutation_rate, bottom_epochs;
    float scale; //scale accounting for average number of bottom-level evaluations
    const float reward_max = 10.0f;// beyond 10-fold improvement is not meaningful
    RL(long seed, std::string &parameter, float bf, float pf, float mf) : ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>(bf, pf, mf)
    {
        scale = C_Pars::pop::size * (B_Pars::bottom_epochs * 2 * B_Pars::pop::size + C_Pars::percentage_evaluated*4096.0f ) ;
        controller = RLController();
        if (parameter == "mutation_rate")
        {
            controller.addParameter("mutation_rate", ParameterController::ParameterType::NUMERIC_DOUBLE_, 0.f, 1.f);
            num_params = 1;
        }
        else if (parameter == "bottom_epochs")
        {
            controller.addParameter("bottom_epochs", ParameterController::ParameterType::NUMERIC_INT_, 1.f, this->bottom_epochs_factor * B_Pars::bottom_epochs);
            num_params = 1;
        }
        else if (parameter == "both")
        {
            controller.addParameter("mutation_rate", ParameterController::ParameterType::NUMERIC_DOUBLE_, 0.f, 1.f);
            controller.addParameter("bottom_epochs", ParameterController::ParameterType::NUMERIC_INT_, 1.f, this->bottom_epochs_factor * B_Pars::bottom_epochs);
            num_params = 2;
        }
        else
        {
            throw std::runtime_error("not implemented");
        }
        controller.initialize(seed, "bins:5");
    }
    virtual float get_mutation_rate()
    {
        //std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return mutation_rate;
    }

    virtual int get_bottom_epochs()
    {
        std::cout << "bottom epochs " << bottom_epochs << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return bottom_epochs;
    }

    virtual void set_stats(EvalStats &eval_stats)
    {
        this->eval_stats = eval_stats;
        float cf = this->eval_stats.best_metafitness;
        float ratio = 0;
        if (cf == 0)
        {
            ratio = f_last;
        }
        else if (cf > 0 && f_last >= 0)
        {
            ratio = cf / f_last;
        }
        else if (cf < 0 && f_last >= 0)
        {
            ratio = cf - f_last;
        }
        else if (cf < 0 && f_last < 0)
        {
            ratio = f_last / cf;
        }
        float rwrd = std::min(reward_max, scale * (ratio - 1) / (global::nb_evals - this->evaluations));
        //	System.out.println(rwrd);
        if (rwrd < 0)
            rwrd = 0;
        std::vector<float> obs(7);
        obs[0] = rwrd;
        obs[1] = this->eval_stats.best_metafitness;
        obs[2] = this->eval_stats.avg_metafitness;
        obs[3] = this->eval_stats.sd_metafitness;
        obs[4] = this->eval_stats.metagenotype_diversity;
        obs[5] = stagnation;
        obs[6] = obs[0];
        std::cout << "obs " << std::endl; 
        for (size_t i = 0; i < obs.size(); ++i)
        {
            std::cout << obs[i] << ", ";
        }
        std::cout << std::endl;
        controller.updateObservables(obs);
        // Update parameters
        float normal_be = ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>::get_bottom_epochs();
        bottom_epochs = controller.getNextValue("bottom_epochs", normal_be);

        float normal_mr = ParameterControl<EvalStats, MetaPhen, B_Pars, C_Pars>::get_mutation_rate();
        mutation_rate = controller.getNextValue("mutation_rate", normal_mr);
	    std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        //set_action(obs);//done directly in get_mutation_rate and get_bottom_epochs
        // Update stagnation count
        if (cf > f_last)
        {
            // An improvement was made, reset the stagnation count
            stagnation = 0;
        }
        else
        {
            stagnation++;
        }
        // Update the last current best fitness
        f_last = this->eval_stats.best_metafitness;
        // Update evaluations
        this->evaluations = global::nb_evals;
    }
};
template <typename EvalStats, typename MetaPhen, typename B_Params, typename C_Params>
ParameterControl<EvalStats, MetaPhen, B_Params, C_Params> *init_parameter_control(long seed, std::string choice)
{
    if (choice == "b1p1")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 1.f);
    }
    else if (choice == "b1p2")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 2.f);
    }
    else if (choice == "b1p5")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 5.f);
    }
    else if (choice == "b1p10")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 10.f); //100%
    }
    else if (choice == "b2p1")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "b2p2")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(2.f, 2.f);
    }
    else if (choice == "b2p5")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(2.f, 5.f);
    }
    else if (choice == "b2p10")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(2.f, 10.f); //100%
    }
    else if (choice == "b5p1")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(5.f, 1.f);
    }
    else if (choice == "b5p2")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(5.f, 2.f);
    }
    else if (choice == "b5p5")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(5.f, 5.f);
    }
    else if (choice == "b5p10")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(5.f, 10.f); //100%
    }
    else if (choice == "b10p1")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "b10p2")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 2.f);
    }
    else if (choice == "b10p5")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 5.f);
    }
    else if (choice == "b10p10")
    {
        return new ParameterControl<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 10.f); //100%
    }
    else if (choice == "epochannealing_b2p1")
    {
        return new EpochAnnealing<EvalStats, MetaPhen, B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "epochannealing_b10p1")
    {
        return new EpochAnnealing<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "epochendogeneous_b10p1")
    {
        return new EpochEndogenous<EvalStats, MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "mutationannealing_b1p1m2")
    {
        return new MutationAnnealing<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 1.f, 2.f);
    }
    else if (choice == "mutationannealing_b1p1m8")
    {
        return new MutationAnnealing<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 1.f, 8.f);
    }
    else if (choice == "mutationendogeneous_b1p1m8")
    {
        return new MutationEndogenous<EvalStats, MetaPhen, B_Params, C_Params>(1.f, 1.f, 8.f);
    }
    else if (choice == "mutationrl_b1p1m8")
    {
        std::string parameter = "mutation_rate";
        return new RL<EvalStats, MetaPhen, B_Params, C_Params>(seed, parameter, 1.f, 1.f, 8.f);
    }
    else
    {
        throw std::runtime_error("not expected argument for parameter control " + choice);
    }
}

#endif
