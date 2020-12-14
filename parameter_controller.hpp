/* note this is a translation of RLController in http://www.few.vu.nl/~gks290/resources/gecco2014.tar.gz */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <sstream>
//
class ParameterController
{
protected:
    class Transition
    {
    public:
        std::vector<float> input_;
        int action_;
        std::vector<float> result_;
        float reward_;
        float value_;

        Transition(const std::vector<float> &input, int action, const std::vector<float> &result, float reward)
        {
            input_ = input;
            action_ = action;
            result_ = result;
            reward_ = reward;
            //	value_ = reward_ + getState(result_).V_;
        }
    };

    class TransitionComparator
    {
    protected:
        int attribute_;

    public:
        TransitionComparator()
        {
            attribute_ = 0;
        }
        void setAttribute(int a)
        {
            attribute_ = a;
        }
        bool operator()(const Transition &o1, const Transition &o2)
        {
            if (o1.input_[attribute_] < o2.input_[attribute_]) // sort ascending order
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };
    class Random
    {
        std::random_device rd;
        std::mt19937 gen;

    public:
        Random() {}
        Random(long seed)
        {
            gen = std::mt19937(seed);
        }
        /* int in [0,num-1] */
        int nextInt(size_t num)
        {
            std::uniform_int_distribution<unsigned> distrib(0, num - 1);
            return distrib(gen);
        }
        /* float in [0,1] */
        float nextFloat()
        {
            std::uniform_real_distribution<float> distrib(0.0f, 1.0f);
            return distrib(gen);
        }
    };
    class TreeNode
    {
    public:
        int id_;
        TreeNode *left_, *right_;
        std::vector<float> Q_;
        float V_;
        float trace_;
        std::vector<float> action_traces_;
        int attribute_;
        float point_;
        std::vector<Transition> transitions_;

        TreeNode(int actions, int id)
        {
            // ID
            id_ = id;
            // Children
            left_ = NULL;
            right_ = NULL;
            // Values
            V_ = 0;
            Q_ = std::vector<float>(actions);
            for (int i = 0; i < actions; i++)
                Q_[i] = 0;
            // Traces
            trace_ = 0;
            action_traces_ = std::vector<float>(actions);
            for (int i = 0; i < actions; i++)
                action_traces_[i] = 0;
            // Transition points archive
            transitions_ = std::vector<Transition>();
        }
        void updateValues(float alpha, float error)
        {
            // The max Q value found
            float max = 0;
            for (int i = 0; i < Q_.size(); i++)
            {
                // Calculate Q' and update max
                Q_[i] += alpha * error * action_traces_[i];
                if (Q_[i] > max)
                    max = Q_[i];
            }
            // The state value V = max Q(s,a)
            V_ = max;
        }
        bool decayTraces(float gamma, float lambda, float threshold)
        {
            // Decay state trace
            trace_ *= gamma * lambda;
            // Decay all action traces of the state
            for (int j = 0; j < action_traces_.size(); j++)
                action_traces_[j] *= gamma * lambda;
            // The highest action trace cannot be higher than the state trace
            // If the state trace is below the threshold, return false
            if (trace_ < threshold)
                return false;
            return true;
        }
        std::vector<float> toFloat()
        {
            // Contents:
            //		Internal Split Nodes:
            //								0: 0 (type)
            //								1: ID
            //								2: attribute (used for splitting)
            //								3: split value
            //								4: ID of left child
            //								5: ID of right child
            //		Leaf State Nodes:
            //								0: 1 (type)
            //								1: ID
            //								2: State value V
            //								3: The state trace value
            //								4: The size of the transitions archive
            //								5 - actions+4: The action values Q(a)
            //								actions+6 - 2*actions+4: The action trace values t(a)
            // Leaf state node
            if (left_ == NULL)
            {
                int actions = Q_.size();
                std::vector<float> res(2 * Q_.size() + 5);
                res[0] = 1;
                res[1] = id_;
                res[2] = V_;
                res[3] = trace_;
                res[4] = transitions_.size();
                for (int i = 0; i < actions; i++)
                    res[i + 5] = Q_[i];
                for (int i = 0; i < actions; i++)
                    res[i + actions + 5] = action_traces_[i];
                return res;
                // Internal split node
            }
            else
            {
                std::vector<float> res(6);
                res[0] = 0;
                res[1] = id_;
                res[2] = attribute_;
                res[3] = point_;
                res[4] = left_->id_;
                res[5] = right_->id_;
                return res;
            }
        }
    };
    struct KolmogorovSmirnoff
    {
        static std::pair<float, float> range(const std::vector<float> &a)
        {
            float min = +INFINITY;
            float max = -INFINITY;
            for (float xx : a)
            {
                if (xx < min)
                {
                    min = xx;
                }
                else
                {
                    if (xx > max)
                    {
                        max = xx;
                    }
                }
            }
            return std::pair<float, float>{min, max};
        }
        static float critical_value(float alpha)
        {
            return std::sqrt(-(alpha / 2.0f) / 2.0f);
        }
        static float proportion_leq(const std::vector<float> &a, float x)
        {
            float step = 1.f / (float)a.size();
            float prop = 0.0f;
            for (float xx : a)
            {
                if (xx <= x)
                {
                    prop += step;
                }
            }
            return prop;
        }
        static float max_diff(const std::vector<float> &a, const std::vector<float> &b)
        {
            std::pair<float, float> r = range(a);
            std::pair<float, float> r2 = range(b);
            float min = std::min(std::get<0>(r), std::get<0>(r2));
            float max = std::max(std::get<1>(r), std::get<1>(r2));
            float stepsize = 0.001f * (max - min);
            float maxdiff = 0.f;
            // iterate over a
            for (float x : a)
            {
                float p1 = proportion_leq(a, x);
                float p2 = proportion_leq(b, x);
                float diff = std::abs(p1 - p2);
                if (diff > maxdiff)
                {
                    maxdiff = diff;
                }
            }
            // iterate over b
            for (float x : b)
            {
                float p1 = proportion_leq(a, x);
                float p2 = proportion_leq(b, x);
                float diff = std::abs(p1 - p2);
                if (diff > maxdiff)
                {
                    maxdiff = diff;
                }
            }
            return maxdiff;
        }
        static bool significant(float md, float m, float n, float alpha)
        {
            return md > std::sqrt(-std::log(alpha / 2.0f) / 2.0f) * std::sqrt((m + n) / (n * m));
        }
        static float normalised_dvalue(const std::vector<float> &a, const std::vector<float> &b)
        {
            float n = (float)a.size();
            float m = (float)b.size();
            float md = max_diff(a, b) / std::sqrt((n + m) / (n * m));
            return md;
        }
    };
    // The type of the parameter, i.e. numeric or symbolic. For numeric parameters there are options for int or
    // float to simplify getting and setting.
    enum ParameterType
    {
        NUMERIC_DOUBLE_,
        NUMERIC_INT_,
        SYMBOLIC_
    };
    // Random
    Random *random_;
    long seed_;
    // Parameters, types and ranges
    std::vector<std::string> parameters_;
    std::map<std::string, ParameterType> parameter_types_;            // Parameter types
    std::map<std::string, std::pair<float, float>> parameter_ranges_; // Ranges of parameters
    // Observables
    std::vector<float> current_observables_;
    bool initialized_ = false;

public:
    //--------------------------------Controller Interface----------------------------------//
    void initialize(long seed, const std::string &settings)
    {
        // Random
        seed_ = seed;
        random_ = new Random(seed_);
        // Make sure there is at least one parameter controlled
        if (parameter_types_.empty())
            throw std::runtime_error("Attempting to initialize controller without having added any parameters to be controlled.");
        // Specific initialization
        initializeSpecific(settings);
        initialized_ = true;
    }
    void addParameter(std::string name, ParameterType type, float min, float max)
    {

        if (initialized_)
            throw std::runtime_error("Attempting to change settings after initialization.");
        parameters_.push_back(name);
        parameter_types_[name] = type;
        parameter_ranges_[name] = std::pair<float, float>{min, max};
    }
    float getNextValue(std::string name)
    {
        if (!initialized_)
            throw std::runtime_error("Attempting to get parameter value before initializing.");
        if (std::find(parameters_.begin(), parameters_.end(), name) == parameters_.end())
            throw new std::runtime_error("Attempting to get value of nonexistent parameter with name '" + name + "'. Add a parameter using 'addParameter()' before initialization.");
        return implementNextValue(name);
    }
    void updateObservables(const std::vector<float> &obs)
    {
        current_observables_ = obs;
        implementUpdate(obs);
    }
    std::vector<float> getStats()
    {
        if (!initialized_)
            throw std::runtime_error("Attempting to get stats before initializing.");
        return implementStats();
    }
    //--------------------------------------------------------------------------------------//

    //-----------------------------------Implementation-------------------------------------//
    virtual float implementNextValue(const std::string& name) = 0;
    virtual void initializeSpecific(const std::string& settings) = 0;
    virtual void implementUpdate(const std::vector<float> &obs) = 0;
    virtual std::vector<float> implementStats() = 0;
    virtual std::string getName() = 0;
    //--------------------------------------------------------------------------------------//
};

class RLController : public ParameterController
{

private:
    // Action discretization bins
    int action_bins_ = 5;
    // Actions encoding into single number
    int action_num_ = 0;              // Number of action combinations
    std::vector<int> action_periods_; // Period of each action in single encoding
    std::vector<int> action_sizes_;   // Number of distinct values for each action
    // The state tree
    TreeNode *state_tree_;
    int tree_size_;
    int split_nodes_;
    int state_nodes_;
    int node_serial_;
    // RL parameters
    float epsilon_ = 0.1;
    float gamma_ = 0.8;
    float alpha_ = 0.9;
    float alpha_zero_ = 0.2;
    // UTree parameters
    int split_threshold_ = 60;
    float min_sample_ = 0.2;
    float ks_p_threshold_ = 0.05;
    int split_fail_threshold_ = 10;
    // Eligibility traces
    std::set<TreeNode *> eligible_states_;
    float eligibility_threshold_ = 0.001;
    float lambda_ = 0.8;
    // The current state and action
    std::vector<float> current_observables_;
    TreeNode *current_state_;
    int current_action_;
    // The number of steps/updates
    //	int updates_;
    //	long t_;
protected:
    //-----------------------------Controller Implementation--------------------------------//

    void initializeSpecific(const std::string& settings)
    {
        std::stringstream ss(settings);
        // Get settings
        while (ss.good())
        {
            std::string substring;
            std::getline(ss, substring, ';');
            if (substring.rfind("bins:", 0) == 0)
                action_bins_ = std::atoi(substring.substr(5).c_str());
            else if (substring.rfind("epsilon:", 0) == 0)
                epsilon_ = std::atof(substring.substr(8).c_str());
            else if (substring.rfind("gamma:", 0) == 0)
                gamma_ = std::atof(substring.substr(6).c_str());
            else if (substring.rfind("alpha:", 0) == 0)
                alpha_ = std::atof(substring.substr(6).c_str());
            else if (substring.rfind("lambda:", 0) == 0)
                lambda_ = std::atof(substring.substr(7).c_str());
            else if (substring.rfind("splitat:", 0) == 0)
                split_threshold_ = std::atof(substring.substr(8).c_str());
            else
                throw std::runtime_error("unrecognised settings");
        }

        // Calculate number, periods and sizes of actions
        action_num_ = 1;
        action_sizes_ = std::vector<int>(parameters_.size());
        action_periods_ = std::vector<int>(parameters_.size());
        for (int i = 0; i < parameters_.size(); i++)
        {
            if (parameter_types_[parameters_[i]] == ParameterType::SYMBOLIC_)
            {
                // For symbolic use all possible values
                std::pair<float, float> rng = parameter_ranges_[parameters_[i]];
                action_sizes_[i] = (int)(std::get<1>(rng) - std::get<0>(rng));
            }
            else
            {
                // For numeric parameters use the number of bins
                action_sizes_[i] = action_bins_;
            }
            action_num_ *= action_sizes_[i];
            action_periods_[i] = action_num_;
        }
        std::cout << "Actions: " << action_num_ << std::endl;
        // Initialize state tree
        state_tree_ = new TreeNode(action_num_, 0);
        node_serial_ = 1;
        tree_size_ = 1;
        state_nodes_ = 1;
        split_nodes_ = 0;
        // Initialize eligibility list
        //eligible_states_ = std::vector<TreeNode*>();
        // Initial current state/action
        current_state_ = state_tree_;
        current_action_ = random_->nextInt(action_num_);
        // Initialize update count
        //	updates_ = 0;
        //
        //		t_ = System.currentTimeMillis();
    }
    virtual float implementNextValue(const std::string& name)
    {
        auto it = std::find(parameters_.begin(), parameters_.end(), name);
        int index = it - parameters_.begin();
        int a = decodeAction(current_action_)[index];
        std::pair<float, float> rng = parameter_ranges_[name];
        if (parameter_types_[name] == ParameterType::SYMBOLIC_)
        {
            // For symbolic parameters just return the value
            return a + std::get<0>(rng);
        }
        else
        {
            // For numeric parameters return a random value within the selected bin
            return std::get<0>(rng) + ((std::get<1>(rng) - std::get<0>(rng)) / action_bins_) * (a + random_->nextFloat());
        }
    }
    virtual void implementUpdate(const std::vector<float>& obs)
    {
        // Increase counter
        //	updates_++;
        // The first number is the reward signal
        float reward = obs[0];
        // The rest of the vector is the observables input
        std::vector<float> input(obs.size() - 1);
        for (int i = 0; i < input.size(); i++)
            input[i] = obs[i + 1];
        // Get next state from tree and decide action
        TreeNode *next_state = getState(input);
        int next_action = selectAction(next_state);
        // Calculate TD error
        float action_delta = reward + gamma_ * next_state->Q_[next_action] - current_state_->Q_[current_action_];
        // Update trace of current state and state-action pair and add to eligible list if necessary
        current_state_->action_traces_[current_action_] = 1;
        current_state_->trace_ = 1;

        eligible_states_.insert(current_state_);
        // Update all eligible states
        float alpha = alpha_;
        if (reward == 0)
            alpha = alpha_zero_;
        for (auto s : eligible_states_)
        {
            s->updateValues(alpha, action_delta); // Update values
            if (!s->decayTraces(gamma_, lambda_, eligibility_threshold_))
                eligible_states_.erase(s);
        }
        // Add transition to state archive (if not first action)
        if (current_observables_.empty())
            current_state_->transitions_.push_back(Transition(current_observables_, current_action_, input, reward));
        // Try splitting if the transition archive is bigger than the threshold
        if (current_state_->transitions_.size() > split_threshold_ && random_->nextFloat() < 0.1)
        {
            // If the state split was the next state, re-get next state
            if (splitState(current_state_))
            {
                if (next_state == current_state_)
                    next_state = getState(input);
                //		ks_p_threshold_ = 0.05;
            } //else{
            //		if(current_state_.transitions_.size()> split_fail_threshold_*split_threshold_ && ks_p_threshold_<0.5) ks_p_threshold_ += 0.005;
            //	}
        }
        /*if(updates_%split_threshold_==0){
			// Update the tree
			splitTree(state_tree_);
			// Re-get the next state because it may have been split
			next_state = getState(input);
		}*/
        // Step, set current state and action
        current_observables_ = input;
        current_state_ = next_state;
        current_action_ = next_action;
    }
    virtual std::vector<float> implementStats()
    {
        // This a dynamic controller, thus the stats change with every update
        // Contents:
        //			0: The number of bins used
        //			1: The total number of actions
        //			2: The epsilon value (for the epsilon-greedy action selection)
        //			3: The alpha value (for the TD update)
        //			4: The gamma value (for the TD update)
        //			5: The minimum archive size required for splitting a state
        //			6: The minimum amount of datapoints for a new state when splitting
        //			7: The minimum P value of the Kolomogorov-Smirnov test for accepting a split
        //			8: The lambda value (for the eligibility traces)
        //			9: The minimum trace required for treating a state
        //			10: The size of the tree
        //			11: The number of internal split nodes
        //			12: The number of leaf state nodes
        //	REMOVED!13 - end: The tree
        //	float tree[] = new float[(split_nodes_*6)+(state_nodes_*(2*action_num_+5))];
        //	treeToFloat(state_tree_, tree, 0);
        //	float res[] = new float[13+tree.size()];
        std::vector<float> res(13);
        res[0] = action_bins_;
        res[1] = action_num_;
        res[2] = epsilon_;
        res[3] = alpha_;
        res[4] = gamma_;
        res[5] = split_threshold_;
        res[6] = min_sample_;
        res[7] = ks_p_threshold_;
        res[8] = lambda_;
        res[9] = eligibility_threshold_;
        res[10] = tree_size_;
        res[11] = split_nodes_;
        res[12] = state_nodes_;
        //	for(int i=0; i<tree.size(); i++) res[13+i] = tree[i];
        return res;
    }
    virtual std::string getName()
    {
        return "RL Controller";
    }
    //--------------------------------------------------------------------------------------//
private:
    //------------------------------------Private Utils-------------------------------------//
    std::vector<int> decodeAction(int action)
    {
        std::vector<int> res(parameters_.size());
        int prev = 1;
        for (int i = 0; i < parameters_.size(); i++)
        {
            res[i] = (action % action_periods_[i]) / prev;
            prev = action_periods_[i];
        }
        return res;
    }
    TreeNode *getState(const std::vector<float> &obs)
    {
        // Start from root
        TreeNode *state = state_tree_;
        // If it is not a leaf (left and right children should only exist together)
        while (state->left_ != NULL)
        {
            // Check node condition and select appropriate child
            if (obs[state->attribute_] < state->point_)
                state = state->left_;
            else
                state = state->right_;
        }
        return state;
    }
    int selectAction(TreeNode *state)
    {
        // Use e-greedy action selection
        if (random_->nextFloat() < epsilon_)
        {
            // Uniform random selection from all actions
            return random_->nextInt(action_num_);
        }
        else
        {
            // Select the one with the highest Q value
            std::vector<float> q = state->Q_;
            float max = q[0];
            int index = 0;
            for (int i = 0; i < action_num_; i++)
            {
                if (q[i] > max)
                {
                    max = q[i];
                    index = i;
                }
            }
            return index;
        }
    }
    bool splitState(TreeNode *state)
    {
        // Update values of datapoints in the state's archive
        std::vector<Transition> archive = state->transitions_;
        for (int i = 0; i < archive.size(); i++)
        {
            Transition t = archive[i];
            t.value_ = t.reward_ + getState(t.result_)->V_;
        }
        // Find the best split point
        int obs_n = archive[0].input_.size();
        float best = 100;
        //	float best = 0;
        int best_att = 0;
        float best_pnt = 0;
        TransitionComparator comp = TransitionComparator();
        // Calculate sample limit
        int minp = (int)(archive.size() * min_sample_);
        // Try all observables
        for (int att = 0; att < obs_n; att++)
        {
            // Sort archive according to current observable
            comp.setAttribute(att);
            std::sort(archive.begin(), archive.end(), comp);
            // Check all possible split points (maintaining a minimum sample size)
            //	for(int point=minp; point<archive.size()-minp; point++){
            // Check 100 equally spaced split points
            int len = 1;
            if (archive.size() - 2 * minp > 200)
                len = (int)std::round((float)(archive.size() - 2 * minp) / 100.0);
            for (int point = minp; point < archive.size() - minp; point += len)
            {
                // If points' values are too close cannot split between them
                if (std::abs(archive[point - 1].input_[att] - archive[point].input_[att]) < 0.001)
                    continue;
                // Create sample arrays
                std::vector<float> a(point);
                for (int k = 0; k < point; k++)
                    a[k] = archive[k].value_;
                std::vector<float> b(archive.size() - point);
                for (int k = 0; k < archive.size() - point; k++)
                    b[k] = archive[point + k].value_;
                // Do the Kolmogorov-Smirnov test to get the probability that the two samples come from the same distribution
                float D = KolmogorovSmirnoff::normalised_dvalue(a, b);

                // Sum up values
                /*	float sum1 = 0;
				for(int k=0; k<point; k++) sum1 += archive.elementAt(k).value_;
				float sum2 = 0;
				for(int k=0; k<archive.size()-point; k++) sum2 += archive.elementAt(point+k).value_;
				if(std::abs(sum1-sum2)>best){
					best = std::abs(sum1-sum2);
					best_att = att;
					// Place the splitting point between the two values
					best_pnt = (archive.elementAt(point-1).input_[att]+archive.elementAt(point).input_[att])/2;
				}*/
                // Check if p is acceptable (smaller than threshold)
                if (D < KolmogorovSmirnoff::critical_value(ks_p_threshold_))
                {
                    // If so, then consider for a splitting point
                    if (D < best)
                    {
                        best = D;
                        best_att = att;
                        // Place the splitting point between the two values
                        best_pnt = (archive[point - 1].input_[att] + archive[point].input_[att]) / 2;

                        //				System.out.println("Archive size " + archive.size() + " point " + point + " left " + archive.elementAt(point-1).input_[att] + " right " + archive.elementAt(point).input_[att] + " P value " + best + " slpit at " + best_pnt);
                    }
                }
            }
        }
        // If best point found is not good enough don't split
        if (best > KolmogorovSmirnoff::critical_value(ks_p_threshold_))
            return false;
        //	if(best==0) return false;
        // Perform split
        // Create new states
        TreeNode *left = new TreeNode(action_num_, node_serial_++);
        TreeNode *right = new TreeNode(action_num_, node_serial_++);
        // Split archive
        //	System.out.println("Splitting on attribute " + best_att + " at point " + best_pnt);
        //	System.out.println("Parent state with " + archive.size() + " points");
        for (int i = 0; i < archive.size(); i++)
        {
            Transition t = archive[i];
            if (t.input_[best_att] < best_pnt)
                left->transitions_.push_back(t);
            else
                right->transitions_.push_back(t);
        }
        //	System.out.println("Original had " + state->transitions_.size() + " Left has " + left->transitions_.size() + " and right has " + right->transitions_.size() + " (minp=" + minp +")");

        // Initialize values of new states to the values of the original state
        left->V_ = state->V_;
        left->Q_ = std::vector<float>(action_num_);
        for (int i = 0; i < action_num_; i++)
            left->Q_[i] = state->Q_[i];
        right->V_ = state->V_;
        right->Q_ = std::vector<float>(action_num_);
        for (int i = 0; i < action_num_; i++)
            right->Q_[i] = state->Q_[i];
        // Initialize traces of new states to the traces of the original state
        left->trace_ = state->trace_;
        left->action_traces_ = std::vector<float>(action_num_);
        for (int i = 0; i < action_num_; i++)
            left->action_traces_[i] = state->action_traces_[i];
        right->trace_ = state->trace_;
        right->action_traces_ = std::vector<float>(action_num_);
        for (int i = 0; i < action_num_; i++)
            right->action_traces_[i] = state->action_traces_[i];
        // Add new states to the eligibles list if necessary
        if (state->trace_ > eligibility_threshold_)
        {
            eligible_states_.insert(left);
            eligible_states_.insert(right);
        }
        // Make state a split point
        state->attribute_ = best_att;
        state->point_ = best_pnt;
        state->left_ = left;
        state->right_ = right;
        state->transitions_.clear(); // Free memory
        state->Q_.clear();
        state->action_traces_.clear();
        eligible_states_.erase(state); // Remove from eligible states
        // Increase tree and nodes sizes and return success
        tree_size_ += 2;
        split_nodes_++;
        state_nodes_++;
        return true;
    }
    int treeToFloat(TreeNode *root, std::vector<float> res, int pos)
    {
        // Write root
        std::vector<float> me = root->toFloat();
        //System.arraycopy(me, 0, res, pos, me.size());
        res.insert(res.begin() + pos, me.begin(), me.end());
        pos += me.size();
        // If not a leaf, write children
        if (root->left_ != NULL)
        {
            pos = treeToFloat(root->left_, res, pos);
            pos = treeToFloat(root->right_, res, pos);
        }
        return pos;
    }
    /*void splitTree(TreeNode root)
	{
		// If this is a leaf state node try to split if its archive is big enough
		if(root.left_==NULL){
			if(root.transitions_.size()>split_threshold_) splitState(root);
		// Otherwise it is an internal node, recur for both children
		}else{
			splitTree(root.left_);
			splitTree(root.right_);
		}
	}*/
    //--------------------------------------------------------------------------------------//
};