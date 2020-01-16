#ifndef BOTTOM_TYPEDEFS_HPP
#define BOTTOM_TYPEDEFS_HPP

#define PRINTING
#include <rhex_dart/descriptors.hpp>
#include <sferes/eval/eval.hpp>
#include <meta-cmaes/params.hpp>

#include <meta-cmaes/global.hpp>

#include <boost/serialization/vector.hpp> // serialising database vector

#include <boost/serialization/array.hpp>
#include <Eigen/Dense>

//#include <boost/circular_buffer.hpp>
//#include <meta-cmaes/circular_buffer_serialisation.hpp>
#include <meta-cmaes/sampled.hpp>
#include <stdexcept>



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// BOTTOM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int NUM_BASE_FEATURES = 15;  // number of base features
// const int NUM_TOP_CELLS = 15;      // number of cells in the meta-map
const int NUM_BOTTOM_FEATURES = 3; // number of features for bottom level maps
const int NUM_GENES = NUM_BASE_FEATURES * NUM_BOTTOM_FEATURES;

/* base-features */
typedef Eigen::Matrix<float, NUM_BASE_FEATURES, 1, Eigen::DontAlign, NUM_BASE_FEATURES, 1> base_features_t; // 0 options and size cannot grow

/* weights to construct bottom-level map features from base_features */
typedef Eigen::Matrix<float, NUM_BOTTOM_FEATURES, NUM_BASE_FEATURES, Eigen::DontAlign, NUM_BOTTOM_FEATURES, NUM_BASE_FEATURES> weight_t;

/* bottom-level map features */
typedef Eigen::Matrix<float, NUM_BOTTOM_FEATURES, 1, Eigen::DontAlign, NUM_BOTTOM_FEATURES, 1> bottom_features_t;

// // bottom-level typedefs
//typedef sferes::eval::Eval<BottomParams> bottom_eval_t;

typedef sferes::gen::Sampled<24, BottomParams> bottom_gen_t; // 24 parameters for our controller
typedef size_t bottom_gen_data_t;                            // sampled data type is based on unsigned ints
typedef boost::fusion::vector<rhex_dart::safety_measures::BodyColliding, rhex_dart::safety_measures::MaxHeight, rhex_dart::safety_measures::TurnOver> base_safe_t;
typedef boost::fusion::vector<rhex_dart::descriptors::DutyCycle, rhex_dart::descriptors::BodyOrientation, rhex_dart::descriptors::AvgCOMVelocities> base_desc_t;
typedef rhex_controller::RhexControllerBuehler base_controller_t;
typedef rhex_dart::RhexDARTSimu<rhex_dart::safety<base_safe_t>, rhex_dart::desc<base_desc_t>> simulator_t;
// note to self:
// adding rhex_dart::rhex_control<base_controller_t> as first argument to Simu type seems to fail

namespace global
{

template <typename DataType>
struct DataEntry
{
  friend class boost::serialization::access;

  std::vector<DataType> genotype;
  base_features_t base_features;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  float fitness;
  DataEntry() {}
  DataEntry(const std::vector<DataType> &g, const base_features_t &b, const float &f) : genotype(g), base_features(b), fitness(f)
  {
  }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    ar &boost::serialization::make_nvp("base_features", base_features);
    ar &boost::serialization::make_nvp("fitness", fitness);
    ar &boost::serialization::make_nvp("genotype", genotype);
  }
};
struct SampledDataEntry : public DataEntry<size_t>
{
  SampledDataEntry() {}
  SampledDataEntry(const std::vector<size_t> &g, const base_features_t &b, const float &f) : DataEntry<size_t>(g, b, f)
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
};

struct EvoFloatDataEntry : public DataEntry<float>
{
  EvoFloatDataEntry() {}
  EvoFloatDataEntry(const std::vector<float> &g, const base_features_t &b, const float &f) : DataEntry<float>(g, b, f)
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
};

template <size_t capacity, typename DataType>
struct CircularBuffer
{
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
    if (sp > capacity)
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
    ar &boost::serialization::make_nvp("data", data);
    ar &boost::serialization::make_nvp("sp", sp);
    ar &boost::serialization::make_nvp("max_sp", max_sp);
  }
};

// struct DataBase   // filtering based on BD is problematic: either requires many checks or requires huge memory for fine-grained multi-array
// {
//   typedef std::array<typename array_t::index, behav_dim> behav_index_t;
//   typedef std::array<float, behav_dim> point_t;
//   typedef boost::multi_array<DataEntry, behav_dim> array_t;
//   behav_index_t behav_shape;
//   size_t nb_evals;
//   bottom_pop_t _pop;

//   weight_t W; //characteristic weight matrix of this map
//   // EIGEN_MAKE_ALIGNED_OPERATOR_NEW  // note not needed when we use NoAlign
//   DataBase()
//   {
//     assert(behav_dim == BottomParams::ea::behav_shape_size());
//     for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
//       behav_shape[i] = BottomParams::ea::behav_shape(i);
//     _array.resize(behav_shape);
//   }

//   void push_back(const base_features_t& b, const bottom_indiv_t& i1)
//   {
//     if (i1->fit().dead())
//       return;

//     point_t p = get_point(i1);
//     behav_index_t behav_pos;
//     for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
//     {
//       behav_pos[i] = round(p[i] * behav_shape[i]);
//       behav_pos[i] = std::min(behav_pos[i], behav_shape[i] - 1);
//       assert(behav_pos[i] < behav_shape[i]);
//     }

//     if (!_array(behav_pos) || (i1->fit().value() - _array(behav_pos)->fit().value()) > BottomParams::ea::epsilon || (fabs(i1->fit().value() - _array(behav_pos)->fit().value()) <= BottomParams::ea::epsilon && _dist_center(i1) < _dist_center(_array(behav_pos))))
//     {
//       _array(behav_pos) = i1;
//       return true;
//     }
//   }

//   point_t get_point()
//   {
//     point_t p;
//     for (size_t i = 0; i < NUM_BASE_FEATURES; ++i)
//       p[i] = std::min(1.0f, indiv->fit().desc()[i]);
//     return p;
//   }

// };

//typedef std::vector<DataEntry> database_t;// will become too long

// will use first-in-first-out queue such that latest DATABASE_SIZE individuals are maintained
typedef SampledDataEntry data_entry_t;
typedef CircularBuffer<BottomParams::MAX_DATABASE_SIZE, data_entry_t> database_t;
database_t database;
} // namespace global

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/

namespace sferes
{
namespace fit
{
class FitBottom
{

public:
  weight_t W;

  FitBottom() : _dead(false){};
  FitBottom(const weight_t &w) : W(w), _dead(false)
  {
  }

  void set_desc(const std::vector<float> &d)
  {
    _desc = d;
  }

  void set_value(float v)
  {
    _value = v;
  }
  std::vector<float> desc()
  {
    return _desc;
  }

  float value()
  {
    return _value;
  }
  template <typename Indiv>
  void eval(Indiv &indiv)
  {

    //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    this->_value = 0;
    this->_dead = false;
    _eval<Indiv>(indiv);
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // std::cout << "Time difference = " <<     std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
  }

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    //dbg::trace trace("fit", DBG_HERE);

    ar &boost::serialization::make_nvp("_value", this->_value);
  }

  bool dead() { return _dead; }
  std::vector<double> ctrl() { return _ctrl; }

  std::vector<float> get_desc(const base_features_t &b)
  {
#ifdef PRINTING

#endif
    bottom_features_t D = W * b;
    std::vector<float> vec(D.data(), D.data() + D.rows() * D.cols());
#ifdef PRINTING
    std::cout << " getting descriptor " << std::endl;
    std::cout << " w =  " << W << std::endl;
    std::cout << " b = " << b << std::endl;
    std::cout << " D = " << D << std::endl;
#endif
    return vec;
  }
  mode::mode_t mode() const
  {
    return _mode;
  }
  void set_mode(mode::mode_t m)
  {
    _mode = m;
  }

protected:
  mode::mode_t _mode;
  bool _dead;
  std::vector<double> _ctrl;
  float _value = 0.0f;
  std::vector<float> _desc;

  // descriptor work done here, in this case duty cycle
  template <typename Indiv>
  void _eval(Indiv &indiv)
  {
    // copy of controller's parameters
    _ctrl.clear();

    for (size_t i = 0; i < 24; i++)
      _ctrl.push_back(indiv.data(i));

    // launching the simulation
    auto robot = global::global_robot->clone();

    simulator_t simu(_ctrl, robot);
    simu.run(BottomParams::simu::time); // run simulation for 5 seconds

    this->_value = simu.covered_distance();

    std::vector<float> desc;
    base_features_t b;
    float dead = -1000.0f;
    // these assume a behaviour descriptor of size 6.
    if (dead > this->_value)
    {
      // this means that something bad happened in the simulation
      // we kill this individual
      this->_dead = true; // no need to do anything
      // desc.resize(6);
      // desc[0] = 0;
      // desc[1] = 0;
      // desc[2] = 0;
      // desc[3] = 0;
      // desc[4] = 0;
      // desc[5] = 0;
      // this->_value = -1000.0f;// note this causes troubles;
      // -> due to optimisation (presumably) the code is evaluated within if first, therefore the above condition seems to always be true
    }
    else
    {
      desc.resize(6);
      //get the base_features
      get_base_features(b, simu);
      // convert to final descriptor
      desc = get_desc(b);
      this->_desc = desc;
      this->_dead = false;
      //push to the database
#ifdef PRINTING
      std::cout << " adding entry with fitness " << this->_value << std::endl;
#endif
      global::database.push_back(global::data_entry_t(indiv.gen().data(), b, this->_value));
    }
  }

  /* the included descriptors determine the base-features */
  void get_base_features(base_features_t &base_features, simulator_t &simu)
  {

    std::vector<double> results;
    simu.get_descriptor<rhex_dart::descriptors::DutyCycle, std::vector<double>>(results);

    for (size_t i = 0; i < results.size(); ++i)
    {
      base_features(i, 0) = results[i];
    }

    simu.get_descriptor<rhex_dart::descriptors::BodyOrientation, std::vector<double>>(results);
    for (size_t i = 0; i < results.size(); ++i)
    {
      base_features(i + 6, 0) = results[i];
    }
    Eigen::Vector3d velocities;
    simu.get_descriptor<rhex_dart::descriptors::AvgCOMVelocities, Eigen::Vector3d>(velocities);// cf. skeleton : .54 .39 .139
    base_features(12, 0) = std::min(1.0,std::max(0.0, velocities[0] / (2.0*global::BODY_LENGTH)));// [0, 2] body lengths (moving backwards is unlikely; .54 is body length)
    base_features(13, 0) = std::min(1.0,std::max(0.0,(velocities[1] + 0.80*global::BODY_WIDTH) / (1.60*global::BODY_WIDTH)));// [-0.80,0.80] body widths, body cannot suddenly rotate heavily
    base_features(14, 0) = std::min(1.0,std::max(0.0,(velocities[2] + 1.0*global::BODY_HEIGHT) / (1.60*global::BODY_HEIGHT)));// [-1,0.60] body heights; body usually tilts backwards
  }
};
} // namespace fit
} // namespace sferes

// now that FitBottom is defined, define the rest of the bottom level
typedef sferes::fit::FitBottom bottom_fit_t;
typedef sferes::phen::Parameters<bottom_gen_t, bottom_fit_t, BottomParams> base_phen_t;
typedef boost::shared_ptr<base_phen_t> bottom_indiv_t;

class MapElites
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
  size_t nb_evals;
  bottom_pop_t _pop; // current batch

  weight_t W; //characteristic weight matrix of this map
  // EIGEN_MAKE_ALIGNED_OPERATOR_NEW  // note not needed when we use NoAlign
  MapElites()
  {
    assert(behav_dim == BottomParams::ea::behav_shape_size());
    for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
      behav_shape[i] = BottomParams::ea::behav_shape(i);
    _array.resize(behav_shape);
  }

  void random_pop()
  {
    // do not need it; random pop comes from meta-cmaes
  }

  // for resuming
  void _set_pop(const std::vector<boost::shared_ptr<base_phen_t>> &pop)
  {
#ifdef PRINTING

    std::cout << "setting pop " << std::endl;
#endif
    assert(!pop.empty());

    //        std::cout << this->res_dir() << " " << this->gen() << std::endl;

    //        std::string fname = ea.res_dir() + "/archive_" +
    //                boost::lexical_cast<std::string>(ea.gen()) +
    //                std::string(".dat");

    for (size_t h = 0; h < pop.size(); ++h)
    {
      //            std::cout << "Fitness of ind " << h << " is " << pop[h]->fit().value() << std::endl;
      //            std::cout << "Descriptor is " ; //<< pop[h]->fit().desc()[0] << std::endl;
      //            for (size_t desc_index = 0; desc_index < pop[h]->fit().desc().size(); ++desc_index)
      //                std::cout << pop[h]->fit().desc()[desc_index] << " ";
      //            std::cout << std::endl;

      //            pop[h]->develop();
      //            pop[h]->fit().eval(*pop[h]);  // we need to evaluate the individuals to get the descriptor values

      point_t p = get_point(pop[h]);

      behav_index_t behav_pos;
      for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
      {
        behav_pos[i] = round(p[i] * behav_shape[i]);
        behav_pos[i] = std::min(behav_pos[i], behav_shape[i] - 1);
        assert(behav_pos[i] < behav_shape[i]);
      }
      _array(behav_pos) = pop[h];
    }
  }

  void epoch()
  {
    this->_pop.clear();

    for (const bottom_phen_ptr_t *i = _array.data(); i < (_array.data() + _array.num_elements()); ++i)
      if (*i)
        this->_pop.push_back(*i);
#ifdef PRINTING
    std::cout << "start map-elites epoch with pop of " << this->_pop.size() << " individuals " << std::endl;
    std::cout << "start map-elites epoch with array of " << this->_array.size() << " individuals " << std::endl;
#endif
    bottom_pop_t ptmp;
    for (size_t i = 0; i < BottomParams::pop::size; ++i)
    {
      bottom_indiv_t p1 = _selection(this->_pop);
      bottom_indiv_t p2 = _selection(this->_pop);
      boost::shared_ptr<base_phen_t> i1, i2;
      p1->cross(p2, i1, i2);
      i1->mutate();
      i2->mutate();
      i1->develop();
      i2->develop();
      ptmp.push_back(i1);
      ptmp.push_back(i2);
    }

    eval_map(ptmp, 0, ptmp.size());

    for (size_t i = 0; i < ptmp.size(); ++i)
    {
      _add_to_archive(ptmp[i]);
    }
    nb_evals += ptmp.size();
  }

  long int getindex(const array_t &m, const bottom_phen_ptr_t *requestedElement, const unsigned short int direction) const
  {
    int offset = requestedElement - m.origin();
    return (offset / m.strides()[direction] % m.shape()[direction] + m.index_bases()[direction]);
  }

  behav_index_t getindexarray(const array_t &m, const bottom_phen_ptr_t *requestedElement) const
  {
    behav_index_t _index;
    for (unsigned int dir = 0; dir < behav_dim; dir++)
    {
      _index[dir] = getindex(m, requestedElement, dir);
    }

    return _index;
  }

  const array_t &archive() const
  {
    return _array;
  }

  template <typename I>
  point_t get_point(const I &indiv) const
  {
    return _get_point(indiv);
  }

  bottom_indiv_t _selection(const bottom_pop_t &pop)
  {
    int x1 = misc::rand<int>(0, pop.size());
    return pop[x1];
  }

  void eval_map(std::vector<boost::shared_ptr<base_phen_t>> &pop, size_t begin, size_t end)
  {
    //dbg::trace trace("eval", DBG_HERE);
    assert(pop.size());
    assert(begin < pop.size());
    assert(end <= pop.size());
    for (size_t i = begin; i < end; ++i)
    {
      eval_individual(pop[i]);
    }
  }

  void eval_individual(boost::shared_ptr<base_phen_t> &ind)
  {

    ind->fit() = bottom_fit_t(W);
    ind->develop();
    ind->fit().eval<base_phen_t>(*ind);
  }

  void do_epochs(size_t num_epochs)
  {
    nb_evals = 0;
    for (size_t i = 0; i < num_epochs; ++i)
    {
      this->epoch();
    }
  }

  bool _add_to_archive(bottom_indiv_t &i1)
  {
    if (i1->fit().dead())
    {
      return false;
    }

    point_t p = _get_point(i1);

    behav_index_t behav_pos;
    for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
    {
      behav_pos[i] = round(p[i] * behav_shape[i]);
      behav_pos[i] = std::min(behav_pos[i], behav_shape[i] - 1);
      assert(behav_pos[i] < behav_shape[i]);
    }

    if (!_array(behav_pos) || (i1->fit().value() - _array(behav_pos)->fit().value()) > BottomParams::ea::epsilon || (fabs(i1->fit().value() - _array(behav_pos)->fit().value()) <= BottomParams::ea::epsilon && _dist_center(i1) < _dist_center(_array(behav_pos))))
    {
      _array(behav_pos) = i1;
      _non_empty_indices.insert(behav_pos);
      return true;
    }
    return false;
  }
  std::vector<bottom_indiv_t> sample_individuals()
  {
    size_t num_individuals = std::max(1, (int)std::round(CMAESParams::pop::percentage_evaluated * _non_empty_indices.size()));
    return _pick(_non_empty_indices.size(), num_individuals);
  }

protected:
  array_t _array;
  array_t _prev_array;
  std::set<behav_index_t> _non_empty_indices; // all non-empty solutions' indices stored here


  template <typename T>
  T _get_Nth_Element(std::set<T> &searchSet, int n)
  {
     return *(std::next(searchSet.begin(), n));
  }
  std::vector<bottom_indiv_t> _pick(size_t N, size_t k)
  {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::unordered_set<size_t> elems = _pickSet(N, k, gen);

    std::vector<bottom_indiv_t> result;
    for (size_t el : elems)
    {
      behav_index_t pos = _get_Nth_Element<behav_index_t>(_non_empty_indices,el);
#ifdef PRINTING
      std::cout << "chosen position: ";
      for (int i = 0; i < behav_dim; ++i)
      {
        std::cout << pos[i] << ",";
      }
      std::cout << "\n";
#endif
      result.push_back(_array(pos));
    }
    return result;
  }
  // sampling without replacement (see https://stackoverflow.com/questions/28287138/c-randomly-sample-k-numbers-from-range-0n-1-n-k-without-replacement)
  std::unordered_set<size_t> _pickSet(size_t N, size_t k, std::mt19937 &gen)
  {
    std::uniform_int_distribution<> dis(0, N-1);
    std::unordered_set<size_t> elems;
    elems.clear();

    while (elems.size() < k)
    {
      elems.insert(dis(gen));
    }

    return elems;
  }
  template <typename I>
  float _dist_center(const I &indiv)
  {
    /* Returns distance to center of behavior descriptor cell */
    float dist = 0.0;
    point_t p = _get_point(indiv);
    for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
      dist += pow(p[i] - (float)round(p[i] * (float)(behav_shape[i] - 1)) / (float)(behav_shape[i] - 1), 2);

    dist = sqrt(dist);
    return dist;
  }

  template <typename I>
  point_t _get_point(const I &indiv) const
  {
    point_t p;
    for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
      p[i] = std::min(1.0f, indiv->fit().desc()[i]);

    return p;
  }
};

/* phenotype has members _gen and _fit, the genotype and fitmap, 
                        and is responsible for individuals random init, mutation, development */
template <typename Phen>
class MapElitesPhenotype : public Phen, public MapElites
{
public:
  MapElitesPhenotype(){}; // will create random genotype and no fitness

  /*  set the weights (genotype)    */
  // void set_weights()
  // {
  //   W = this->gen().data(); // get the genotype

  //   // #ifdef PRINTING
  //   //     std::cout << "setting weights (genotype) of the phenotype "<< std::endl;
  //   //     for (float w : weights)
  //   //     {
  //   //       std::cout << w <<",";
  //   //     }
  //   //     std::cout <<" \n " << W << std::endl;
  //   // #endif
  // }
  void genotype_to_mat(const std::vector<float> &weights)
  {
    size_t count = 0;
#ifdef PRINTING
    std::cout << "before conversion " << std::endl;
#endif
    for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
    {
      float sum = std::accumulate(weights.begin() + j * NUM_BASE_FEATURES, weights.begin() + (j + 1) * NUM_BASE_FEATURES, 0.0);
      for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
      {
        W(j, k) = weights[count] / sum; // put it available for the MapElites parent class

#ifdef PRINTING
        std::cout << "sum " << sum << std::endl;
        std::cout << weights[count] << std::endl;
        std::cout << W(j, k) << "," << std::endl;
#endif
        ++count;
      }
    }
#ifdef PRINTING
    std::cout << "after conversion " << std::endl;
    std::cout << W << std::endl;
#endif
  }
  void random_pop()
  {
    for (size_t i = 0; i < BottomParams::pop::init_size; ++i)
    {
      boost::shared_ptr<base_phen_t> indiv(new base_phen_t());
      indiv->random();
      eval_individual(indiv);
    }
  }
  void develop()
  {
#ifdef PRINTING
    std::cout << "start developing the map-phenotype" << std::endl;
#endif
    /* do develop of the subclass phenotype*/
    Phen::develop();

    /* fill map j with individuals */
    genotype_to_mat(this->gen().data());
    for (int i = 0; i < global::database.size(); ++i)
    {
      entry_to_map(global::database[i], W); // obtain behavioural features and put individuals in the map
    }
#ifdef PRINTING
    std::cout << "stop developing the map-phenotype" << std::endl;
#endif
  }

  void entry_to_map(const global::data_entry_t &entry, const weight_t &weight)
  {

    // use weight and base features --> bottom-level features

    // create new individual
    boost::shared_ptr<base_phen_t> individual(new base_phen_t());
    //
    individual->fit() = bottom_fit_t(weight);
    individual->fit().set_desc(individual->fit().get_desc(entry.base_features));
    individual->fit().set_value(entry.fitness);
    entry.set_genotype<boost::shared_ptr<base_phen_t>>(individual);

    this->_add_to_archive(individual);
#ifdef PRINTING
    std::cout << "added individual to archive " << std::endl;
#endif
  }
};

#endif
