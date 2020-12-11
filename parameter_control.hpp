
#ifndef PARAMETER_CONTROL_HPP
#define PARAMETER_CONTROL_HPP

#include <string>
#include <meta-cmaes/global.hpp>
#include <meta-cmaes/params.hpp>

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct ParameterControl
{
    float bottom_epochs_factor = 1.0f;
    float percentage_evaluated_factor = 1.0f;
    float mutation_rate_factor = 1.0f;
    MetaPhen phenotype;
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
        std::cout << "mutation rate = " << mutation_rate << std::endl;
        return mutation_rate;
    }
};

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct EpochAnnealing : public ParameterControl<MetaPhen, B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochAnnealing(float bf, float pf) : ParameterControl<MetaPhen, B_Pars, C_Pars>(bf, pf)
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

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct EpochEndogenous : public ParameterControl<MetaPhen, B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochEndogenous(float bf, float pf) : ParameterControl<MetaPhen, B_Pars, C_Pars>(bf, pf)
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

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct MutationAnnealing : public ParameterControl<MetaPhen, B_Pars, C_Pars>
{
    float min_mutation_rate = 0.001f;
    MutationAnnealing(float bf, float pf, float mf) : ParameterControl<MetaPhen, B_Pars, C_Pars>(bf, pf, mf)
    {
    }

    virtual float get_mutation_rate()
    {
        float ratio = (float)(C_Pars::pop::max_evals - global::nb_evals) / (float)C_Pars::pop::max_evals;
        float mutation_rate = this->min_mutation_rate + ratio * (this->mutation_rate_factor * B_Pars::sampled::mutation_rate - this->min_mutation_rate); //
        std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return mutation_rate;
    }
};

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct MutationEndogenous : public ParameterControl<MetaPhen, B_Pars, C_Pars>
{
    float min_mutation_rate = 0.001f;
    MutationEndogenous(float bf, float pf, float mf) : ParameterControl<MetaPhen, B_Pars, C_Pars>(bf, pf, mf)
    {
    }

    virtual float get_mutation_rate()
    {
        size_t last = NUM_GENES - 1;
        float last_gene = this->phenotype.gen().data()[last];                                                                                                // in [0,1]
        float mutation_rate = this->min_mutation_rate + last_gene * (this->mutation_rate_factor * B_Pars::sampled::mutation_rate - this->min_mutation_rate); //
        std::cout << "mutation rate " << mutation_rate << " at evals " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return mutation_rate;
    }
};

template <typename MetaPhen, typename B_Params, typename C_Params>
ParameterControl<MetaPhen, B_Params, C_Params> *init_parameter_control(std::string choice)
{
    if (choice == "b1p1")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(1.f, 1.f);
    }
    else if (choice == "b1p2")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(1.f, 2.f);
    }
    else if (choice == "b1p5")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(1.f, 5.f);
    }
    else if (choice == "b1p10")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(1.f, 10.f); //100%
    }
    else if (choice == "b2p1")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "b2p2")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(2.f, 2.f);
    }
    else if (choice == "b2p5")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(2.f, 5.f);
    }
    else if (choice == "b2p10")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(2.f, 10.f); //100%
    }
    else if (choice == "b5p1")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(5.f, 1.f);
    }
    else if (choice == "b5p2")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(5.f, 2.f);
    }
    else if (choice == "b5p5")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(5.f, 5.f);
    }
    else if (choice == "b5p10")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(5.f, 10.f); //100%
    }
    else if (choice == "b10p1")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "b10p2")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(10.f, 2.f);
    }
    else if (choice == "b10p5")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(10.f, 5.f);
    }
    else if (choice == "b10p10")
    {
        return new ParameterControl<MetaPhen, B_Params, C_Params>(10.f, 10.f); //100%
    }
    else if (choice == "epochannealing_b2p1")
    {
        return new EpochAnnealing<MetaPhen, B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "epochannealing_b10p1")
    {
        return new EpochAnnealing<MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "epochendogeneous_b10p1")
    {
        return new EpochEndogenous<MetaPhen, B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "mutationannealing_b1p1m2")
    {
        return new MutationAnnealing<MetaPhen, B_Params, C_Params>(1.f, 1.f,2.f);
    }
    else if (choice == "mutationannealing_b1p1m8")
    {
        return new MutationAnnealing<MetaPhen, B_Params, C_Params>(1.f, 1.f,8.f);
    }
    else if (choice == "mutationendogeneous_b1p1m8")
    {
        return new MutationEndogenous<MetaPhen, B_Params, C_Params>(1.f, 1.f,8.f);
    }
    else
    {
        throw std::runtime_error("not expected argument for parameter control " + choice);
    }
}

#endif
