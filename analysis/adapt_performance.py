




import argparse
from filenames import *
from plots import *

parser = argparse.ArgumentParser(description='Process destination folder.')
parser.add_argument('-d', dest='DEST', type=str)

args = parser.parse_args()



def get_performances(mins,means,maxs, condition,test_type,replicates):
    mes = []
    ms = []
    Ms = []
    for replicate in replicates:
        filename = get_file_name_test(args.DEST,condition,test_type,replicate)
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

def get_performances_pop(mins,means,maxs, n_pop, condition, test_type, replicates):
    mes = []
    ms = []
    Ms = []
    for replicate in replicates:
        filename = get_file_name_test(args.DEST, condition, test_type, replicate)
        print("will get performance at " + filename)
        x = [[] for i in range(n_pop)]
        p=0
        temp=[]
        with open(filename, 'r') as f:
            for line in f:
                if line:  # avoid blank lines
                    stripped = line.strip()
                    if stripped == "END META-INDIVIDUAL":
                        x[p]=temp
                        temp=[]
                        p+=1
                    else:
                        temp.append(float(stripped))
        #assert len(x) == n_pop
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

if __name__ == "__main__":
    conditions=["meta", "bo", "duty" ,"lv" ,"random"]
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
                    p = get_performances_pop(mins[j],means[j],maxs[j],n_pop,c, t, replicates)
                else:
                    p=get_performances(mins[j],means[j],maxs[j],c,t,replicates)
    with open("adaptation_table", "w") as f:
        make_table(f, (means,),
                   rowlabels=["damage","environment"],
                   columnlabels=labels,
                   conditionalcolumnlabels=[("Mean", "float2")],
                   transpose=True)