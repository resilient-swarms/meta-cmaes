#ifndef STAT_DATABASE_HPP_
#define STAT_DATABASE_HPP_

#include <numeric>
#include <boost/multi_array.hpp>
#include <boost/serialization/array.hpp>
#include <sferes/stat/stat.hpp>
#include <meta-cmaes/bottom_typedefs.hpp>
#include <meta-cmaes/mapelites_phenotype.hpp>
#include <meta-cmaes/top_typedefs.hpp>
#include <meta-cmaes/stat_map.hpp>
#include <meta-cmaes/eval_total.hpp>
#include <Eigen/Dense>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

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
            std::string _resume_file;
            size_t _capacity, _sp, _max_sp;
            global::database_t _database;
            size_t _nb_evals;
            boost::shared_ptr<ParameterControl> _param_ctrl;
            template <typename E>
            void refresh(const E &ea)
            {

                if (ea.gen() % CMAESParams::pop::dump_period == 0 || ea.is_stopped())
                {
#ifdef PRINTING
                    std::cout << "starting dump of Stat_Database" << std::endl;
                    _write_database(std::string("database_"), ea);
#endif
                    _pop = ea.pop();
                    _write_recovered_performances(std::string("recovered_perf"), ea);
                    get_database();
                    std::cout << "number of evaluations so far " << _nb_evals << std::endl;
                    _resume_file = (ea.res_dir() + "/resume_file_gen_" + std::to_string(ea.gen()) + ".dat");
                    this->_param_ctrl = sferes::eval::param_ctrl;
                }
            }

            void write_resume()
            {
                std::cout << "will write resume file " << _resume_file << std::endl;
                int n = _resume_file.length();
                char *_resume_f = new char[n + 1];
                strcpy(_resume_f, _resume_file.c_str());
                cmaes_WriteToFile(&global::evo, "resume", _resume_f);
            }
            void get_database()
            {
                _database = global::database;
                _nb_evals = global::nb_evals;
            }
            void set_globals()
            {
                // reset the data-base
                global::database = _database;
                global::nb_evals = _nb_evals;
                int n = _resume_file.length();
                char *_resume_f = new char[n + 1];
                strcpy(_resume_f, _resume_file.c_str());
                resume_distr(_resume_f);
                sferes::eval::param_ctrl = this->_param_ctrl;
            }
            // show the n-th individual from zero'th map in the population
            void show(std::ostream & os, size_t n)
            {

                //first get the mean
                boost::shared_ptr<Phen> mean = boost::shared_ptr<Phen>(new Phen());
                size_t num_weights = this->_pop[0]->gen().data().size();
                for (size_t i = 0; i < num_weights; ++i)
                {
                    mean->gen().data(i, (float)global::evo.rgxmean[i]);
                }
                std::map<typename Phen::behav_index_t, Eigen::VectorXd> feature_list;
                mean->develop(feature_list);
#ifdef GRAPHIC // just display solution n of the mean_archive

                float val = 0.0f;
                size_t count = 0;
                std::cout << "loading individual" << n << std::endl;
                for (const bottom_indiv_t *k = mean->archive().data(); k < (mean->archive().data() + mean->archive().num_elements()); ++k)
                {
                    if (count == n)
                    {
                        float val = sferes::fit::RecoveredPerformance<base_phen_t>::_eval_all(**k);
                        std::cout << val << std::endl;
                        return;
                    }
                    ++count;

                    //std::cout << count;
                }

#else // write down all test performances
                for (auto it = feature_list.begin(); it != feature_list.end(); ++it)
                {
                    os << it->second.transpose() << std::endl;
                }
                os << "END FEATURES POPULATION MEAN " << std::endl;
                mean->feature_map.print_weights(os);
                os << "END WEIGHTS POPULATION MEAN " << std::endl;
                //

                // then get the current meta-population
                auto t1 = Clock::now();
                for (size_t i = 0; i < this->_pop.size(); ++i)
                {
                    this->_pop[i]->develop(); // don't take into account any additions to the database; so you know which one is the best according to evolution
                    this->_pop[i]->feature_map.print_weights(os);
                }
                auto t2 = Clock::now();
                std::cout << "database load time for 5 maps: "
                          << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
                          << " milliseconds" << std::endl;
                //also write the mean archive to file
                std::string s = _resume_file;
                std::string delimiter = "resume";
                std::string folder = s.substr(0, s.find(delimiter)); // get the folder in which the resume file is
                std::string name = folder + std::string("/mean_archive.dat");
                std::ofstream meanarchive_file(name);
                size_t count = 0;
                for (const bottom_indiv_t *k = mean->archive().data(); k < (mean->archive().data() + mean->archive().num_elements()); ++k)
                {
                    if (*k)
                    {
                        meanarchive_file << count;
                        std::vector<float> bd = (*k)->fit().desc();
                        for (size_t i = 0; i < bd.size(); ++i)
                        {
                            meanarchive_file << " " << bd[i];
                        }
                        meanarchive_file << " " << (*k)->fit().value() << std::endl;
                    }
                    ++count;
                }
                // first do population mean
                _show_(os, mean->archive());
                os << "END STATS POPULATION MEAN " << std::endl;

                // don't take into account any additions to the database; so you know which one is the best according to evolution
                for (size_t i = 0; i < this->_pop.size(); ++i)
                {

                    _show_(os, this->_pop[i]->archive());
                }
#endif
            }

            void _show_(std::ostream & os, const boost::multi_array<bottom_indiv_t, BottomParams::ea::behav_dim> &archive)
            {
                std::cout << "show stat" << std::endl;
                std::cout << "read the archive" << std::endl;
#ifdef INDIVIDUAL_DAMAGE
                sferes::fit::RecoveredPerformance<base_phen_t>::test_max_recovery(os, archive);
#else
                sferes::fit::RecoveredPerformance<base_phen_t>::test_recoveredperformance(os, archive);
#endif
            }

            template <class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                // reset the data-base
                ar &BOOST_SERIALIZATION_NVP(_pop);

                ar &BOOST_SERIALIZATION_NVP(_database);

                ar &BOOST_SERIALIZATION_NVP(_nb_evals);

                ar &BOOST_SERIALIZATION_NVP(_resume_file);
                ar &BOOST_SERIALIZATION_NVP(_param_ctrl);
                if (Archive::is_loading::value)
                {
#ifndef TEST
                    if (_nb_evals > CMAESParams::pop::max_evals)
                    {
                        std::cout << "this evolutionary run has already finished" << std::endl;
                        exit(0);
                    }
#endif
                    set_globals();
                }
                else
                {
                    write_resume();
                }
            }

        protected:
            template <typename EA>
            void _write_database(const std::string &prefix,
                                 const EA &ea) const
            {
                std::cout << "writing..." << ea.gen() << prefix << std::endl;
                std::string fname = ea.res_dir() + "/" + boost::lexical_cast<std::string>(ea.gen()) + prefix + std::string(".dat");

                std::ofstream ofs(fname.c_str());
                global::database.write(ofs);
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
