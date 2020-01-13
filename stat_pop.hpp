#ifndef STAT_MAP_HPP_
#define STAT_MAP_HPP_

#include <numeric>
#include <boost/multi_array.hpp>
#include <boost/serialization/array.hpp>
#include <sferes/stat/stat.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
// #define MAP_WRITE_PARENTS

namespace sferes
{
namespace stat
{
SFERES_STAT(Stat_Pop, Stat){
    public :

        Stat_Pop(){}

    template <typename E>
    void refresh(const E &ea){
        if (ea.gen() % CMAESParams::pop::dump_period == 0){
#ifdef PRINTING
            std::cout << "starting dump of Stat_Pop" << std::endl;
#endif
_write_pop(std::string("pop_"), ea);
} // namespace stat
} // namespace sferes

template <typename EA>
void _write_pop(const std::string &prefix,
                const EA &ea) const
{
    std::cout << "writing..." << prefix << ea.gen() << std::endl;
    std::string fname = ea.res_dir() + "/" + prefix + boost::lexical_cast<std::string>(ea.gen()) + std::string(".dat");

    std::ofstream ofs(fname.c_str());

    size_t offset = 0;
    ofs << ea.gen() << " " << ea.nb_evals() << "\n  \\\\\\\\ \n";
    for (size_t i = 0; i < ea.pop().size(); ++i)
    {
        std::vector<float> data = ea.pop()[i]->gen().data();
        ofs << "(";
        for (size_t i = 0; i < data.size(); ++i)
        {
            ofs << std::setprecision(5) << data[i] << ",";
        }
        ofs << ") \t " << ea.pop()[i]->fit().value() << std::endl;
    }
}
}
;
} // namespace stat
} // namespace sferes

#endif