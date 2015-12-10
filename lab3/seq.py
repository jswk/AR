#!/usr/bin/python2.7

import sys
from math import ceil


def step(sol, t, D, best):
    # verify if sol is fine
    d = dict()
    for p, time in zip(sol,[tup[1] for tup in t]):
        val = d.get(p, 0.)
        d[p] = val + time
        if d[p] > D:
            return best
    this_cost = int(sum([ceil(v) for v in d.itervalues()]))

    if this_cost >= best:
        # print("bound %s > %s"%(this_cost, best))
        return best

    if len(sol) == len(t):
        print("%s %s"%(sol, this_cost))
        return min(this_cost, best)

    # run step for other possibilites
    P = max(sol) + 1
    for p in range(P+1):
        best = step(sol+[p], t, D, best)

    return best


if __name__ == '__main__':

    if len(sys.argv) != 2:
        print("Usage: %s problem_file"%(sys.argv[0]))
        exit(1)

    with open(sys.argv[1], 'r') as f:
        D = int(f.readline())
        N = int(f.readline())
        t = []
        for n in range(N):
            t.append((n, float(f.readline())))
    t.sort(key=lambda tup: tup[1], reverse=True)
    print(t)

    best = sum([int(ceil(T[1])) for T in t])
    print("best = %s"%best)
    step([0], t, D, best)

