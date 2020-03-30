
from adapt_performance import *


conditions =  "cmaescheck" + conditions
labels = "Single-objective EA" + labels


def get_damagemaxperformance_from_file_cmaescheck(fileprefix):
    num_damages = 12
    max_value = -float("infinity")
    temp = []
    for damage in range(num_damages):
        filename = fileprefix+"/damage"+str(damage)+"/best_fit.dat"

        with open(filename, 'r') as f:
            for line in f:
                if line:  # avoid blank lines
                    stripped = line.strip().split('\t')[-1] # get the second value, the best value of the current generation
                    current_value=float(stripped)
                    if current_value > max_value:
                        max_value = current_value
                    temp.append(max_value)

    return temp


def get_damagemaxperformance_from_file_pop(filename,n_pop):
    num_damages=12
    x = [[] for i in range(n_pop)]
    map_index=0
    damage_index=-1
    p = 0
    temp = []
    weights = []
    temp_w = []
    with open(filename, 'r') as f:
        for line in f:
            if line:  # avoid blank lines
                stripped = (line.strip())
                if len(weights) < n_pop:  # collecting weights
                    if stripped == "END WEIGHTS":
                        weights.append(temp_w)
                        temp_w = []
                    else:
                        temp_w.append(np.array(stripped.split(),dtype=float))
                else:  # collecting performance data
                    if stripped.startswith("DAMAGE"): # start new damage
                        damage_index+=1
                        if damage_index >= 0 :
                            x[map_index]=temp
                            temp=[]
                        if damage_index == num_damages-1:
                            damage_index=0
                            map_index+=1

                    else:
                        temp.append(float(stripped))
    return weights, x


def get_damageperformances_pop(mins,means,maxs, n_pop, condition, test_type, replicates,selection_criterion):
    mes = []
    ms = []
    Ms = []

    #if type!="test": assert(selection_criterion is None)

    for replicate in replicates:
        if n_pop > 1:
            if selection_criterion=="train_performance":     # reliable choice as it takes into account database changes and 100% of solutions in map
                f = get_file_name_train(args.DEST, condition, test_type, replicate)
                print("will get max index from "+str(f))
                w, perf = get_damagemaxperformance_from_file_pop(f, n_pop)
                perf = [np.mean(perf[i]) for i in range(len(perf))]
                max_index = np.argmax(perf)

        filename = get_file_name_test(args.DEST, condition, test_type, replicate)

        print("will get performance at " + filename)
        if condition == "cmaescheck":
            w, x = get_damagemaxperformance_from_file_cmaescheck(filename,n_pop)
        else:
            w, x = get_damagemaxperformance_from_file_pop(filename, n_pop)
        #assert len(x) == n_pop

        print("will select based on train performance")
        print(condition + " " + test_type + " " + replicate + ":")
        # print(p)
        mes.append(np.mean(x[max_index]))
        print("mean=" + str(mes[-1]))
        Ms.append(np.max(x[max_index]))
        print("max=" + str(Ms[-1]))  #
        ms.append(np.min(x[max_index]))
        print("min=" + str(ms[-1]))

                # else:
                #     raise Exception("empty performance list")


    mins.append(ms)
    maxs.append(Ms)
    means.append(mes)

def get_performances(type,selection_criterion):






    mins=[[] for i in test_types]
    maxs=[[] for i in test_types]
    means=[[] for i in test_types]


    for j, t in enumerate(test_types):
            for c in conditions:
                    if c.endswith("meta"):
                        n_pop = 5
                    else:
                        n_pop = 1
                    get_damageperformances_pop(mins,means,maxs, n_pop, c, t, replicates,selection_criterion)
    with open(type+"_performances.txt", "w") as f:
        make_table(f, (means,),
                   rowlabels=["damage"],
                   columnlabels=labels,
                   conditionalcolumnlabels=[("Mean", "float2")],
                   transpose=True)
    print(type)
    print()
    print("damage")
    make_significance_table(means[0],0)   # damage -> comp to damage
    make_significance_table(means[0],4)  # damage -> comp to damage













if __name__ == "__main__":
    get_performances(type="train",selection_criterion="train_performance")
    get_performances(type="test",selection_criterion="train_performance")