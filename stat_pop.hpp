#ifndef STAT_DATABASE_HPP_
#define STAT_DATABASE_HPP_

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

public:
    std::vector<boost::shared_ptr<Phen>> _pop;
    size_t _capacity, _sp, _max_sp;
    global::database_t _database;
    template <typename E>
    void refresh(const E &ea)
    {

        
        if (ea.gen() % CMAESParams::pop::dump_period == 0)
        {
#ifdef PRINTING
            std::cout << "starting dump of Stat_Database" << std::endl;
            _write_database(std::string("database_"), ea);
#endif
            _pop = ea.pop();
            _write_recovered_performances(std::string("recovered_perf"), ea);
            get_database();
        }
        
    }

    void get_database()
    {
        _capacity = global::database.get_capacity();
        _database = global::database;
        _sp       = global::database.sp;
        _max_sp   = global::database.max_sp;
    }
    void set_database()
    {
        // reset the data-base
        global::database = _database;
        global::database.sp = _sp;
        global::database.max_sp = _max_sp;
    }
    // show the n-th individual from zero'th map in the population
    void show(std::ostream & os, size_t k)
    {
        std::cerr << "loading map 0, individual " + std::to_string(k);
        //set_database();// don't need it since now MetaCmaes invokes this at _load
        // develop map 0
        this->_pop[0]->develop();
        // evaluate individual k within this map
        _show_individual(os, k);
    }

    template <class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        // reset the data-base
        ar &BOOST_SERIALIZATION_NVP(_pop);
        ar &BOOST_SERIALIZATION_NVP(_capacity);

        for (size_t i = 0; i < _capacity; ++i)
        {
            ar &BOOST_SERIALIZATION_NVP(_database[i]);
        }
        ar &BOOST_SERIALIZATION_NVP(_sp);
        ar &BOOST_SERIALIZATION_NVP(_max_sp);
        if (Archive::is_loading::value)
        {
            set_database();
        }
    }

protected:
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

    template <typename EA>
    void _write_recovered_performances(const std::string &prefix,
                                       const EA &ea) const
    {
        std::cout << "writing..." << prefix << ea.gen() << std::endl;
        std::string fname = ea.res_dir() + "/" + prefix + boost::lexical_cast<std::string>(ea.gen()) + std::string(".dat");

        std::ofstream ofs(fname.c_str());
        for (size_t k = 0; k < _pop.size(); ++k)
        {
            ofs << _pop[k]->fit().value() << "\t";
        }
        ofs << "\n";
    }
    void _show_individual(std::ostream & os, size_t k)
    {
        bottom_indiv_t ind = this->_pop[0]->archive().data()[k];
        ind->fit() = bottom_fit_t(this->_pop[0]->W);
        ind->develop();
        ind->show(os);
        ind->fit().set_mode(fit::mode::view);
        ind->fit().eval(*ind);
    }
};
} // namespace stat
} // namespace sferes

// SFERES_STAT(Stat_Pop, Stat)
// {
//     typedef boost::shared_ptr<phen_t> indiv_t;
//     typedef typename std::vector<indiv_t> pop_t;
//     pop_t _pop;
//     size_t nb_evals;

// public:
//     Stat_Pop() {}

//     template <typename E>
//     void refresh(const E &ea)
//     {
//     }

//     template <class Archive>
//     void serialize(Archive & ar, const unsigned int version)
//     {

//         ar &BOOST_SERIALIZATION_NVP(_pop);
//     }
// };
//} // namespace stat
//} // namespace sferes

#endif