#ifndef STAT_DATABASE_HPP_
#define STAT_DARABASE_HPP_

#include <numeric>
#include <boost/multi_array.hpp>
#include <boost/serialization/array.hpp>
#include <sferes/stat/stat.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
#include <Eigen/Dense>
// #define MAP_WRITE_PARENTS

namespace boost
{
template <class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
inline void serialize(
    Archive &ar,
    Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> &t,
    const unsigned int file_version)
{
    size_t rows = t.rows(), cols = t.cols();
    ar &rows;
    ar &cols;
    if (rows * cols != t.size())
        t.resize(rows, cols);

    for (size_t i = 0; i < t.size(); i++)
        ar &BOOST_SERIALIZATION_NVP(t.data()[i]);
}
} // namespace boost

namespace sferes
{
namespace stat
{
SFERES_STAT(Stat_Database, Stat){
    public :
        Stat_Database(){}

    template <typename E>
    void refresh(const E &ea){
        if (ea.gen() % CMAESParams::pop::dump_period == 0){
#ifdef PRINTING
            std::cout << "starting dump of Stat_Database" << std::endl;
#endif
_write_database(std::string("database_"), ea);
} // namespace stat
} // namespace sferes
template <typename EA>
void _write_database(const std::string &prefix,
                     const EA &ea) const
{
    std::cout << "writing..." << prefix << ea.gen() << std::endl;
    std::string fname = ea.res_dir() + "/" + prefix + boost::lexical_cast<std::string>(ea.gen()) + std::string(".dat");

    std::ofstream ofs(fname.c_str());
    for (size_t k = 0; k < global::database.size(); ++k)
    {
        ofs << global::database[k].base_features << "\t" << global::database[k].fitness << std::endl;
    }
}

template <class Archive>
void serialize(Archive &ar, const unsigned int version)
{
    ar &BOOST_SERIALIZATION_NVP(global::database);
}
}
;
} // namespace stat
} // namespace sferes

#endif