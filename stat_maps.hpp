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
SFERES_STAT(Stat_Maps, Stat)
{
public:
    typedef typename std::vector<bottom_indiv_t> bottom_pop_t;
    typedef typename bottom_pop_t::iterator bottom_it_t;
    typedef typename std::vector<std::vector<bottom_indiv_t>> bottom_front_t;
    typedef boost::shared_ptr<base_phen_t> bottom_phen_ptr_t;
    static const size_t behav_dim = BottomParams::ea::behav_dim;

    typedef std::array<float, behav_dim> point_t;
    typedef boost::multi_array<bottom_phen_ptr_t, behav_dim> array_t;
    typedef std::array<typename array_t::index, behav_dim> behav_index_t;

    behav_index_t behav_shape;
    behav_index_t behav_strides;
    behav_index_t behav_indexbase;

    Stat_Maps()
    {
        for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
            behav_shape[i] = BottomParams::ea::behav_shape(i);
    }

    template <typename E>
    void refresh(const E &ea)
    {
        if (ea.gen() % CMAESParams::pop::dump_period == 0)
        {

            for (size_t map = 0; map < ea.pop().size(); ++map)
            {
                auto archive = ea.pop()[map]->archive();
                // write all current archives to a file

                for (size_t k = 0; k < behav_dim; ++k)
                {
                    assert(archive.shape()[k] == behav_shape[k]);
                    behav_strides[k] = archive.strides()[k];
                    behav_indexbase[k] = archive.index_bases()[k];
                }

                _write_archive(archive, map,  std::string("archive" + std::to_string(map) + "_"), ea);
            }
        }
    }

    template <typename EA>
    void _write_archive(const array_t &array,
                    size_t map,
                        const std::string &prefix,
                        const EA &ea) const
    {
        std::cout << "writing..." << prefix << ea.gen() << std::endl;
        std::string fname = ea.res_dir() + "/" + prefix + boost::lexical_cast<std::string>(ea.gen()) + std::string(".dat");

        std::ofstream ofs(fname.c_str());

        size_t offset = 0;
        for (const bottom_indiv_t *i = array.data(); i < (array.data() + array.num_elements()); ++i)
        {
            if (*i)
            {
                behav_index_t posinarray = ea.pop()[map]->getindexarray(array, i);
                assert(array(posinarray)->fit().value() == (*i)->fit().value());

                ofs << offset << "    ";
                for (size_t dim = 0; dim < behav_dim; ++dim)
                    ofs << posinarray[dim] / (float)behav_shape[dim] << " ";
                ofs << " " << array(posinarray)->fit().value() << " ";
                for (size_t k = 0; k < array(posinarray)->gen().size(); ++k)
                    ofs << array(posinarray)->gen().data(k) << " ";
                ofs << std::endl;
            }
            ++offset;
        }
    }
}; // namespace stat
} // namespace stat
} // namespace sferes

#endif