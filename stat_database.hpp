#ifndef STAT_MAP_HPP_
#define STAT_MAP_HPP_

#include <numeric>
#include <boost/multi_array.hpp>
#include <boost/serialization/array.hpp>
#include <sferes/stat/stat.hpp>
#include <bottom_typedefs.hpp>
// #define MAP_WRITE_PARENTS

namespace sferes
{
namespace stat
{
SFERES_STAT(Stat_Database, Stat)
{
public:
    Stat_Database()
    {
    }

    template <typename E>
    void refresh(const E &ea)
    {
        if (ea.gen() % CMAESParams::pop::dump_period == 0)
        {
            _write_database(ea._pop[i]->archive(), std::string("database_"), ea);
        }
    }
    template <typename EA>
    void _write_database(const array_t &array,
                         const std::string &prefix,
                         const EA &ea) const
    {
        std::cout << "writing..." << prefix << ea.gen() << std::endl;
        std::string fname = ea.res_dir() + "/" + prefix + boost::lexical_cast<std::string>(ea.gen()) + std::string(".dat");

        std::ofstream ofs(fname.c_str());

        size_t offset = 0;
        for (size_t k = 0; k < global::database.size(); ++k)
        {
           
            ofs << global::database[k].base_features << "\t" << global::database[k].fitness << std::endl;
        }
    }

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP();
    }

}; // namespace stat
} // namespace stat
} // namespace sferes

#endif