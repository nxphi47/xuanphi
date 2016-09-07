import re
import csv
import os
dirList = ['5-10','5-10-15', '5-15','10-15']
fileName = ['fvA', 'fvB', 'fvC']
removeList = [[75, 76, 77, 78, 79, 80, 81, 540, 623],[244, 270, 271, 272, 273, 274, 275, 276, 968],[18, 19, 20, 21, 22, 23, 24, 608],[535]]

for dl in range(0,4):
    os.chdir(dirList[dl])
    print os.getcwd()
    
    for i in range(0,3):
        trainName = "train-NUS.%s.txt" % fileName[i]
        trainFile = open(trainName, 'r')
        testFile = open('test.txt', 'r')
        fname = "processed.data.20160527-NUS_%s.csv" % fileName[i]
        rawTrain = open('rawTrain.txt', 'wa')
        rawTest = open('rawTest.txt', 'wa')
        rawFile = csv.reader(open(fname, 'rb'), delimiter = "\t")
        trainFile = [a for a in trainFile]
        raw = [a for a in rawFile]
        raw.pop(0)

        trainFile.pop()
        tname = [t.split(',') for t in testFile][0]

        ftrainName = "comfeat_train%d.txt" % (i)
        ftestName = "comfeat_test%d.txt" % (i)
    
        comfeatTrain = open(ftrainName, 'w+')
        comfeatTest = open(ftestName, 'w+')

        for item in trainFile:
            # extract the combined feature
            rawL = item[14:]
            featL = item[30:]
            outer = re.compile("\[(.+)\]")
            m = outer.search(featL)
            inner_str = m.group(1)
            res = re.sub('[\[,\]]', '', inner_str)
            # extract the joint ID
            tmp = rawL.split(']')[0]
            tmpnum = [int(a) for a in tmp.split(',')]

            total = sum(tname[i].rstrip('\n') in item for i in range(len(tname)))
            height = 0;
            if set(tmpnum).intersection(removeList[dl]):
                continue;
                
            for j in range(0,3):
                rid = tmpnum[j] - 1
                tmp1 = raw[rid][1].split(',')
                tmp2 = raw[rid][2].split(',')
                height = raw[rid][5]

                for k in range(len(tmp1)):
                    if total > 0:
                        rawTest.write('%s %s ' % (tmp1[k], tmp2[k]))
                    else:
                        rawTrain.write('%s %s ' % (tmp1[k], tmp2[k]))

            if total > 0:
                rawTest.write(height + '\n')
            else:
                rawTrain.write(height + '\n')

            if total > 0:
                comfeatTest.write(res + '\n')
            else:
                comfeatTrain.write(res + '\n')

        comfeatTrain.close()
        comfeatTest.close()




    os.chdir("..")
