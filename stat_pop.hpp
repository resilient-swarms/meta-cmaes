#ifndef STAT_DATABASE_HPP_
#define STAT_DARABASE_HPP_

#include <numeric>
#include <boost/multi_array.hpp>
#include <boost/serialization/array.hpp>
#include <sferes/stat/stat.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
#include <meta-cmaes/top_typedefs.hpp>
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
SFERES_STAT(Stat_Pop, Stat)
{
    typedef boost::shared_ptr<phen_t> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    pop_t _pop;
    size_t nb_evals;
    void _show_individual(std::ostream & os, size_t k)
    {
        bottom_indiv_t ind = this->_pop[0]->archive().data()[k];
        ind->fit() = bottom_fit_t(this->_pop[0]->W);
        ind->develop();
        ind->show(os);
        ind->fit().set_mode(fit::mode::view);
        ind->fit().eval(*ind);
    }
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

public:
    Stat_Pop() {}

    template <typename E>
    void refresh(const E &ea)
    {
        if (ea.gen() % CMAESParams::pop::dump_period == 0)
        {
#ifdef PRINTING
            std::cout << "starting dump of Stat_Database" << std::endl;
#endif
            _write_database(std::string("database_"), ea);
            _pop = ea.pop();
        }
    }

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP(global::database);
        ar &BOOST_SERIALIZATION_NVP(_pop);
    }
    void show(std::ostream & os, size_t k)
    {
        std::cerr << "loading map 0, individual " + std::to_string(k);
        // develop map 0
        this->_pop[0]->develop();
        // evaluate individual k within this map
        _show_individual(os, k);
    }
};
} // namespace stat
} // namespace sferes

#endif