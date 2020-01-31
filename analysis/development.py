
# behavioural metrics defined in Mouret, J., & Clune, J. (2015). Illuminating search spaces by mapping elites. 1–15.
import os
import operator

import argparse

from process_archive import *
from filenames import *

from plots import *
import copy


PRINT=False



parser = argparse.ArgumentParser(description='Process destination folder.')
parser.add_argument('-d', dest='DEST', type=str)

args = parser.parse_args()


def print_conditional(some_string):
    if PRINT:
        print(some_string)

def get_all_performances(bd_dims, path,conversion_func=None,from_fitfile=False):
    if conversion_func is not None:
        all_performances = [conversion_func(fitness) for fitness in get_bin_performances_uniquearchive(path,bd_dims,from_fitfile).values()]
    else:
        all_performances = [fitness for fitness in get_bin_performances_uniquearchive(path,bd_dims,from_fitfile).values()]
    return all_performances

def global_performances(bd_dims, BD_directory,  runs, archive_file_path, max_performance,conversion_func,from_fitfile=False):
    stats = []
    for run in runs:
        stats.append(_global_performance(bd_dims,  BD_directory, run, archive_file_path, max_performance,conversion_func,from_fitfile))

    print_conditional("global performances: " + str(stats))
    return stats

def _global_performance( bd_dims, BD_directory, run,archive_file_path,max_performance,conversion_func=None,from_fitfile=False):
    """
    For each run, the single highest- performing
    solution found by that algorithm anywhere in the search space
    divided by the highest performance possi- ble in that domain.
    If it is not known what the maximum theoretical performance is,
    as is the case for all of our do- mains,
    it can be estimated by dividing by the highest performance found by any algorithm in any run. ( BUT avoid estimation if you can)
    This measure is the traditional, most common way to evaluate optimization algorithms.
    This measure is the traditional, most common way to evaluate optimization algorithms.
    One can also measure whether any illumination algorithm also performs well on this measurement.
    Both the ideal optimization algorithm and the ideal illumination
    algorithm are expected to perform perfectly on this measure

    :param BD_directory: directory in which all the runs of a BD are located,
            e.g. ~/Desktop/history_obstacles
    :param experiment_file_path: relative path from the BD_directory to the archive file
    """
    path=get_archive_filepath(BD_directory, run, archive_file_path)
    all_performances = get_all_performances(bd_dims, path,  conversion_func,from_fitfile)
    return max(all_performances)/max_performance


def avg_performances(bd_dims, BD_directory,  runs, archive_file_path, max_performance,conversion_func,from_fitfile):
    stats = []
    for run in runs:
        stats.append(_avg_performance(bd_dims, BD_directory,  run, archive_file_path, max_performance,conversion_func,from_fitfile))
    print_conditional("avg performances: " + str(stats))
    return stats

def _avg_performance(bd_dims, BD_directory,  run,archive_file_path,max_performance,conversion_func=None,from_fitfile=False):
    """
    For each run, the single highest- performing
    solution found by that algorithm anywhere in the search space
    divided by the highest performance possi- ble in that domain.
    If it is not known what the maximum theoretical performance is,
    as is the case for all of our do- mains,
    it can be estimated by dividing by the highest performance found by any algorithm in any run. ( BUT avoid estimation if you can)
    This measure is the traditional, most common way to evaluate optimization algorithms.
    This measure is the traditional, most common way to evaluate optimization algorithms.
    One can also measure whether any illumination algorithm also performs well on this measurement.
    Both the ideal optimization algorithm and the ideal illumination
    algorithm are expected to perform perfectly on this measure

    :param BD_directory: directory in which all the runs of a BD are located,
            e.g. ~/Desktop/history_obstacles
    :param experiment_file_path: relative path from the BD_directory to the archive file
    """
    path=get_archive_filepath(BD_directory,run, archive_file_path)
    all_performances=get_all_performances(bd_dims, path, conversion_func,from_fitfile)
    return np.mean(all_performances)/max_performance

def global_reliabilities(bd_dims,BD_directory, runs,archive_file_path,by_bin):
    """
    averages the global reliability across the different maps in the combined archive
    :param BD_directory:
    :param runs:
    :param archive_file_path:
    :return:
    """

    combined_archive=get_combined_archive(BD_directory, bd_dims,  runs, archive_file_path,by_bin=by_bin)
    stats = []
    for run in runs:
        stats.append(_global_reliability(bd_dims, combined_archive, BD_directory, run, archive_file_path,by_bin))
    print_conditional("global reliabilities: "+str(stats))
    return stats

def _global_reliability(bd_dims, combined_archive,BD_directory, run, archive_file_path,by_bin):

    """
    For each run, the average across all cells of the highest-performing solution the algorithm found for each cell
    (0 if it did not produce a solution in that cell)
    divided by the best known performance for that cell as found by any run of any algorithm.
    Cells for which no solution was found by any run of any algorithm are not included in the calculation
    (to avoid dividing by zero, and because it may not be possible to fill such cells and algorithms
     thus should not be penalized for not doing so).

     NOTE: because here we are comparing descriptors with the same algorithm,
     and different descriptors have different meaning for the cells,
     the best performance and filled cells are computed by the different runs of a single setting

     i.e., ignore cells that have not been filled in any of the runs,
     but for cells that are filled compute a map's performance/max(performance) and average it

    :param BD_directory: directory in which all the runs of a BD are located,
            e.g. ~/Desktop/history_obstacles
    :params runs, the number of runs
    :param archive_file_path, the relative path from the BD_directory to the archive file
    :return:
    """
    path = get_archive_filepath(BD_directory, run, archive_file_path)
    if by_bin == "individual":
        all_non_empty_performances = get_ind_performances_uniquearchive(path, bd_dims)

    else:
        all_non_empty_performances = get_bin_performances_uniquearchive(path, bd_dims, as_string=True)

    cell_performances = []

    for bin , archive_perfs in combined_archive.items():  # take the mean across the cells that are not empty across runs
        performance = all_non_empty_performances.get(bin, None)
        if performance is not None:
            max_performance = max(archive_perfs)
            assert performance <= max_performance
            if max_performance > 0:
                cell_performances.append(performance / max_performance)
            else:
                cell_performances.append(0.0)
        else:
            cell_performances.append(0.0)

    mean = np.mean(cell_performances)
    return mean

def precisions(bd_dims, BD_directory,runs,archive_file_path,by_bin):
    """
    averages the precision of the different maps in the combined archive
    :param BD_directory:
    :param runs:
    :param archive_file_path:
    :return:
    """
    combined_archive=get_combined_archive(BD_directory, runs, archive_file_path,by_bin=by_bin)
    stats = []
    for run in runs:
        stats.append(_precision(bd_dims,combined_archive,BD_directory, run, archive_file_path))
    print_conditional("precisions"+str(stats))
    return stats
def _precision(bd_dims,  combined_archive, BD_directory, run, archive_file_path):

    """
    Same as global reliability, but for each run, the normalized performance
    is averaged only for the cells that were filled by that algo- rithm **in that run**

    i.e., ignore cells not filled in the current run,
    but for cells that are filled compute a map's performance/max(performance) and average it

    :param BD_directory: directory in which all the runs of a BD are located,
            e.g. ~/Desktop/history_obstacles
    :params runs, the number of runs
    :param archive_file_path, the relative path from the BD_directory to the archive file
    :return:
    """
    path = get_archive_filepath(BD_directory, run, archive_file_path)
    all_non_empty_performances = get_bin_performances_uniquearchive(path,bd_dims)

    cell_performances = []

    for bin , archive_perf in all_non_empty_performances.items():  # take the mean across the cells that are not empty *in this run*
        performance = archive_perf
        cp=combined_archive[bin]
        max_performance = max(cp)
        assert performance <= max_performance
        cell_performances.append(performance/max_performance)
    mean = np.mean(cell_performances)
    return mean
def coverages(bd_shape,BD_directory, runs, archive_file_path):
    stats = []
    for run in runs:
        stats.append(_coverage(bd_shape,BD_directory, run, archive_file_path))
    print_conditional("global coverages"+str(stats))
    return stats

def _coverage(bd_shape,BD_directory, run, archive_file_path):
    """
    Measures how many cells of the feature space a run of an algorithm 
    is able to fill of the total number that are possible to fill.
    :return: 
    """

    path = get_archive_filepath(BD_directory, run, archive_file_path)
    all_non_empty_performances = get_bin_performances_uniquearchive(path,len(bd_shape))
    num_filled=len(all_non_empty_performances)
    max_num_filled=get_bins(bd_shape)
    return float(num_filled)/float(max_num_filled)

def absolutecoverages(bd_shape,BD_directory, runs, archive_file_path):
    stats = []
    for run in runs:
        stats.append(_absolutecoverage(bd_shape,BD_directory, run, archive_file_path))
    print_conditional("global coverages"+str(stats))
    return stats

def _absolutecoverage(bd_shape,BD_directory, run, archive_file_path):
    """
    Measures how many cells of the feature space a run of an algorithm
    is able to fill
    :return:
    """

    path = get_archive_filepath(BD_directory, run, archive_file_path)
    all_non_empty_performances = get_bin_performances_uniquearchive(path,len(bd_shape))
    num_filled=len(all_non_empty_performances)
    return float(num_filled)

# def _spread(bd_shape,BD_directory, run, archive_file_path,
#             individuals=[],distance_metric=norm_Euclidian_dist,
#             bd_start=1,comp=[]):
#     """
#     Measures how many cells of the feature space a run of an algorithm
#     is able to fill
#     :return:
#     """

#     path = get_archive_filepath(BD_directory, run, archive_file_path)
#     bd_list  = get_individual_bds(path,individuals,bd_start)
#     temp=0.0
#     comps=0.0




#     if not comp:
#         for i,bd1 in enumerate(bd_list):
#             for j,bd2 in enumerate(bd_list):
#                 if i!=j:
#                     temp+=distance_metric(bd1,bd2)
#                     comps+=1.0
#         temp/=comps
#         assert comps==len(bd_list)*(len(bd_list)-1)
#         return temp
#     else:
#         comp =  get_individual_bds(path,comp,bd_start)[0]


#         for i,bd1 in enumerate(bd_list):
#             temp+=distance_metric(bd1,comp)  # no need to check for equality since if comp==bd1 this should be penalised
#             comps+=1.0
#         temp/=comps
#         assert comps==len(bd_list)
#         return temp

# def uniqueness(BD_directory,runs, gener,targets, bin_indexes):
#     """
#     calculates uniqueness values, for combined archive and single archive
#     :param BD_directory:
#     :param runs:
#     :param archive_file_path:
#     :param bin_indexes
#     :return:
#     """
#     ub_dict={}
#     comb_ub_dict={}
#     for target in targets:
#         comb_unique_bins=set([])
#         unique_bins=[]
#         for run in runs:

#             u=set(parse_bins(BD_directory+"/results"+str(run)+"/analysis"+str(gener)+"_"+target+"REDUCED.dat",bin_indexes[target]))
#             unique_bins.append(u)
#             comb_unique_bins = comb_unique_bins | u
#         ub_dict[target]=unique_bins
#         comb_ub_dict[target]=comb_unique_bins

#     return ub_dict,comb_ub_dict


def globalcoverage(bd_dims, BD_directory, runs,archive_file_path,by_bin):
    """
    averages the precision of the different maps in the combined archive
    :param BD_directory:
    :param runs:
    :param archive_file_path:
    :return:
    """
    combined_archive=get_combined_archive(BD_directory, bd_dims,  runs, archive_file_path,by_bin=by_bin)
    num_filled = len(combined_archive)
    return float(num_filled)



def add_boxplotlike_data(stats, y_bottom,y_mid,y_top, y_label,method_index,statistic="mean_SD"):
    """
    return x.25,x.50 and x.75
    :return:
    """
    if statistic=="median_IQR":
        x1,x2,x3=tuple(np.quantile(stats, q=[.25, .50, .75]))
    elif statistic=="mean_SD":
        sd = np.std(stats)
        x2 = np.mean(stats)
        x1 = x2 - sd
        x3 = x2 + sd
    elif statistic=="meanall_replicatesd": # when joining different fitfuns

        x2=np.mean(np.array(stats))
        sds=[np.std(stats[i]) for i in range(len(stats))]
        sd=np.mean(sds)
        x1= x2 - sd
        x3 = x2 + sd
        # assumes fitfun is first dimension of stats

    else:
        raise Exception("statistic %s not known"%(statistic))

    y_bottom[y_label][method_index].append(x1)
    y_mid[y_label][method_index].append(x2)
    y_top[y_label][method_index].append(x3)
# def translated_coverages(t,BD_dir,runs, targets):
#     d={target:[] for target in targets}
#     for run in runs:
#         for target, shape in targets.items():
#             archive_file = "analysis" + str(t) + "_" + target + "REDUCED.dat"
#             cov = _absolutecoverage(shape, BD_dir, run, archive_file)
#             d[target].append(cov)
#     print_conditional("translated coverages " + str(d))
#     return d

# def translated_spreads(t,BD_dir,runs,targets,bd_start,dists,individuals,comp):
#     d = {target: [] for target in targets}
#     for run in runs:
#         for target, shape in targets.items():
#             if individuals[run-1]:  # look for the unreduced archive
#                 archive_file = "analysis" + str(t) + "_" + target + ".dat"
#             else: # look for the reduced archive individuals
#                 archive_file = "analysis" + str(t) + "_" + target + "REDUCED.dat"
#             s = _spread(shape, BD_dir, run, archive_file,bd_start=bd_start.get(target,1),
#                         distance_metric=dists[target],
#                         individuals=individuals[run-1],
#                         comp=comp[run-1])
#             d[target].append(s)
#     print_conditional("translated spreads " + str(d))
#     return d

# def add_boxplotlike_data(stats, y_bottom,y_mid,y_top, y_label,method_index,statistic="mean_SD"):
#     """
#     return x.25,x.50 and x.75
#     :return:
#     """
#     if statistic=="median_IQR":
#         x1,x2,x3=tuple(np.quantile(stats, q=[.25, .50, .75]))
#     elif statistic=="mean_SD":
#         sd = np.std(stats)
#         x2 = np.mean(stats)
#         x1 = x2 - sd
#         x3 = x2 + sd
#     elif statistic=="meanall_replicatesd": # when joining different fitfuns

#         x2=np.mean(np.array(stats))
#         sds=[np.std(stats[i]) for i in range(len(stats))]
#         sd=np.mean(sds)
#         x1= x2 - sd
#         x3 = x2 + sd
#         # assumes fitfun is first dimension of stats

#     else:
#         raise Exception("statistic %s not known"%(statistic))

#     y_bottom[y_label][method_index].append(x1)
#     y_mid[y_label][method_index].append(x2)
#     y_top[y_label][method_index].append(x3)

# def convert_CoverageFitness(fitness,grid_size=0.1212,max_velocity=0.10, time_per_trial=120, total_cells=1090):
#     """
#     :param fitness: the coverage fitness e.g. 0.10 means 10% of all cells are visited
#     :param grid_size: e.g. each cell 0.14 m
#     :param max_velocity: e.g. maximal velocity of thymio is 0.10m/s
#     :param time_per_trial: e.g. 120 seconds
#     :param arena_size: e.g. (4,4) is a 4-by-4 arena
#     :return:
#     """
#     max_cells_per_second = max_velocity/grid_size
#     max_cells_per_trial = np.ceil(max_cells_per_second*time_per_trial)

#     visited_cells = np.ceil(total_cells*fitness)  #e.g. 160 cells, then fitness=0.1  means 16 cells visited
#     return visited_cells/max_cells_per_trial  # coverage now means visited cells compared to the maximum possible

# def print_best_individuals(BD_dir,outfile, number,generation):
#     solutions, indexes = get_best_individuals(BD_dir, range(1,6), "archive_"+str(generation)+".dat",number,criterion="fitness")
#     with open(outfile+"fitness.txt", 'w') as f:
#         i=0
#         for key,value in solutions.items():
#             f.write("%s %s %.3f \n "%(indexes[i], str(key),value[0]))
#             i+=1
#         f.flush()



#     solutions, indexes = get_best_individuals(BD_dir, range(1, 6), "archive_"+str(generation)+".dat", number, criterion="diversity")
#     with open(outfile + "diversity.txt", 'w') as f:
#         i = 0
#         for array in solutions:
#             f.write("%s %s %.3f \n" % (indexes[i], array[0:-1], array[-1]))
#             i += 1


def try_add_performance_data(i,bd_shapes,bybin_list,directory,runs,archive_file, y_bottom,y_mid,y_top,from_fitfile=False):

    bd_dims=len(bd_shapes[i])

    try:
        avg_perform = avg_performances(bd_dims,  directory, runs, archive_file, 1.0,
                                       conversion_func=None,from_fitfile=from_fitfile)
        add_boxplotlike_data(avg_perform, y_bottom, y_mid, y_top, y_label="average_performance", method_index=i)

        global_perform = global_performances(bd_dims, directory, runs, archive_file, 1.0,
                                             conversion_func=None,from_fitfile=from_fitfile)
        add_boxplotlike_data(global_perform, y_bottom, y_mid, y_top, y_label="global_performance", method_index=i)

        if not from_fitfile:
            # coverage = coverages(bd_shapes[i], directory, runs, archive_file)
            # add_boxplotlike_data(coverage, y_bottom, y_mid, y_top, y_label="coverage", method_index=i)

            absolutecoverage = absolutecoverages(bd_shapes[i], directory,  runs, archive_file)
            add_boxplotlike_data(absolutecoverage, y_bottom, y_mid, y_top, y_label="absolute_coverage", method_index=i)

            globalcov = globalcoverage(bd_dims, directory, runs, archive_file,by_bin=bybin_list[i])
            add_boxplotlike_data([globalcov], y_bottom, y_mid, y_top, y_label="global_coverage", method_index=i)

            global_reliability = global_reliabilities(bd_dims, directory, runs, archive_file,by_bin=bybin_list[i])
            add_boxplotlike_data(global_reliability, y_bottom, y_mid, y_top, y_label="global_reliability", method_index=i)
    except IOError as e:
            print(e)

def get_time(t,label):
    if label.endswith("meta"):
        time=int(t//20)
    else:
        time=t
    return time

def coverage_development_plots(title,runs,times,BD_directory,title_tag, bd_type, legend_labels,bybin_list,fig=None,ax=None,metrics=None):

    colors=["C0","C1","C2","C3","C3","C4"]  # colors for the lines
    # (numsides, style, angle)
    markers=[(1,1,0),(1,2,0),(1,3,0),(3,1,0),(3,2,0),(3,3,0),(4,1,0),(4,2,0),(4,3,0)] # markers for the lines
    y_labels=["absolute_coverage"]


    boxes=[(.10,.40),(.10,.60),(.10,.60),(.45,.15),(0.20,0.20),(0.20,0.20)] # where to place the legend box
    y_bottom={ylabel:[[] for i in bd_type] for ylabel in y_labels}
    y_mid = {ylabel: [[] for i in bd_type]  for ylabel in y_labels}
    y_top = {ylabel: [[] for i in bd_type]  for ylabel in y_labels}



    for t in times:
        for i in range(len(bd_type)):
            print(legend_labels[i])
            projected=legend_labels[i].endswith("projected")
            abs_coverages=[]
            try:
                time=get_time(t,legend_labels[i])
                archive_file, directory = get_archive_plus_dir(BD_directory,bd_type[i],time)
                abs_coverage=absolutecoverages(bd_shapes[i], directory, runs, archive_file)
                abs_coverages.append(abs_coverage)

            except Exception as e:
                print(e)
            add_boxplotlike_data(abs_coverages, y_bottom, y_mid, y_top, y_label="absolute_coverage",method_index=i,
                                 statistic="meanall_replicatesd")


    j=0
    maximum_line = (times,[4096 for i in times])
    annots = {"text": "maximal coverage=4096","xy":(5000,4400),"xytext":(5000,4400),
              "fontsize":22,"align": "center"}
    for label in y_labels:
        #ylim=[0,4500]

        createPlot(y_mid[label],x_values=np.array(times),
                   save_filename=BD_directory + "/" + label + ".pdf", legend_labels=legend_labels,
                   colors=colors,markers=markers,xlabel="generations",ylabel=label.replace("_"," "),
                   xlim=[0,times[-1]+500],xscale="linear",yscale="log",ylim=[10**1,10**4],
                   legendbox=boxes[j],annotations=[annots],xticks=[],yticks=[],task_markers=[],scatter=False,
                   legend_cols=1,legend_fontsize=26,legend_indexes=[],additional_lines=[maximum_line],index_x=[],
                   xaxis_style="plain",y_err=[],force=True,fill_between=(y_bottom[label],y_top[label]),
                   ax=ax,title=title )

        j+=1




def development_plots(title,runs,times,BD_directory,bd_type, legend_labels,bybin_list,fig=None,ax=None,metrics=None):

    # bd_type = ["history","cvt_mutualinfo","cvt_mutualinfoact","cvt_spirit"]  #legend label
    #
    # legend_labels=["handcrafted","mutualinfo","mutualinfoact","spirit"]  # labels for the legend
    # colors=["C"+str(i) for i in range(len(bd_type))]  # colors for the lines
    # # (numsides, style, angle)
    # markers=[(3,1,0), (3,2,0),(4,1,0),(4,2,0)] # markers for the lines
    # bd_shapes = [32**2, 1000,1000,1000]  # shape of the characterisation
    # y_labels=["global_performance","global_reliability","precision","coverage"]

    # bd_type = ["baseline","history","cvt_rab_spirit","Gomes_sdbc_walls_and_robots_std","environment_diversity","environment_diversity"]  #legend label
    # legend_labels=["design","handcrafted","SPIRIT","SDBC","QED","QED-Translated"]  # labels for the legend




    colors=["C0","C1","C2","C3","C3","C4"]  # colors for the lines
    # (numsides, style, angle)
    markers=[(1,1,0),(1,2,0),(1,3,0),(3,1,0),(3,2,0),(3,3,0),(4,1,0),(4,2,0),(4,3,0)] # markers for the lines
    y_labels=["global_performance","average_performance","absolute_coverage","global_coverage","global_reliability"]


    boxes=[(.10,.40),(.10,.60),(.10,.60),(.45,.15),(0.20,0.20),(0.20,0.20)] # where to place the legend box
    y_bottom={ylabel:[[] for i in bd_type] for ylabel in y_labels}
    y_mid = {ylabel: [[] for i in bd_type]  for ylabel in y_labels}
    y_top = {ylabel: [[] for i in bd_type]  for ylabel in y_labels}


    for t in times:
        for i in range(len(bd_type)):
            time=get_time(t,legend_labels[i])
  
            print(legend_labels[i])
            archive_file, directory = get_archive_plus_dir(BD_directory,bd_type[i],time)
            try_add_performance_data(i,bd_shapes,bybin_list,directory,runs,archive_file, y_bottom,y_mid,y_top,from_fitfile=False)



    j=0

    for label in y_labels:
        ylim=[0,4096] if label in ["absolute_coverage","global_coverage"]   else [0.0,1.0]
        axis = None #if ax is None else ax[j]
        temp_labels = copy.copy(legend_labels)

        createPlot(y_mid[label],x_values=np.array(times),colors=colors,markers=markers,xlabel="generations",ylabel=label.replace("_"," "),ylim=None,
                   save_filename=args.DEST+"/Results/"+label+".pdf",legend_labels=temp_labels,
                   xlim=[0,times[-1]+500],xscale="linear",yscale="linear",
               legendbox=boxes[j],annotations=[],xticks=[],yticks=[],task_markers=[],scatter=False,
               legend_cols=1,legend_fontsize=26,legend_indexes=[],additional_lines=[],index_x=[],
               xaxis_style="plain",y_err=[],force=True,fill_between=(y_bottom[label],y_top[label]),
                   ax=axis,title=title )
        j+=1






def apply_star_and_bold(text,descriptor,target,max_descriptor,second_max_descriptor):
    if descriptor==max_descriptor:
        text=text+r"^{*}"
        if target == descriptor:
            text=r"$"+text+r"$"
        else:
            text=r"$\mathbf{"+text+"}$"
    else:
        if descriptor==second_max_descriptor:
            text = r"$\mathbf{" + text + "}$"
    return text





def create_all_development_plots():

    bd_type = ["damage_meta", "envir_meta", "bo", "duty","lv","random"]  # file system label for bd
    legend_labels=["damage-meta","envir-meta","body-orientation","duty-factor","linear-velocity","random"]  # labels for the legend
    bybin_list=["bd", "bd", "bd", "bd", "bd", "bd"]
    fig, axs = PLT.subplots(1, 4, figsize=(40, 10))  # coverage, avg perf., global perf., global reliability
    development_plots(title="",runs=runs, times=times,
                          BD_directory=args.DEST,bd_type=bd_type,
                          legend_labels=legend_labels,bybin_list=bybin_list,
                          ax = axs)


    finish_fig(fig, args.DEST+"/Results/development_plot.pdf")

def create_coverage_development_plots():
    bd_type = ["damage_meta", "envir_meta", "bo", "duty","lv","random"]  # file system label for bd
    legend_labels=["damage-meta","envir-meta","body-orientation","duty-factor","linear-velocity","random"]  # labels for the legend
    bybin_list=["bd", "individual", "individual", "bd", "bd", ""]
    times=range(0,7000, 1000)
    fig, axs = PLT.subplots(1,1, figsize=(15, 10))  # coverage, avg perf., global perf., global reliability
    coverage_development_plots(title="",runs=runs, times=times,
                          BD_directory=args.DEST,
                         title_tag="",bd_type=bd_type,
                          legend_labels=legend_labels,bybin_list=bybin_list,
                          ax = axs)


    finish_fig(fig, args.DEST +"/Results/coverage_plot.pdf")

def damage_performance():
    filenames=[get_output_folder_test(folder=args.DEST,condition="meta",test_type="damage",replicate=r) for r in runs]
    max_gen=270
    pop=5
    plotlines=[[] for i in range(pop)]
    plotstds = [[] for i in range(pop)]
    ts = range(0, max_gen + 10, 10)
    for t in ts:
        all_values=[[] for i in range(pop)]
        for f in filenames:
            path = f + "/recovered_perf"+str(t)+".dat"
            line = open(path).readline()
            values = line.split("\t")
            for i  in range(pop):
                all_values[i].append(float(values[i]))



        for i in range(pop):
            plotlines[i].append(np.mean(all_values[i]))
            plotstds[i].append(np.std(all_values[i]))
    markers = [(1, 1, 0), (1, 2, 0), (1, 3, 0), (3, 1, 0), (3, 2, 0), (3, 3, 0), (4, 1, 0), (4, 2, 0),
                           (4, 3, 0)]
    createPlot(plotlines, x_values=np.array(ts), colors=["C"+str(i) for i in range(pop)], xlabel="meta-generations",
               ylabel="damage recovery performance", ylim=None,markers=markers,
               save_filename=args.DEST + "/Results/recovery.pdf", legend_labels=["i"+str(i) for i in range(pop)],
               xlim=[0, ts[-1] + 10], xscale="linear", yscale="linear")

def envir_performance():
    filenames=[get_output_folder_test(folder=args.DEST,condition="meta",test_type="envir",replicate=r) for r in runs]
    max_gen=270
    pop=5
    plotlines=[[] for i in range(pop)]
    plotstds = [[] for i in range(pop)]
    ts = range(0, max_gen + 10, 10)
    for t in ts:
        all_values=[[] for i in range(pop)]
        for f in filenames:
            path = f + "/recovered_perf"+str(t)+".dat"
            line = open(path).readline()
            values = line.split("\t")
            for i  in range(pop):
                all_values[i].append(float(values[i]))



        for i in range(pop):
            plotlines[i].append(np.mean(all_values[i]))
            plotstds[i].append(np.std(all_values[i]))
    markers = [(1, 1, 0), (1, 2, 0), (1, 3, 0), (3, 1, 0), (3, 2, 0), (3, 3, 0), (4, 1, 0), (4, 2, 0),
                           (4, 3, 0)]
    createPlot(plotlines, x_values=np.array(ts), colors=["C"+str(i) for i in range(pop)], xlabel="meta-generations",
               ylabel="environment adaptation performance", ylim=None,markers=markers,
               save_filename=args.DEST + "/Results/adaptation.pdf", legend_labels=["i"+str(i) for i in range(pop)],
               xlim=[0, ts[-1] + 10], xscale="linear", yscale="linear")


if __name__ == "__main__":
    
    


    # for fitfun in fitfuns:
    #     # fitness-specific
    #     make_translation_table(fitfun, [get_bd_dir(fitfun)], runs)

        # global
    runs=["1","2","3"]
    fitfuns = [""]  # ,"DecayBorderCoverage","Flocking"]
    bd_type = ["envir_meta","damage_meta", "bo", "duty","lv","random"
             ]  # file system label for bd
    bd_shapes =[(16,16,16), (16,16,16), (4,4,4,4,4,4),(4,4,4,4,4,4), (16,16,16),(16,16,16)]  # shape of the characterisation
    times=range(0,7500,1000)

    #make_translation_table("DEBUG", [get_bd_dir(f) for f in fitfuns], runs,times=[generation],source="best")
         # print_best_individuals(
         #     BD_dir="/home/david/Data/ExperimentData/"+fitfun+"range11/Gomes_sdbc_walls_and_robots_std",
         #     outfile="best_solutions_"+fitfun+"NOCORRECT", number=10, generation=1200)


    #make_evolution_table(fitfuns, bd_type, runs, generation,load_existing=False)

    damage_performance()
    envir_performance()
    create_coverage_development_plots()

    create_all_development_plots()