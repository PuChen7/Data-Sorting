

#!/usr/bin/python

import os
import string
import random
#
def csvFormat(data):
    ret=""
    for dt in data:
        ret=ret+dt
    return ret

if __name__ == '__main__':
    dataGroup=["demo.csv","demo2.csv","demo3.csv"]
    dataSet=[]
    for dataFile in dataGroup:
        file = open(dataFile, "r")
        dataSet.append(file.readlines())
        file.close()


    level = input("how many levels\n")
    rootdir = "testdir"
    if not os.path.exists(rootdir):
        os.makedirs(rootdir)
    directories=list(string.ascii_lowercase[26-level:])
    files=range(10)
    subdir=""
    for dir in directories:
        subdir+=str(dir)+"/"
        for file in files:
            if not os.path.exists(str(rootdir+"/"+subdir)):
                os.makedirs(str(rootdir+"/"+subdir))
            filename = rootdir+"/"+subdir + str(dir) + "_" + str(file) + ".csv"
            file = open(filename, "w")
            index = random.randint(0,2)
            csv = csvFormat(dataSet[index])
            file.write(csv)
            file.close()


