#!/usr/bin/env python

import sys
from math import ceil
from mpi4py import MPI
from heapq import heappop, heappush
from collections import deque

stage_depth = 3

def solve_stage(best, sol, D, t, depth=0):
    # verify if sol is fine
    d = dict()
    for p, time in zip(sol,[tup[1] for tup in t]):
        val = d.get(p, 0.)
        d[p] = val + time
        if d[p] > D:
            return []
    this_cost = int(sum([ceil(v) for v in d.itervalues()]))

    if this_cost >= best:
        # print("bound %s > %s"%(this_cost, best))
        return []

    if len(sol) == len(t) or depth == stage_depth:
        # print("%s %s"%(sol, this_cost))
        return [(this_cost, sol)]

    # run step for other possibilities
    P = max(sol) + 1
    nodes = []
    for p in range(P+1):
        nodes = nodes+solve_stage(best, sol+[p], D, t, depth=depth+1)

    return nodes

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

    comm = MPI.COMM_WORLD
    is_root = comm.rank == 0

    if is_root:
        with open(sys.argv[1], 'r') as f:
            D = int(f.readline())
            N = int(f.readline())
            t = []
            for n in range(N):
                t.append((n, float(f.readline())))
        t.sort(key=lambda tup: tup[1], reverse=True)
    else:
        D = None
        t = None

    comm.Barrier()
    D = comm.bcast(D, root=0)
    t = comm.bcast(t, root=0)
    N = len(t)

    if is_root:
        best = sum([int(ceil(T[1])) for T in t])
        best_sol = range(len(t))
        heap = [(N, best, [0])]
        reqs = deque()

    finish = False

    while not finish:
        if is_root:
            while comm.iprobe():
                res = comm.recv()
                # request for task
                if res[0] == 'req':
                    reqs.append(res[1])
                    # print res
                # new solution found
                elif res[0] == 'res':
                    # print res
                    score = res[1]
                    sol = res[2]
                    if score < best:
                        best = score
                        best_sol = sol
                # new tasks to be appended to heap
                elif res[0] == 'add':
                    # print (res[0], len(res[1]))
                    for task in res[1]:
                        if task[1] < best:
                            heappush(heap, task)
            # assign tasks to requests
            while len(heap) > 0 and len(reqs) > 0:
                task = heappop(heap)
                if task[1] > best:
                    continue
                req = reqs.popleft()
                comm.send(('task', best, task[2]), req)
                # print (('task', best, task[2]), req)
            if len(reqs) == comm.size-1 and len(heap) == 0:
                print('%d %s' % (best, best_sol))
                for req in reqs:
                    comm.send(('exit',), req)
                    # print (('exit',), req)
                finish = True
        else:
            res = comm.sendrecv(('req', comm.rank), 0)
            if res[0] == 'task':
                best = res[1]
                seed = res[2]
                sols = solve_stage(best, seed, D, t)
                if len(sols) == 0:
                    continue
                elif len(sols[0][1]) == N:
                    sol = min(sols)
                    comm.send(('res',)+sol, 0)
                else:
                    priority = N-len(sols[0][1])+1
                    sols = [(priority,)+sol for sol in sols]
                    comm.send(('add', sols), 0)
            elif res[0] == 'exit':
                finish = True



    # best = sum([int(ceil(T[1])) for T in t])
    # print("best = %s"%best)
    # step([0], t, D, best)

def pprint(str="", end="\n", comm=MPI.COMM_WORLD):
    """Print for MPI parallel programs: Only rank 0 prints *str*."""
    if comm.rank == 0:
        print str+end,
