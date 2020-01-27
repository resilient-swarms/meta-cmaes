




import argparse
from filenames import *

parser = argparse.ArgumentParser(description='Process destination folder.')
parser.add_argument('-d', dest='DEST', type=str)

args = parser.parse_args()



def get_performances(condition,test_type,replicate):
    filename = get_file_name_test(args.DEST,condition,test_type,replicate)
    print("will get performance at "+ filename)
    x=[]
    with open(filename, 'r') as f:
        for line in f:
            if line: #avoid blank lines
                x.append(float(line.strip()))
    return x


if __name__ == "__main__":
    conditions=["meta", "bo", "duty" ,"lv" ,"random"]
    test_types=["envir","damage"]
    for t in test_types:
        for r in ["1","2"]:
            for c in conditions:
                if c!="meta" and r=="2": continue
                p=get_performances(c,t,r)
                if p:
                    print(c + " "+t+ " "+r+":")
                    #print(p)
                    print("mean="+str(np.mean(p)))
                    print("max="+str(np.max(p)))
