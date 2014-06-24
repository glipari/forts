#!/usr/bin/python

"""
    This script generates the Hierarchical Scheduling Composition (HSC)
    model in the form of IMITATOR input. 

    Copyright (C) 2013 Youcheng Sun <y.sun@sssup.it> and Giuseppe Lipari. 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

"""

import copy
import json
import argparse

class SERVER :
    def __init__(self, pp, qq) :
        self.P = pp
        self.Q = qq

    def __str__(self) :
        return "SERVER(P=%s, Q=%s)" % (self.P, self.Q)
    def __repr__(self) :
        return self.__str__()

class TASKS :
    def __init__(self, ii, oo, cc, tt, dd) :
        self.id = ii
        self.O = oo
        self.C = cc
        self.T = tt
        self.D = dd

    def __str__(self) :
        return "TASKS(id=%s, id=%s, C=%s, T=%s, D=%s)" % (self.id, self.O, self.C, self.T, self.D)
    def __repr__(self) :
        return self.__str__()

"""
   Generates the starting part of the file
"""
def gen_clocks(out, ntasks) :
    out.write("\n")
    out.write("    ")
    for i in range(1, ntasks+1) :
        out.write("p"+str(i)+", ")
    for i in range(1, ntasks) :
        out.write("c"+str(i)+", ")
    #out.write("c"+str(ntasks)+" , x, y :continuous;\n")
    out.write("c"+str(ntasks)+" :continuous;\n")


"""
   Generates the parameter part. It needs to be called for each task
"""
def gen_task_parms(out, task) :
    out.write("    ")
    #out.write("O" + str(task.id) + " == " + str(task.O) +", ")
    out.write("T" + str(task.id) + " == " + str(task.T) +", ")
    out.write("C" + str(task.id) + " == " + str(task.C) +", ")
    out.write("D" + str(task.id) + " == " + str(task.D))
    #out.write(": discrete;\n\n")                  

""" 
    Writes the server parameters
""" 
def gen_server_parms(out, server) :
    out.write("\n    ")
    #out.write("P == " + str(server.P) + ", Q == " + str(server.Q))
    #out.write(": discrete;\n\n")                  


##
## Generates all possible combination of arrivals of ntasks
## basically a list of 0, 1, with 2^ntasks elements
##
def com(l1, l2):
    n = len(l1)
    for i in range(n):
      if l1[i]==l2[i]: continue
      if l1[i]==0: return True
      return False

def generate_arrival_list(ntasks) :
    if ntasks == 1 : 
        return [ [0], [1] ] 
    else :
        l = generate_arrival_list(ntasks - 1)
        l2 = []
        for x in l :
            y = copy.copy(x)
            x.append(0)
            y.append(1)
            l2.append(x)
            l2.append(y)
        #n = len(l2)
        #for i in range(n):
        #  for j in range(n-1):
        #    if com(l2[j],l2[j+1]):
        #      tmp1 = copy.copy(l2[j])
        #      tmp2 = copy.copy(l2[j+1])
        #      l2[j] = tmp2
        #      l2[j+1] = tmp1

        return l2



## returns the name of the state 
## which is taskId + R if running, or W if waiting
## (x or f must be added in front)
def build_state(l,cpus) :
    state = ""
    flag = True
    c = 0
    for i, e in enumerate(l) :
        if e == 1 :
            if flag == True :
                state = state + str(i+1) + "R"
                c = c + 1
                if c == cpus : flag = False
            else :
                state = state + str(i+1) + "W"
    return state

##
## Build the invariant for a initial state
##
#def build_invariant(l, fl = True) :
def build_init_invariant(l, cpus, fl=True) :
    inv = "true"
    #c = 0
    #for i, e in enumerate(l):
    #  if inv != "" :
    #      inv = inv + " & "
    #  inv = inv + "p{0}-T{0}<=0".format(str(i+1))

    return inv
##
## Build the invariant for a state
##
#def build_invariant(l, fl = True) :
def f_build_invariant(l, cpus, fl=True) :
    inv = ""
    c = 0
    for i, e in enumerate(l):
        if e == 1 :
            c = c + 1
            if fl :
                #if inv != "" :
                    #inv = inv + " & "
                if c <= cpus :
                    #if i == len(l) - 1:
                    if 1 == 1:
                      if inv != "" :
                          inv = inv + " & "
                      inv = inv + "p{0}-D{0}<=0".format(str(i+1))
                    #else:
                    #  if inv != "" :
                    #      inv = inv + " & "
                    #  inv = inv + "c{0}>=0".format(str(i+1))
                else :
                    #if i == len(l) - 1:
                    if 1 == 1:
                      if inv != "" :
                          inv = inv + " & "
                      inv = inv + "p{0}-D{0}<=0".format(str(i+1))
            #else:
            #    if c <= cpus :
            #        #if i == len(l) - 1:
            #        if i == i:
            #          inv = inv + "c{0}>=1&".format(str(i+1))
        #elif fl:
        #  if inv != "" :
        #      inv = inv + " & "
        #  inv = inv + "p{0}-T{0}<=0".format(str(i+1))

    return inv

##
## Build the invariant for a state
##
#def build_invariant(l, fl = True) :
def build_invariant(l, cpus, fl=True) :
    inv = ""
    c = 0
    for i, e in enumerate(l):
        if e == 1 :
            c = c + 1
            if fl :
                #if inv != "" :
                    #inv = inv + " & "
                if c <= cpus :
                    #if i == len(l) - 1:
                    if 1 == 1:
                      if inv != "" :
                          inv = inv + " & "
                      inv = inv + "c{0}>=0 & p{0}-D{0}<=0".format(str(i+1))
                    else:
                      if inv != "" :
                          inv = inv + " & "
                      inv = inv + "c{0}>=0".format(str(i+1))
                else :
                    #if i == len(l) - 1:
                    if 1 == 1:
                      if inv != "" :
                          inv = inv + " & "
                      inv = inv + "p{0}-D{0}<=0".format(str(i+1))
            else:
                if c <= cpus :
                    #if i == len(l) - 1:
                    if i == i:
                      inv = inv + "c{0}>0&".format(str(i+1))
                      #inv = inv + "p{0}-D{0}<=0".format(str(i+1))
        #elif fl:
        #  if inv != "" :
        #      inv = inv + " & "
        #  inv = inv + "p{0}-T{0}<=0".format(str(i+1))

    return inv

##
## Exchange between active and frozen states
## For now, it works only for single core system
##
#def build_invariant(l, fl = True) :
def active_frozen(l, cpus, fl=True) :
    inv = ""
    c = 0
    for i, e in enumerate(l):
        if e == 1 :
            c = c + 1
            if fl :
                if c <= cpus :
                    #if i == len(l) - 1:
                    if i == i:
                      inv = inv + "c{0}>0".format(str(i+1))
                      #inv = inv + "p{0}-D{0}<=0".format(str(i+1))
            else:
		inv = inv +"true"
		return inv
            #    if c <= cpus :
            #        #if i == len(l) - 1:
            #        if i == i:
            #          inv = inv + "c{0}>=1".format(str(i+1))
            #          #inv = inv + "p{0}-D{0}<=0".format(str(i+1))

    return inv

##
## The clocks to be stopped in an executing state 
##
#def build_exec_stop(l, fl=True) :
def f_build_exec_stop(l, cpus) :
    wait = ""
    c = 0
    flag = True
    for i, e in enumerate(l):
        if wait != "" :
            wait = wait + ", "
        else: 
            wait = wait + "wait { "
        if e == 1 :
            if flag == True:
                wait = wait + "c{0}'=0".format(str(i+1))
            else:
                wait = wait + "c{0}'=0".format(str(i+1))
            c = c + 1
            if c == cpus:
                flag = False
        else: 
            wait = wait + "c{0}'=0".format(str(i+1))
    wait = wait + "}\n"
    return wait

##
## The clocks to be stopped in an executing state 
##
#def build_exec_stop(l, fl=True) :
def build_exec_stop(l, cpus) :
    wait = ""
    c = 0
    flag = True
    for i, e in enumerate(l):
        if wait != "" :
            pass #wait = wait + ", "
        else: 
            wait = wait + "wait { "
        if e == 1 :
            if flag == True:
                if wait == "wait { " :
                    wait = wait + "c{0}'=-1".format(str(i+1))
                else :
                    wait = wait + ", c{0}'=-1".format(str(i+1))
            else:
                if wait != "wait { ":
                    wait = wait + ","
                wait = wait + "c{0}'=0".format(str(i+1))
            c = c + 1
            if c == cpus:
                flag = False
        else: 
            if wait != "wait { ":
                wait = wait + ","
            wait = wait + " c{0}'=0".format(str(i+1))
    wait = wait + "}\n"
    return wait

##
## Counts the number of active tasks (the number of 1s)
##    
def count_active(l) :
    s = 0
    for e in l :
        s = s + e
    return s

##
## Returns the index of the first active task
##
def find_first(l) :
    for i, e in enumerate(l) :
        if e == 1 :
            return i+1
    
##
## Returns the list of index for active tasks
##
def find_active(l, cpus):
    c = 0
    act = []
    for i, e in enumerate(l):
        if c == cpus: break
        if e== 1:
          act.append(i+1)
          c = c + 1
          
    return act

##
## Returns the list of index for inactive tasks
##
def find_inactive(l, cpus):
    c = 0
    inact = []
    for i, e in enumerate(l):
        if e== 0:
          inact.append(i+1)
    return inact

##
## Returns the list of index for tasks in waiting list
##
def find_wlist(l, cpus):
    c = 0
    wlist = []
    for i, e in enumerate(l):
        if e== 1:
          if c < cpus :
              c = c + 1
          else: wlist.append(i+1)
          
    return wlist

#
# Removes one active task index from l
#
def remove_active(l, tk) :
    l2 = copy.copy(l)
    for i, e in enumerate(l2) :
        if i == tk-1 :
            l2[i] = 0
            return l2
    return l2

#
# Removes the first 1
#
def remove_first(l) :
    l2 = copy.copy(l)
    for i, e in enumerate(l2) :
        if e == 1 :
            l2[i] = 0
            return l2
    return l2

##
## The list of indexes (in strings) of the low priority tasks
##
def low_prio_list(l) :
    l2 = remove_first(l)
    sl = []
    for i, e in enumerate(l2): 
        if e == 1 :
            sl.append(str(i+1))

    return sl

##
## The list of non active tasks
##
def new_arrivals_list(l) :
    sl = []
    for i, e in enumerate(l): 
        if e == 0 :
            sl.append(i)

    return sl
    
##
## Set the i-th bith to 1
##
def set_arrival(l, i) :
    l2 = copy.copy(l)
    l2[i] = 1
    return l2


"""
   Generates the scheduling automaton
"""
def gen_sched(out, ntasks, cpus) :    
    alist = generate_arrival_list(ntasks)

    out.write("\n\nautomaton sched\n\n")
    #out.write("sync : ")
    #for i in range(1, ntasks+1) :
    #    id = str(i)
    #    if i != ntasks:
    #        out.write("r{0}, ".format(id))
    #    else :
    #        out.write("r{0};\n\n".format(id))
    #out.write(" empty, go, frozen;\n\n");

    #out.write("loc idle: while true " );
    l = alist[0]
    out.write("loc idle : while {0}".format(build_init_invariant(l, cpus)))
    print l
    s = build_exec_stop(l, cpus)
    if s != "" :
        out.write(" {0}".format(s))
    for i in range(1, ntasks+1) :
        id = str(i)
        out.write("    when p{0}-T{0}>=0  do {{p{0}'=0, c{0}'=C{0}}} goto x{0}R;\n".format(id))
    out.write("\n");

    #alist = generate_arrival_list(ntasks)
    alist = alist[1:]
    
    for l in alist :
        print l
        ## build the x state
        state = "x" + build_state(l, cpus)            
        print "State : " + state

        out.write("\nloc {0} : while {1}".format(state, 
                                                 build_invariant(l, cpus)))
        s = build_exec_stop(l, cpus)
        if s != "" :
            out.write(" {0}".format(s))

        ## first edge: the task completes execution
        act = find_active(l, cpus)
        inact = find_inactive(l, cpus)
        wlist = find_wlist(l, cpus)
        for i,e in enumerate(act) :
            if count_active(l) == 1 :
                newstate = "idle"
            	#out.write("    when c{0} == 0  sync empty do {{}} goto {1};\n".
            	out.write("    when c{0} == 0  do {{}} goto {1};\n".
            	          format(str(e),
            	                 newstate))
            else:
                label = ""
                newstate = "x" + build_state(remove_active(l,e), cpus)
            	out.write("    when c{0} == 0  do {{}} goto {1};\n".
            	          format(str(e),
            	                 newstate))

        ## second set of edges: deadline misses
        for i,e in enumerate(act):
            #if not e == len(l) : continue
            out.write("    when c{0}>0 & p{0}-D{0}==0 do {{}} goto error;\n".
                  format(str(e)))
        for i,e in enumerate(wlist):
            #if not e == len(l) : continue
            out.write("    when p{0}-D{0}==0 do {{}} goto error;\n".
                  format(str(e)))

        # another set of edges: arrival of new jobs
        for i,e in enumerate(act):
            out.write("    when p{0}-T{0}>=0  sync r{0} do {{p{0}'=0, c{0}'=c{0}+C{0}}}  goto {1} ;\n".
                      format(str(e), state))
        for i,e in enumerate(wlist):
            out.write("    when p{0}-T{0}>=0  sync r{0} do {{p{0}'=0, c{0}'=c{0}+C{0}}}  goto {1};\n".
                      format(str(e), state))
        for i,e in enumerate(inact):
            ll = []
            mm = 0
            pos=-1
            for ii, ee in enumerate(l):
              ll.append(0)
            for ii, ee in enumerate(l):
              if ee == 0:
                continue
              else : mm = mm+1
              if mm > cpus : break
              if e < ii+1: 
                pos=ii
            if mm >= cpus and pos != -1: ll[pos]=1
            newstate = "x" + build_state(set_arrival(l, e-1), cpus)
            out.write("    when {0}p{1}-T{1}>=0   do {{p{1}'=0, c{1}'=C{1}}} goto {2};\n".
                     format(build_invariant(ll, cpus,False),
                            str(e),
                            newstate))
        ## next, the frozen edge
        #out.write("    when {0} sync frozen do {{}} goto {1};\n".
        #          format(active_frozen(l, cpus, True),
                         #"f" + build_state(l,cpus)))
    ## now, let us build the f state
#    for l in alist :
#        print l
#        ## build the x state
#        state = "f" + build_state(l, cpus)            
#        print "State : " + state
#
#        out.write("\nloc {0} : while {1}".format(state, 
#                                                 f_build_invariant(l, cpus)))
#        s = f_build_exec_stop(l, cpus)
#        if s != "" :
#            out.write(" {0}".format(s))
#
#        ## first edge: the task completes execution
#        act = find_active(l, cpus)
#        inact = find_inactive(l, cpus)
#        wlist = find_wlist(l, cpus)
#        #for i,e in enumerate(act) :
#        #    if count_active(l) == 1 :
#        #        newstate = "idle"
#        #    else:
#        #        label = ""
#        #        newstate = "x" + build_state(remove_active(l,e), cpus)
#        #    out.write("    when c{0} == 0  do {{}} goto {1};\n".
#        #              format(str(e),
#        #                     newstate))
#
#        ## second set of edges: deadline misses
#        for i,e in enumerate(act):
#            #if not e == len(l) : continue
#            out.write("    when p{0}-D{0}==0 do {{}} goto error;\n".
#                  format(str(e)))
#        for i,e in enumerate(wlist):
#            #if not e == len(l) : continue
#            out.write("    when p{0}-D{0}==0 do {{}} goto error;\n".
#                  format(str(e)))
#
#        ## another set of edges: arrival of new jobs
#        #for i,e in enumerate(act):
#        #    out.write("    when p{0}-T{0}>=0  sync r{0} do {{p{0}'=0, c{0}'=c{0}+C{0}}}  goto {1} ;\n".
#        #              format(str(e), state))
#        #for i,e in enumerate(wlist):
#        #    out.write("    when p{0}-T{0}>=0  sync r{0} do {{p{0}'=0, c{0}'=c{0}+C{0}}}  goto {1};\n".
#        #              format(str(e), state))
#        for i,e in enumerate(inact):
#            ll = []
#            mm = 0
#            pos=-1
#            for ii, ee in enumerate(l):
#              ll.append(0)
#            for ii, ee in enumerate(l):
#              if ee == 0:
#                continue
#              else : mm = mm+1
#              if mm > cpus : break
#              if e < ii+1: 
#                pos=ii
#            if mm >= cpus and pos != -1: ll[pos]=1
#            newstate = "f" + build_state(set_arrival(l, e-1), cpus)
#            out.write("    when {0}p{1}-T{1}==0   sync r{1} do {{p{1}'=0, c{1}'=C{1}}} goto {2};\n".
#                     format(f_build_invariant(ll, cpus, False),
#                            str(e),
#                            newstate))
#        ## next, the frozen edge
#        out.write("    when {0} sync go do {{}} goto {1};\n".
#                  format(active_frozen(l, cpus, False),
#                         "x" + build_state(l,cpus)))
#
    out.write("\nloc error: while true wait {}\n\n")
    out.write("end\n\n")

"""
   Generate the server automaton
"""
def gen_server(out, ntasks, server) :
    out.write("automaton server\n\n")
    
    out.write("sync: ")
    for i in range(1, ntasks+1) :
        if i != 1 :
            out.write(", ")
        out.write("r{0}".format(str(i)))
    out.write(", empty, go, frozen;\n")

    out.write("loc idle: while true wait{x'=0, y'=0}\n")
    for i in range(1, ntasks+1) :
        out.write("    when true sync r{0} do {{ x'=0, y'=0 }} goto active;\n".
                  format(str(i)))
    out.write("loc active: while y-x <= P-Q wait {x'=0}\n")
    for i in range(1, ntasks+1) :
        out.write("    when true sync r{0} do {{ }} goto active;\n".
                  format(str(i)))
    out.write("    when true sync go do {} goto executing;\n")
    
    out.write("loc executing: while x <= Q wait {}\n")
    for i in range(1, ntasks+1) :
        out.write("    when true sync r{0} do {{ }} goto executing;\n".
                  format(str(i)))
    out.write("    when x == Q sync frozen do {} goto recharging;\n")
    out.write("    when {0}*x >= {1}*y sync empty do {{  }} goto empty;\n".
              format(str(server.P), str(server.Q)))
    out.write("    when {0}*x <= {1}*y-1 sync empty  do {{ x'=0,y'=0 }} goto idle;\n".
              format(str(server.P), str(server.Q)))
    out.write("    when x <= Q-1 sync frozen do { } goto active;\n");
    
    out.write("loc recharging: while y <= P wait {x'=0}\n")
    for i in range(1, ntasks+1) :
        out.write("    when true sync r{0} do {{ }} goto recharging;\n".
                  format(str(i)))
    out.write("    when y == P do {x'=0,y'=0} goto active;\n")
    
    out.write("loc empty: while {0}*x >= {1}*y wait {{x'=0}}\n".
              format(str(server.P), str(server.Q)))
    for i in range(1, ntasks+1) :
        out.write("    when true sync r{0} do {{ }} goto active;\n".
                  format(str(i)))
    out.write("    when {0}*x == {1}*y do {{}} goto idle;\n".
              format(str(server.P), str(server.Q)))
    out.write("end\n\n")

def gen_init(out, ntasks) :
    out.write("init := \n")
    out.write("    loc[sched] == idle & \n")  
    #out.write("    loc[server] == idle & \n")  
    for i in range(1, ntasks) :
        out.write("    p{0}>=0 & c{0}==0 &\n".
                  format(str(i)))
    out.write("    p{0}>=0 & c{0}==0;\n".
              format(str(ntasks)))
    out.write("bad := loc[sched]==error;\n")
    

def create_tasks(value) :
    tasks = []
    for i in value :
        task = TASKS(i["id"], i["O"], i["C"], i["T"], i["D"])
        tasks.append(task)
    return tasks

def create_server(value) :
    for i in value :
        server = SERVER(i["P"], i["Q"])
    return server

def main() : 

    parser = argparse.ArgumentParser(description='Given a periodic server and its tasks described in a JSON file, this script builds the HSC model in the form of IMITATOR input.')
    parser.add_argument("inputfile", help="The JSON file to process")     
    parser.add_argument("outputfile", help="The output IMITATOR file")     
    args = parser.parse_args()

    fin = open(args.inputfile)
    json_data = fin.read()
    data = json.loads(json_data)

    
    for k,v in data.iteritems() :
        if k == "SERVER" :
           s = create_server(v)
        if k == "CPUS" :
           cpus = v 
        if k == "TASKS" :
           tasks = create_tasks(v)

    fin.close()

    fout = open(args.outputfile, "w")
    n = len(tasks)

    gen_clocks(fout, n)

    for i in xrange(n) :
        gen_task_parms(fout, tasks[i])
        #fout.write(", ")
    #gen_server_parms(fout, s)
        if i < n-1:
          fout.write(", ")
        else:
          fout.write(": discrete; ")


    cpus = 2
    gen_sched(fout, n, cpus)
    #gen_server(fout, n, s)
    gen_init(fout, n)

    fout.close()

if __name__ == "__main__" :
    main()
