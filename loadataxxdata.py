import numpy as np

class ataxxdata:
    def __init__(self,filename):
        self.nowindex=0
        td=np.loadtxt(filename,dtype=np.float32)
        #np.random.shuffle(td)
        tcol=td[:,0:1]
        tp=td[:,1:148]
        tval=td[:,148:149]
        tp=tp.reshape([-1,49,3])
        #tp=td[:,1:50]
        #tval=td[:,50:51]
        #tp=tp.reshape([-1,49,1])
        self.ecnt=td.shape[0]
        print("%d data loaded"%(self.ecnt))
        self.data=[tcol,tp,tval]
    def next_batch(self, batchsize):
        s1=[x[self.nowindex:self.nowindex+batchsize] for x in self.data]
        self.nowindex+=batchsize
        if self.nowindex>self.ecnt:
            self.nowindex%=self.ecnt
            return [np.vstack((s1[i],self.data[i][:self.nowindex])) for i in range(3)]
        else:
            return s1
