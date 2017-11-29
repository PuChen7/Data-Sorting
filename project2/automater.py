

#!/usr/bin/python

import os
import string
#
if __name__ == '__main__':
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
            file.close()


