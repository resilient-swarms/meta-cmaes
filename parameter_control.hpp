
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
    MetaPhen phenotype;
    ParameterControl(float bf, float pf)
    {
        this->bottom_epochs_factor = bf;
        this->percentage_evaluated_factor = pf;
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
};

template <typename MetaPhen,typename B_Pars, typename C_Pars> //bottom params
struct EpochAnnealing : public ParameterControl<MetaPhen, B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochAnnealing(float bf, float pf) : ParameterControl<MetaPhen,B_Pars, C_Pars>(bf, pf)
    {
    }
    
    virtual int get_bottom_epochs()
    {
        float ratio = (float)(C_Pars::pop::max_evals - global::nb_evals) / (float)C_Pars::pop::max_evals;
        int bot_epochs = (int)std::round(this->min_bottom_epochs + ratio * (this->bottom_epochs_factor * B_Pars::bottom_epochs - this->min_bottom_epochs)); //
        std::cout << "bottom epochs " << bot_epochs << " at generation " << global::nb_evals << " / " << C_Pars::pop::max_evals << std::endl;
        return bot_epochs;
    }
};

template <typename MetaPhen, typename B_Pars, typename C_Pars> //bottom params
struct EpochEndogenous : public ParameterControl<MetaPhen,B_Pars, C_Pars>
{
    float min_bottom_epochs = 1.0f;
    EpochEndogenous(float bf, float pf) : ParameterControl<MetaPhen,B_Pars, C_Pars>(bf, pf)
    {
    }

    virtual int get_bottom_epochs()
    {
        size_t last = NUM_GENES - 1;
        float last_gene = this->phenotype.gen().data()[last]; // in [0,1]
        int bot_epochs = (int)std::round(this->min_bottom_epochs + last_gene * (this->bottom_epochs_factor * B_Pars::bottom_epochs - this->min_bottom_epochs)); //
        return bot_epochs;
    }
};

template <typename MetaPhen, typename B_Params, typename C_Params>
ParameterControl<MetaPhen,B_Params, C_Params> *init_parameter_control(std::string choice)
{
    if (choice == "b1p1")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(1.f, 1.f);
    }
    else if (choice == "b1p2")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(1.f, 2.f);
    }
    else if (choice == "b1p5")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(1.f, 5.f);
    }
    else if (choice == "b1p10")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(1.f, 10.f); //100%
    }
    else if (choice == "b2p1")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "b2p2")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(2.f, 2.f);
    }
    else if (choice == "b2p5")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(2.f, 5.f);
    }
    else if (choice == "b2p10")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(2.f, 10.f); //100%
    }
    else if (choice == "b5p1")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(5.f, 1.f);
    }
    else if (choice == "b5p2")
    {
        return new ParameterControl< MetaPhen,B_Params, C_Params>(5.f, 2.f);
    }
    else if (choice == "b5p5")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(5.f, 5.f);
    }
    else if (choice == "b5p10")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(5.f, 10.f); //100%
    }
    else if (choice == "b10p1")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "b10p2")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(10.f, 2.f);
    }
    else if (choice == "b10p5")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(10.f, 5.f);
    }
    else if (choice == "b10p10")
    {
        return new ParameterControl<MetaPhen,B_Params, C_Params>(10.f, 10.f); //100%
    }
    else if (choice == "epochannealing_b2p1")
    {
        return new EpochAnnealing<MetaPhen,B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "epochannealing_b10p1")
    {
        return new EpochAnnealing<MetaPhen,B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "epochendogeneous_b10p1")
    {
        return new EpochEndogenous<MetaPhen,B_Params, C_Params>(10.f, 1.f);
    }
    else
    {
        throw std::runtime_error("not expected argument for parameter control " + choice);
    }
}



#endif
