from random import random
import sys
from math import log

n = 30000
prob = 2*log(n)/n

edges = []
for i in range(n):
    for j in range(i):
        if random() <= prob:
            edges.append((i, j))

print(n, len(edges))
for (u, v) in edges:
    print(u, v)
