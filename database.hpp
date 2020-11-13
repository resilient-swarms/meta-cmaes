

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

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {

        ar &BOOST_SERIALIZATION_NVP(sp);
        ar &BOOST_SERIALIZATION_NVP(max_sp);
        ar &BOOST_SERIALIZATION_NVP(data);
    }
};

// template <size_t capacity, typename DataType>
// struct MultiArrayDataBase // filtering based on BD, with approximately the same capacity, but solutions are only replaced by better ones in same behavioural bin
// {
//     typedef std::array<typename array_t::index, behav_dim> behav_index_t;
//     typedef std::array<float, behav_dim> point_t;
//     typedef boost::multi_array<DataEntry, behav_dim> array_t;
//     behav_index_t behav_shape;
//     size_t nb_evals;
//     bottom_pop_t _pop;

//     size_t get_capacity()
//     {
//         return capacity;
//     }
//     // EIGEN_MAKE_ALIGNED_OPERATOR_NEW  // note not needed when we use NoAlign
//     MultiArrayDataBase()
//     {
//         for (size_t i=0; )
//         assert(behav_dim == BottomParams::ea::behav_shape_size());
//         for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
//             behav_shape[i] = BottomParams::ea::behav_shape(i);
//         _array.resize(behav_shape);
//     }

//     void push_back(const base_features_t &b, const bottom_indiv_t &i1)
//     {
//         if (i1->fit().dead())
//             return;

//         point_t p = get_point(i1);
//         behav_index_t behav_pos;
//         for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
//         {
//             behav_pos[i] = round(p[i] * behav_shape[i]);
//             behav_pos[i] = std::min(behav_pos[i], behav_shape[i] - 1);
//             assert(behav_pos[i] < behav_shape[i]);
//         }

//         if (!_array(behav_pos) || (i1->fit().value() - _array(behav_pos)->fit().value()) > BottomParams::ea::epsilon || (fabs(i1->fit().value() - _array(behav_pos)->fit().value()) <= BottomParams::ea::epsilon && _dist_center(i1) < _dist_center(_array(behav_pos))))
//         {
//             _array(behav_pos) = i1;
//             return true;
//         }
//     }

//     point_t get_point()
//     {
//         point_t p;
//         for (size_t i = 0; i < NUM_BASE_FEATURES; ++i)
//             p[i] = std::min(1.0f, indiv->fit().desc()[i]);
//         return p;
//     }
// };

//typedef std::vector<DataEntry> database_t;// will become too long