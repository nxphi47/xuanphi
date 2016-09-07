import matplotlib.pyplot as plt
import GPy as gp
import numpy as np

# load in training and testing data
train = np.loadtxt('rawTrain.txt')
test = np.loadtxt('rawTest.txt')


feat1_train = np.loadtxt('comfeat_train0.txt')
feat2_train = np.loadtxt('comfeat_train1.txt')
feat3_train = np.loadtxt('comfeat_train2.txt')
trainFeat = [feat1_train,feat2_train,feat3_train]

feat1_test = np.loadtxt('comfeat_test0.txt')
feat2_test = np.loadtxt('comfeat_test1.txt')
feat3_test = np.loadtxt('comfeat_test2.txt')
testFeat = [feat1_test,feat2_test,feat3_test]

trainX = train[:,:-1]
trainY = train[:,-1][:,None]

testX = test[:,:-1]
testY = test[:,-1][:,None]

# normalise output
meany = np.mean(trainY)
vary = np.std(trainY, ddof=1)
trainY = (trainY - meany) / vary
for sid in range(-3,4):
    # deal with feature alone
    if sid < 0:
        trainX = trainFeat[-sid-1]
        testX = testFeat[-sid-1]
    # deal with feature combined with raw data
    elif sid > 0:
        trainX = np.hstack((trainX,trainFeat[sid-1]))
        testX = np.hstack((testX,testFeat[sid-1]))

    dim = trainX.shape[1]
    # normalise input
    tmpx = np.concatenate((trainX, testX))
    tmpx = (tmpx - np.mean(tmpx,axis=0))/np.std(tmpx,axis=0,ddof=1)
    numD = len(trainX)
    trainX = tmpx[:numD,:]
    testX = tmpx[numD:,:]


    # setting up a initial guess of hyper-parameters
    #avg1 = [np.log(2) - 2*np.log(np.sqrt(np.mean(np.fabs(np.diff(trainX[:,i]))))) + np.random.normal(0,3) for i in range(trainX.shape[1])]
    #avg2 = [np.log(2) - 2*np.log(0.1*np.sqrt(np.mean(np.fabs(np.diff(trainX[:,i]))))) + np.random.normal(0,3) for i in range(trainX.shape[1])]
    avg1 = [2 / (np.sqrt(np.mean(np.fabs(np.diff(trainX[:,i])))))**2 + np.random.normal(0,5) for i in range(trainX.shape[1])]
    avg2 = [2 / (0.1*np.sqrt(np.mean(np.fabs(np.diff(trainX[:,i])))))**2 + np.random.normal(0,5) for i in range(trainX.shape[1])]
    avg = np.maximum(avg1,avg2)

    # define a kernel function (RBF ard kernel)
    k = gp.kern.RBF(dim,10.0,avg,True)# + gp.kern.White(dim,1.0)

    # get a GPRegression model m, set the restart runs as 5
    m = gp.models.GPRegression(trainX, trainY, k)
    #m.optimize('bfgs')
    m.optimize_restarts(4)


    # obtain the prediction result on testX, res contains 4 arrays: predict mean, predict variance, lower and up 95% confident interval
    res=m.predict(testX)
    # res[0] contains predict mean
    ym = res[0] * vary + meany
    # res[1] contains predict variance
    ys = res[1] * vary * vary

    resname = "res%d.txt" % sid
    resf = open(resname, 'w+')

    # compare with the real value, display the results
    meantest = meany
    #meantest = 15
    s1 = np.where(testY > meantest)[0]
    print "number of >15m samples: %d" % s1.shape
    s2 = np.where(ym[s1] > meantest)[0]
    print "number of >15m correctly labeled: %d" % s2.shape
    resf.write("%d %d\n" % (len(s1), len(s2)))
    s1 = np.where(testY < meantest)[0]
    print "number of <15m samples: %d" % s1.shape
    s2 = np.where(ym[s1] < meantest)[0]
    print "number of <15m correctly labeled: %d" % s2.shape
    resf.write("%d %d\n" % (len(s1), len(s2)))
    for i in range(0,len(ym)):
        resf.write("%lf %lf\n" % (ym[i], ys[i]))
    resf.close()
    
    #print ym
    #print ys
    k = m.kern
    C = k.K(trainX,trainX)

    res = m.param_array.tolist()
    print m
    
    hypname = "res-hyp%d.txt" % sid
    hypf = open(hypname, 'w+')

    signal = res[0]
    signal = 0.5 * np.log(signal)
    noise = res[-1]
    noise = 0.5 * np.log(noise)
    normalize = 0
    hypf.write("%lf %lf %d %d %d %d\n" % (signal, noise, trainX.shape[1], trainX.shape[0], testX.shape[0], normalize))
    res = res[1:-1]
    for item in res:
        hypf.write("%lf\n" % np.log(item))
    hypf.close()


