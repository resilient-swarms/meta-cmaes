
#ifndef PARAMETER_CONTROL_HPP
#define PARAMETER_CONTROL_HPP
#include <string>

template <typename B_Pars, typename C_Pars> //bottom params
struct ParameterControl
{
    float bottom_epochs_factor = 1.0f;
    float percentage_evaluated_factor = 1.0f;
    ParameterControl(float bf, float pf)
    {
        bottom_epochs_factor = bf;
        percentage_evaluated_factor = pf;
    }
    int get_bottom_epochs()
    {
        int bot_epochs = (int)std::round(bottom_epochs_factor * B_Pars::bottom_epochs);
        std::cout << "bottom epochs " << bot_epochs << std::endl;
        return bot_epochs;
    }
    float get_percentage_evaluated()
    {
        float percentage = percentage_evaluated_factor * C_Pars::percentage_evaluated;
        std::cout << "percentage = " << percentage << std::endl;
        return percentage;
    }
};

template <typename B_Params, typename C_Params>
ParameterControl<B_Params, C_Params> *init_parameter_control(std::string choice)
{
    if (choice == "b1p1")
    {
        return new ParameterControl<B_Params, C_Params>(1.f, 1.f);
    }
    else if (choice == "b1p2")
    {
        return new ParameterControl<B_Params, C_Params>(1.f, 2.f);
    }
    else if (choice == "b1p5")
    {
        return new ParameterControl<B_Params, C_Params>(1.f, 5.f);
    }
    else if (choice == "b1p10")
    {
        return new ParameterControl<B_Params, C_Params>(1.f, 10.f); //100%
    }
    else if (choice == "b2p1")
    {
        return new ParameterControl<B_Params, C_Params>(2.f, 1.f);
    }
    else if (choice == "b2p2")
    {
        return new ParameterControl<B_Params, C_Params>(2.f, 2.f);
    }
    else if (choice == "b2p5")
    {
        return new ParameterControl<B_Params, C_Params>(2.f, 5.f);
    }
    else if (choice == "b2p10")
    {
        return new ParameterControl<B_Params, C_Params>(2.f, 10.f); //100%
    }
    else if (choice == "b5p1")
    {
        return new ParameterControl<B_Params, C_Params>(5.f, 1.f);
    }
    else if (choice == "b5p2")
    {
        return new ParameterControl<B_Params, C_Params>(5.f, 2.f);
    }
    else if (choice == "b5p5")
    {
        return new ParameterControl<B_Params, C_Params>(5.f, 5.f);
    }
    else if (choice == "b5p10")
    {
        return new ParameterControl<B_Params, C_Params>(5.f, 10.f); //100%
    }
    else if (choice == "b10p1")
    {
        return new ParameterControl<B_Params, C_Params>(10.f, 1.f);
    }
    else if (choice == "b10p2")
    {
        return new ParameterControl<B_Params, C_Params>(10.f, 2.f);
    }
    else if (choice == "b10p5")
    {
        return new ParameterControl<B_Params, C_Params>(10.f, 5.f);
    }
    else if (choice == "b10p10")
    {
        return new ParameterControl<B_Params, C_Params>(10.f, 10.f); //100%
    }
    else
    {
        throw std::runtime_error("not expected argument for parameter control " + choice);
    }
}
#endif