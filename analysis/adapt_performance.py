




import argparse

from filenames import *
from plots import *

parser = argparse.ArgumentParser(description='Process destination folder.')
parser.add_argument('-d', dest='DEST', type=str)

args = parser.parse_args()
conditions = ["damage_meta", "envir_meta", "bo", "duty", "lv", "random"]
replicates=["1","2","3","david44","david55"]
types=["train","test"]
test_types = ["damage", "envir", ]
labels = ["Damage-meta", "Envir-meta", "Body-orientation", "Duty-factor", "Linear-velocity", "Random-weight"]

def check_files():


    for condition in conditions:
        if condition=="damage_meta":
            time=260
        elif condition=="envir_meta":
            time=270
        else:
            time=7000
        for replicate in replicates:
            counts=[]
            archive_counts=[]
            for type in types:

                for test_type in test_types:
                    count = 0
                    archive_count=0
                    if type == "test":
                        filename = get_file_name_test(args.DEST, condition, test_type, replicate)
                    else:
                        filename = get_file_name_train(args.DEST, condition, test_type, replicate)
                    with open(filename, 'r') as f:
                        for line in f:
                            count+=1
                    counts.append(count)
                    if not condition.endswith("meta"):
                        filename, folder = get_archive_plus_dir(args.DEST, condition, time, index=None)
                        with open(folder+str(replicate)+"/"+filename, 'r') as f:
                            for line in f:
                                    archive_count += 1
                    else:
                        for index in range(5):
                            filename, folder = get_archive_plus_dir(args.DEST, condition, time, index=index)
                            with open(folder+str(replicate)+"/"+filename, 'r') as f:
                                for line in f:
                                    archive_count += 1
                    archive_counts.append(archive_count)
            for i in range(1,len(counts)):
                if counts[0]!=counts[i]:
                    print(condition+replicate)
                    print("train_damage","train_envir","test_damage","test_envir")
                    print(counts)
                    print(archive_counts)
                    break


def get_performances_single(mins,means,maxs, condition,test_type,replicates, type):
    mes = []
    ms = []
    Ms = []
    lengths=[]
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
        lengths.append(len(x))
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

    #if type!="test": assert(selection_criterion is None)

    for replicate in replicates:
        if selection_criterion=="train_performance":     # reliable choice as it takes into account database changes and 100% of solutions in map
            f = get_file_name_train(args.DEST, condition, test_type, replicate)
            print("will get max index from "+str(f))
            w, perf = get_adaptperformance_from_file(f, n_pop)
            perf = [np.mean(perf[i]) for i in range(len(perf))]
            max_index = np.argmax(perf)
        if type=="test":
            # perform selection criterion from data easily obtained in simulation
            # if selection_criterion=="evolution_performance":    # not reliable, as database has been updated and only 10% of solutions in map are evaluated
            #     f_name = get_file_name_metafitness(args.DEST, condition, test_type, replicate, generation)
            #     print("will get max index from "+str(f_name))
            #     with open(f_name,"r") as f_fit:
            #         for line in f_fit:
            #             result = [x.strip() for x in line.split('\t')]
            #         max_index = np.argmax(result[:-1]) # ignore the empty character due to newline


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


    n_pop=5

    mins=[[] for i in test_types]
    maxs=[[] for i in test_types]
    means=[[] for i in test_types]


    for j, t in enumerate(test_types):
            for c in conditions:
                if c.endswith("meta"):
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
    make_significance_table(means[0],0)   # damage -> comp to damage
    make_significance_table(means[0],1)  # damage -> comp to damage
    make_significance_table(means[1],0)  # envir --> comp to envir
    make_significance_table(means[1],1)   # envir --> comp to envir

def make_significance_table(means,comp_index,table_type="resilience"):

        for j, condition in enumerate(labels):
            if comp_index == j:
                continue
            x=means[comp_index]
            y=means[j]
            U, p = ranksums(x, y)
            p_value = "p=%.3f" % (p) if p > 0.001 else r"p<0.001"
            delta,label = cliffs_delta(x,y)
            delta_value = r"\mathbf{%.2f}"%(delta) if label == "large" else r"%.2f"%(delta)
            print( labels[comp_index] + "vs" + condition)
            print(p_value,delta_value)

def cliffs_delta(x,y):
    """
    meaning: proportion x>y minus proportion y>x
    |d|<0.147 "negligible", |d|<0.33 "small", |d|<0.474 "medium", otherwise "large"
    here calculate based on relation with the rank-sum test
    :param U: the result of the Wilcoxon rank-test/Mann-Withney U-test
    :return:
    """
    m=len(x)
    n=len(y)

    # delta =  2.0*U/float(m*n) - 1.0
    if len(x) > 2000 or len(y)>2000:   # avoid memory issues
        print("starting with lengths %d %d "%(m,n))
        print("digitising samples")
        xspace=np.linspace(x.min(),x.max(),500)
        yspace = np.linspace(x.min(), x.max(), 500)
        freq_x=np.histogram(x, bins=xspace)[0]
        freq_y=np.histogram(y, bins=yspace)[0]

        count=0
        for i in range(len(freq_x)):
            for j in range(len(freq_y)):
                num_combos=freq_x[i]*freq_y[j]
                xx=xspace[i]
                yy=yspace[j]
                if xx > yy:
                    count+=num_combos
                else:
                    count-=num_combos
        count/=float(m*n)
    else:
        z=np.array([xx - yy for xx in x for yy in y]) # consider all pairs of data
        count=float(sum(z>0) - sum(z<0))/float(m*n)
    # assert count==delta, "delta:%.3f  count:%.3f"%(delta,count)
    label = None
    magn=abs(count)
    if magn < 0.11:
        label="negligible"
    elif magn < 0.28:
        label="small"
    elif magn < 0.43:
        label="medium"
    else:
        label="large"
    return count, label
if __name__ == "__main__":
    check_files()
    #get_performances(type="train",selection_criterion=None)
    get_performances(type="test",selection_criterion=None)