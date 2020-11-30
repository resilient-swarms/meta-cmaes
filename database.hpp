
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
struct SampledDataEntry
{
    std::vector<size_t> genotype;
    base_features_t base_features;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    float fitness;
    SampledDataEntry() {}
    SampledDataEntry(const std::vector<size_t> &g, const base_features_t &b, const float &f) : genotype(g), base_features(b), fitness(f)
    {
    }
    template <typename Individual>
    void set_genotype(Individual &individual) const
    {
        for (size_t j = 0; j < individual->size(); ++j)
        {
            individual->gen().set_data(j, genotype[j]); // use the Sampled genotype API
        }
    }
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP(base_features);
        ar &BOOST_SERIALIZATION_NVP(fitness);
        ar &BOOST_SERIALIZATION_NVP(genotype);
    }
};

struct EvoFloatDataEntry
{
    std::vector<float> genotype;
    base_features_t base_features;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    float fitness;
    EvoFloatDataEntry() {}
    EvoFloatDataEntry(const std::vector<float> &g, const base_features_t &b, const float &f) : genotype(g), base_features(b), fitness(f)
    {
    }
    // in case we want to use Evofloat instead
    template <typename Individual>
    void set_genotype(Individual &individual) const
    {
        for (size_t j = 0; j < individual->size(); ++j)
        {
            individual->gen().data(j, genotype[j]); // use the EvoFloat genotype API
        }
    }
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &BOOST_SERIALIZATION_NVP(base_features);
        ar &BOOST_SERIALIZATION_NVP(fitness);
        ar &BOOST_SERIALIZATION_NVP(genotype);
    }
};

template <size_t capacity, typename DataType>
struct CircularBuffer
{
    size_t get_capacity()
    {
        return capacity;
    }
    CircularBuffer() : sp(0), max_sp(0)
    {
        data.resize(capacity);
    }
    std::vector<DataType> data;
    size_t sp;
    size_t max_sp;
    DataType &operator[](size_t idx)
    {
        return data[idx];
    }
    typename std::vector<DataType>::iterator end()
    {
        return data.begin() + max_sp;
    }
    typename std::vector<DataType>::iterator begin()
    {
        return data.begin();
    }
    size_t size()
    {
        return max_sp;
    }
    void push_back(const DataType &d)
    {
        if (sp >= capacity)
        {
            // reset sp
            sp = 0;
        }
        data[sp] = d;
        if (max_sp < capacity)
            ++max_sp;
        ++sp;
    }
    void write(const std::ofstream &ofs)
    {
        for (size_t k = 0; k < data.size(); ++k)
        {
            ofs << data[k].base_features.transpose() << "\t" << data[k].fitness << std::endl;
        }
    }
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar &BOOST_SERIALIZATION_NVP(sp);
        ar &BOOST_SERIALIZATION_NVP(max_sp);
        for (size_t i = 0; i < capacity; ++i)
        {
            ar &BOOST_SERIALIZATION_NVP(data[i]);
        }
    }
};

template <size_t num_base_features, size_t capacity, typename DataType>
struct BestKPerBin // filtering based on BD, with approximately the same capacity, but solutions are only replaced by better ones in same behavioural bin
{
    size_t k = 5000;           // maximal entries per bin (will decrease to 1 minimally over the run)
    size_t sp;                 //keep count
    float bins_per_dim = 3.0f; // very coarse map (with 14D around 5M unique bins)

    bool equal(const base_features_t &lhs, const base_features_t &rhs)
    {
        size_t i = 0;
        float bins_within = bins_per_dim * k;
        assert(lhs.size() == rhs.size() && lhs.size() == num_base_features);
        while (i < num_base_features)
        {
            if (std::round(lhs[i] * bins_within) / bins_within != std::round(rhs[i] * bins_within) / bins_within)
            {
                return false;
            }
            i++;
        }
        return true;
    }
    struct classcomp
    {
        /* to sort the std::map */
        bool operator()(const std::vector<float> &lhs, const std::vector<float> &rhs) const
        {
            float bins_per_dim = 3.0f; // very coarse map (with 14D still +/- 5M unique bins)
            size_t dim = num_base_features;
            assert(lhs.size() == rhs.size() && lhs.size() == dim);
            size_t i = 0;
            while (i < dim - 1 && std::round(lhs[i] * bins_per_dim) / bins_per_dim == std::round(rhs[i] * bins_per_dim) / bins_per_dim) //lhs[i]==rhs[i])
                i++;
            return std::round(lhs[i] * bins_per_dim) / bins_per_dim < std::round(rhs[i] * bins_per_dim) / bins_per_dim; //lhs[i]<rhs[i];
        }
    };

    typedef std::map<std::vector<float>, std::vector<DataType>, classcomp> archive_t;
    std::map<std::vector<float>, std::vector<DataType>, classcomp> data;
    typename archive_t::iterator end()
    {
        return data.end();
    }
    typename archive_t::iterator begin()
    {
        return data.begin();
    }
    void write(const std::ofstream &ofs)
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            std::vector<DataType> vec = it->second;
            for (size_t k = 0; k < vec.size(); ++k)
            {
                ofs << vec[k].base_features.transpose() << "\t" << vec[k].fitness << std::endl;
            }
        }
    }

    std::vector<DataType> &operator[](std::vector<float> bd)
    {
        return data[bd];
    }

    void push_back(const DataType &d)
    {
        base_features_t b = d.base_features;
        std::vector<float> bd(b.data(), b.data() + b.rows() * b.cols());
        std::vector<DataType> current_vec = data[bd];
        if (current_vec.empty())
        {
            current_vec.push_back(d);
            data[bd] = current_vec;
            ++sp;
            return;
        }
        for (size_t i = 0; i < current_vec.size(); ++i)
        {
            if (current_vec[i].fitness < d.fitness)
            {

                // keep going until you hit a larger fitness
                continue;
            }
            else
            {
                // any higher-fitness entry with similar behaviour?
                for (size_t j = i; j < current_vec.size(); ++j)
                {
                    if (equal(current_vec[j].base_features, d.base_features))
                    {
                        break; // if so, break
                    }
                }
                // if not, insert the new entry
                current_vec.insert(current_vec.begin() + i, d);
                if (current_vec.size() > k) // bin is full
                {
                    current_vec.erase(current_vec.begin()); //remove lowest-fitness element
                }
                else
                {
                    ++sp; // increment total count of solutions
                    if (full())
                    {
                        std::cout << "database full with sp=" << sp << "and k=" << k << std::endl;
                        prune();
                    }
                }
                data[bd] = current_vec;

                break; //stop
            }
        }
    }
    size_t get_capacity()
    {
        return capacity;
    }

    bool full()
    {
        return sp > capacity;
    }

    void prune()
    {
        if (k == 1)
        {
            std::cout << "k=1 so no more pruning allowed" << std::endl;
        }
        std::cout << "pruning " << std::endl;
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            std::vector<DataType> vec = it->second;
            if (vec.size() == k)
            {
                vec.erase(vec.begin());
                --sp;
            }
            data[it->first] = vec;
        }
        --k; // decrement k accordingly
        std::cout << "database pruned with sp=" << sp << "and k=" << k << std::endl;
    }

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar &BOOST_SERIALIZATION_NVP(k);
        ar &BOOST_SERIALIZATION_NVP(sp);
        ar &BOOST_SERIALIZATION_NVP(data);
    }
};

#endif
