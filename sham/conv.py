import numpy as np

a = np.loadtxt("sham_data2.tsv")
for i in a:
    print("{{{:.4},\t{:.4}}},".format((i[0]-a[0,0])/1000.0,i[1]))
