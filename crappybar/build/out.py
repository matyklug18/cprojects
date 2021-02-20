import sys
import os

y = 16
maxi = 6

for line in sys.stdin:
    loc = line.split()
    if int(loc[0])-y in range(0,y*6) and int(loc[1]) in range(0,y*2):
        num = int(loc[0])//y
        os.system("i3 workspace {}".format(num))
    print(loc)
