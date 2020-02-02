




import argparse
from filenames import *
from plots import *

parser = argparse.ArgumentParser(description='Process destination folder.')
parser.add_argument('-d', dest='DEST', type=str)

args = parser.parse_args()


def get_performances_single(mins,means,maxs, condition,test_type,replicates, type):
    mes = []
    ms = []
    Ms = []
    for replicate in replicates:
        if type=="test":
            filename = get_file_name_test(args.DEST,condition,test_type,replicate)
        else:
            filename = get_file_name_train(args.DEST, condition, test_type, replicate)
        print("will get performance at "+ filename)
        x=[]
        with open(filename, 'r') as f:
            for line in f:
                if line: #avoid blank lines
                    x.append(float(line.strip()))
        if x:
            print(condition + " " + test_type + " " + replicate + ":")
            # print(p)
            mes.append(np.mean(x))
            print("mean=" + str(means[-1]))
            Ms.append(np.max(x))
            print("max=" + str(maxs[-1]))#
            ms.append(np.min(x))
            print("min=" + str(mins[-1]))
        else:
            raise Exception("empty performance list")
    mins.append(ms)
    maxs.append(Ms)
    means.append(mes)


def get_adaptperformance_from_file(filename,n_pop):
    x = [[] for i in range(n_pop)]
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
                        temp_w=np.array(stripped.split(),dtype=float)
                else:  # collecting performance data
                    if stripped == "END TEST META-INDIVIDUAL":
                        x[p] = temp
                        temp = []
                        p += 1
                    else:
                        temp.append(float(stripped))
    return weights, x


def get_performances_pop(mins,means,maxs, n_pop, condition, test_type, replicates,selection_criterion, type):
    mes = []
    ms = []
    Ms = []

    if type!="test": assert(selection_criterion is None)

    for replicate in replicates:

        if type=="test":
            # perform selection criterion from data easily obtained in simulation
            # if selection_criterion=="evolution_performance":    # not reliable, as database has been updated and only 10% of solutions in map are evaluated
            #     f_name = get_file_name_metafitness(args.DEST, condition, test_type, replicate, generation)
            #     print("will get max index from "+str(f_name))
            #     with open(f_name,"r") as f_fit:
            #         for line in f_fit:
            #             result = [x.strip() for x in line.split('\t')]
            #         max_index = np.argmax(result[:-1]) # ignore the empty character due to newline
            if selection_criterion=="train_performance":     # reliable choice as it takes into account database changes and 100% of solutions in map
                f = get_file_name_train(args.DEST, condition, test_type, replicate)
                print("will get max index from "+str(f))
                w, perf = get_adaptperformance_from_file(f, n_pop)
                perf = [np.mean(perf[i]) for i in range(len(perf))]
                max_index = np.argmax(perf)

            filename = get_file_name_test(args.DEST, condition, test_type, replicate)
        else:
            filename = get_file_name_train(args.DEST, condition, test_type, replicate)
        print("will get performance at " + filename)
        w, x = get_adaptperformance_from_file(filename, n_pop)
        #assert len(x) == n_pop
        if selection_criterion is not None:
            print("will select based on train performance")
            print(condition + " " + test_type + " " + replicate + ":")
            # print(p)
            mes.append(np.mean(x[max_index]))
            print("mean=" + str(mes[-1]))
            Ms.append(np.max(x[max_index]))
            print("max=" + str(Ms[-1]))  #
            ms.append(np.min(x[max_index]))
            print("min=" + str(ms[-1]))
        else:
            for i in range(n_pop):
                if x[i]:
                    print(condition + " " + test_type + " " + replicate + ":")
                    print("meta-individual "+str(i))
                    # print(p)
                    mes.append(np.mean(x[i]))
                    print("mean=" + str(mes[-1]))
                    Ms.append(np.max(x[i]))
                    print("max=" + str(Ms[-1]))  #
                    ms.append(np.min(x[i]))
                    print("min=" + str(ms[-1]))
                # else:
                #     raise Exception("empty performance list")


    mins.append(ms)
    maxs.append(Ms)
    means.append(mes)

def get_performances(type,selection_criterion):
    conditions=["meta","bo", "duty" ,"lv" ,"random"]
    labels=["Meta","Body orientation","Duty factor", "Linear velocity", "Random weight"]
    test_types=["damage","envir",]
    n_pop=5

    mins=[[] for i in test_types]
    maxs=[[] for i in test_types]
    means=[[] for i in test_types]
    replicates=["1","2","3"]

    for j, t in enumerate(test_types):
            for c in conditions:
                if c=="meta":
                    get_performances_pop(mins[j],means[j],maxs[j],n_pop,c, t, replicates,selection_criterion, type )
                else:
                    # mins,means,maxs, condition,test_type,replicates, type
                    get_performances_single(mins[j],means[j],maxs[j],c,t,replicates, type)
    with open(type+"_performances.txt", "w") as f:
        make_table(f, (means,),
                   rowlabels=["damage","environment"],
                   columnlabels=labels,
                   conditionalcolumnlabels=[("Mean", "float2")],
                   transpose=True)


if __name__ == "__main__":
    #get_performances(type="train",selection_criterion=None)
    get_performances(type="test",selection_criterion="train_performance")