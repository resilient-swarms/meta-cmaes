
from adapt_performance import *


conditions =  ["cmaescheck"] + conditions
labels = ["Single-objective EA"] + labels


def get_damagemaxperformance_from_file_cmaescheck(fileprefix, function_evaluations, slicing_factor):
    num_damages = 12
    max_value = -float("infinity")
    temp = [[] for i in range(num_damages)]
    num_lines=function_evaluations/float(slicing_factor)
    for damage in range(num_damages):
        filename = fileprefix+"/damage"+str(damage)+"/bestfit.dat"
        print("will get performance at " + filename)
        with open(filename, 'r') as f:
            for line in f:
                if line:  # avoid blank lines
                    stripped = line.strip().split(' ')[-1] # get the second value, the best value of the current generation
                    current_value=float(stripped)
                    if current_value > max_value:
                        max_value = current_value
                    temp[damage].append(max_value)
                    if len(temp[damage]) == num_lines:
                        break
    return temp


def get_damagemaxperformance_from_file_pop(filename,n_pop,function_evaluations, slicingfactor, strip_weights=False):
    print("will get performance at " + filename)
    num_damages=12
    x = [[[] for j in range(num_damages)] for i in range(n_pop)]
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
                if strip_weights and len(weights) < n_pop:  # collecting weights
                    if stripped == "END WEIGHTS":
                        weights.append(temp_w)
                        temp_w = []
                    else:
                        temp_w.append(np.array(stripped.split(),dtype=float))
                else:  # collecting performance data
                    if stripped.startswith("DAMAGE"): # start new damage



                        print("map index is now " + str(map_index))
                        print("damage index is now " + str(damage_index+1))
                        if damage_index >= 0:
                            x[map_index][damage_index]=temp[slicingfactor:function_evaluations+slicingfactor:slicingfactor]
                            temp=[]

                        if damage_index == num_damages - 1:
                            # finish this list
                            damage_index=-1
                            map_index+=1
                            continue


                        damage_index += 1

                    else:

                        temp.append(float(stripped))

        x[map_index-1][num_damages-1] = temp[slicingfactor:function_evaluations+slicingfactor:slicingfactor] # finish the last one as well
    return weights, x


def get_damageperformances_pop(mean_lines,sd_lines1, sd_lines2, n_pop, condition, test_type, replicates,selection_criterion,strip_weights,function_evaluations=80,slicing_factor=5):


    max_index=0
    num_damages=12
    #if type!="test": assert(selection_criterion is None)
    lines=np.zeros((len(replicates),num_damages,int(function_evaluations/float(slicing_factor))))
    for i, replicate in enumerate(replicates):
        if n_pop > 1:
            if selection_criterion=="train_performance":     # reliable choice as it takes into account database changes and 100% of solutions in map
                f = get_file_name_train(args.DEST, condition, test_type, replicate)
                print("will get max index from "+str(f))
                w, perf = get_damagemaxperformance_from_file_pop(f, n_pop, function_evaluations, slicing_factor, strip_weights)
                m=np.mean(lines,axis=(0,1))
                max_index = np.argmax(m[-1])
        if condition == "cmaescheck":

            x = get_damagemaxperformance_from_file_cmaescheck(args.DEST+"/cmaescheck/exp"+replicate, function_evaluations, slicing_factor=slicing_factor)
        else:

            filename = get_file_name_test(args.DEST, condition, test_type, replicate)
            _w, x = get_damagemaxperformance_from_file_pop(filename, n_pop, function_evaluations, slicing_factor,strip_weights)
        #assert len(x) == n_pop
        lines[i] = x[max_index]

    # all done, now aggregate and add to lines to plot
    m=np.mean(lines,axis=(0,1))
    mean_lines.append(m)
    means=np.mean(lines,axis=1)
    sd=np.std(means,axis=0) # SD over replicates
    sd_lines1.append(m-sd)
    sd_lines2.append(m+sd)


def get_performances(type,selection_criterion):

    mean_lines=[]
    sd_lines1=[]
    sd_lines2=[]
    for j, t in enumerate(test_types):
            for c in conditions:
                    if c.endswith("meta"):
                        n_pop = 5
                        strip_weights=True
                    else:
                        n_pop = 1
                        strip_weights=False
                    get_damageperformances_pop(mean_lines,sd_lines1,sd_lines2, n_pop, c, t, replicates,selection_criterion, strip_weights=strip_weights)

    colors = ["C5","C0", "C1", "C2", "C3", "C4"]  # colors for the lines
    # (numsides, style, angle)
    markers = ["*","o", "^", "s", "X", "D"]  # markers for the lines
    y_labels = ["Map coverage"]

    time=np.array(range(5,85,5))
    createPlot(mean_lines, x_values=time,
                   save_filename="damagetest.pdf", legend_labels=labels,
                   colors=colors, markers=markers, xlabel="Function evaluations", ylabel="Best test-performance ($m$)",
                   xlim=[0, time[-1] + 5], xscale="linear", yscale="linear", ylim=None,
                   legendbox=None, annotations=[], xticks=[], yticks=[], task_markers=[], scatter=False,
                   legend_cols=1, legend_fontsize=26, legend_indexes=[], additional_lines=[], index_x=[],
                   xaxis_style="plain", y_err=[], force=True, fill_between=(sd_lines1, sd_lines2))



if __name__ == "__main__":
    #get_performances(type="train",selection_criterion="train_performance")
    get_performances(type="test",selection_criterion="train_performance")