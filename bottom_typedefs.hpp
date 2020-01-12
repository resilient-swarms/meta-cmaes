#ifndef BOTTOM_TYPEDEFS_HPP
#define BOTTOM_TYPEDEFS_HPP

#include <rhex_dart/descriptors.hpp>
#include <sferes/eval/eval.hpp>
#include <meta-cmaes/params.hpp>

#include <meta-cmaes/global.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// BOTTOM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int NUM_BASE_FEATURES = 12;  // number of base features
const int NUM_TOP_CELLS = 15;      // number of cells in the meta-map
const int NUM_BOTTOM_FEATURES = 6; // number of features for bottom level maps
const int NUM_GENES = NUM_BASE_FEATURES * NUM_BOTTOM_FEATURES;

/* base-features */
typedef Eigen::Matrix<float, NUM_BASE_FEATURES, 1, 0, NUM_BASE_FEATURES, 1> base_features_t; // 0 options and size cannot grow

/* weights to construct bottom-level map features from base_features */
typedef Eigen::Matrix<float, NUM_BOTTOM_FEATURES, NUM_BASE_FEATURES, 0, NUM_BOTTOM_FEATURES, NUM_BASE_FEATURES> weight_t;

/* bottom-level map features */
typedef Eigen::Matrix<float, NUM_BOTTOM_FEATURES, 1, 0, NUM_BOTTOM_FEATURES, 1> bottom_features_t;

// // bottom-level typedefs
//typedef sferes::eval::Eval<BottomParams> bottom_eval_t;

typedef sferes::gen::Sampled<24, BottomParams> bottom_gen_t; // 24 parameters for our controller

typedef boost::fusion::vector<rhex_dart::safety_measures::BodyColliding, rhex_dart::safety_measures::MaxHeight, rhex_dart::safety_measures::TurnOver> base_safe_t;
typedef rhex_dart::descriptors::BodyOrientation base_desc_t;
typedef rhex_controller::RhexControllerBuehler base_controller_t;
typedef rhex_dart::RhexDARTSimu<rhex_dart::safety<base_safe_t>, rhex_dart::desc<base_desc_t>> simulator_t;
// note to self: 
// adding rhex_dart::rhex_control<base_controller_t> as first argument to Simu type seems to fail


namespace global
{
struct DataEntry
{
  base_features_t base_features;
  float fitness;
  DataEntry() {}
  DataEntry(const base_features_t &b, const float &f) : base_features(b), fitness(f)
  {
  }
};

typedef std::vector<DataEntry> database_t;
database_t database;
} // namespace global

/* bottom-level fitmap 
used to evaluate behavioural descriptor and fitness of controllers in the normal operating environment
*/
class FitBottom
{

public:
  weight_t W;
  FitBottom(){};
  FitBottom(const weight_t &w) : W(w)
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
    dbg::trace trace("fit", DBG_HERE);

    ar &boost::serialization::make_nvp("_value", this->_value);
  }

  bool dead() { return _dead; }
  std::vector<double> ctrl() { return _ctrl; }

  std::vector<float> get_desc(const base_features_t &b)
  {
    bottom_features_t D = W * b;
    std::vector<float> vec(D.data(), D.data() + D.rows() * D.cols());

    return vec;
  }

protected:
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
    simu.run(5); // run simulation for 5 seconds

    this->_value = simu.covered_distance();

    std::vector<float> desc;
    base_features_t b;
    // these assume a behaviour descriptor of size 6.
    if (this->_value < -1000)
    {
      // this means that something bad happened in the simulation
      // we kill this individual
      this->_dead = true;
      desc.resize(6);
      desc[0] = 0;
      desc[1] = 0;
      desc[2] = 0;
      desc[3] = 0;
      desc[4] = 0;
      desc[5] = 0;
      this->_value = -1000;
    }
    else
    {
      desc.resize(6);
      //get the base_features
      get_base_features(b, simu);
      // convert to final descriptor
      desc = get_desc(b);
    }

    this->_desc = desc;
    //push to the database
    global::database.push_back(global::DataEntry(b, this->_value));
  }

  /* the included descriptors determine the base-features */
  void get_base_features(base_features_t &base_features, simulator_t &simu)
  {

    std::vector<double> results;
    simu.get_descriptor<base_desc_t, std::vector<double>>(results);

    for (size_t i = 0; i < results.size(); ++i)
    {
      base_features(i, 0) = results[i];
    }
  }
};
// now that FitBottom is defined, define the rest of the bottom level
typedef FitBottom bottom_fit_t;
typedef sferes::phen::Parameters<bottom_gen_t, bottom_fit_t, BottomParams> base_phen_t;
typedef boost::shared_ptr<base_phen_t> bottom_indiv_t;

namespace rhex_dart
{
namespace descriptors
{

template <typename D1, typename D2>
struct CombinedDescriptor
{
  D1 first_desc;
  D2 second_desc;
  CombinedDescriptor()
  {
    first_desc = D1();
    second_desc = D2();
  }

  template <typename Simu, typename robot>
  void operator()(Simu &simu, std::shared_ptr<robot> rob, const Eigen::Vector6d &init_trans)
  {
    first_desc(simu, rob, init_trans);
    second_desc(simu, rob, init_trans);
  }

  void get(base_features_t &b)
  {
    std::vector<float> results;
    first_desc.get(results);

    std::vector<float> results2;
    second_desc.get(results2);

    // std::vector<float> results;
    // simu.get_descriptor<D1, std::vector<float>>(results);

    // std::vector<float> results2;
    // simu.get_descriptor<D2, std::vector<float>>(results2);

    // results.insert(results.end(), results2.begin(), results2.end());
    // float *v = &results[0];
    // b(v, results.size());
    for (size_t i = 0; i < results.size(); ++i)
    {
      b(i, 0) = results[i];
    }
    for (size_t i = results.size(); i < results.size() + results2.size(); ++i)
    {
      b(i, 0) = results2[i];
    }
  }
};

} // namespace descriptors

} // namespace rhex_dart

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
  bottom_pop_t _pop;

  weight_t W; //characteristic weight matrix of this map

  MapElites()
  {
    assert(behav_dim == BottomParams::ea::behav_shape_size());
    for (size_t i = 0; i < BottomParams::ea::behav_shape_size(); ++i)
      behav_shape[i] = BottomParams::ea::behav_shape(i);
    _array.resize(behav_shape);
  }

  void random_pop()
  {
    // parallel::init();// we do not need this
    this->_pop.resize(BottomParams::pop::init_size);
    BOOST_FOREACH (boost::shared_ptr<base_phen_t> &indiv, this->_pop)
    {
      indiv = boost::shared_ptr<base_phen_t>(new base_phen_t());
      indiv->random();
    }
    eval(this->_pop, 0, this->_pop.size());
    BOOST_FOREACH (boost::shared_ptr<base_phen_t> &indiv, this->_pop)
      _add_to_archive(indiv);
  }

  // for resuming
  void _set_pop(const std::vector<boost::shared_ptr<base_phen_t>> &pop)
  {
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

    eval(ptmp, 0, ptmp.size());

    for (size_t i = 0; i < ptmp.size(); ++i)
    {
      _add_to_archive(ptmp[i]);
    }
    nb_evals += 2 * BottomParams::pop::size;
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

  void eval(std::vector<boost::shared_ptr<base_phen_t>> pop, size_t begin, size_t end)
  {
    dbg::trace trace("eval", DBG_HERE);
    assert(pop.size());
    assert(begin < pop.size());
    assert(end <= pop.size());
    for (size_t i = begin; i < end; ++i)
    {
      pop[i]->fit() = FitBottom(W);
      pop[i]->develop();
      pop[i]->fit().eval<base_phen_t>(*pop[i]);
    }
  }

  void do_epochs(size_t num_epochs)
  {
    nb_evals = 0;
    for (size_t i = 0; i < num_epochs; ++i)
    {
      this->epoch();
    }
  }

  bool _add_to_archive(bottom_indiv_t i1)
  {
    if (i1->fit().dead())
      return false;

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
      return true;
    }
    return false;
  }

protected:
  array_t _array;
  array_t _prev_array;

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
  void set_weights()
  {
    W = this->gen().data(); // get the genotype

#ifdef PRINTING
    for (float w : weights)
    {
      std::cout << w <<, ;
    }
    std::cout <<\n W << std::endl;
#endif
  }

  void develop()
  {
    /* do develop of the subclass phenotype*/
    Phen::develop();

    /* fill map j with individuals */
    std::vector<float> weights = this->gen().data(); // get the new weight matrix
    size_t count = 0;
    for (size_t j = 0; j < NUM_BOTTOM_FEATURES; ++j)
      for (size_t k = 0; k < NUM_BASE_FEATURES; ++k)
        W(j, k) = weights[count]; // put it available for the MapElites parent class
    ++count;
    for (int i = 0; i < global::database.size(); ++i)
    {
      bottom_indiv_t individual = entry_to_bottomindividual(global::database[i], W); // obtain behavioural features and put individuals in the map
      this->_add_to_archive(individual);
    }
  }

  bottom_indiv_t entry_to_bottomindividual(const global::DataEntry &entry, const weight_t &weight)
  {

    // use weight and base features --> bottom-level features

    // create new individual
    boost::shared_ptr<base_phen_t> individual = boost::shared_ptr<base_phen_t>(new base_phen_t());
    //
    individual->fit() = FitBottom(weight);
    individual->fit().set_desc(individual->fit().get_desc(entry.base_features));
    individual->fit().set_value(entry.fitness);

    return individual;
  }
};

#endif
