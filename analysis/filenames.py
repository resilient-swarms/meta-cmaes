
import numpy as np


def get_output_folder_test(folder,condition,test_type,replicate):
    if condition == "meta":
        #last_gen="50"
        method_tag=test_type+"_"+condition
    else:
        method_tag=condition
        #last_gen="1500"

    return folder+"/"+method_tag+"/exp"+replicate

def get_file_name_test(folder,condition,test_type,replicate):
    
    Outfolder=get_output_folder_test(folder,condition,test_type,replicate)
    return Outfolder+"/"+test_type+"_performance"



def get_archive_plus_dir(folder,condition,time):
    """

    """
    if "meta" in condition:
        archive_tag=str(time)+"_archive0.dat"
    else:
        archive_tag="archive_"+str(time)+".dat"
        #last_gen="1500"

    Outfolder=folder+"/"+condition+"/exp"
    return archive_tag ,Outfolder