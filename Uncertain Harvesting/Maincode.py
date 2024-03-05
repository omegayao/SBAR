# import modules
import numpy as np
from gurobipy import *
# <----------------------- Read Data ----------------------->

path = (r"/home/u20/mahdimahdavi//uncertain harvesting.txt")

with open(path) as f:

 # Sets and indices
    F = list(range(int(f.readline()) + 1))  # fields
    H = list(range(1, int(f.readline()) + 1))  # harvesters
    K = list(range(1, int(f.readline()) + 1))  # harvest seasons
    P = list(range(int(f.readline()) + 1))  # time periods

    ## Parameters
    C_h = float(f.readline())/2  # rent cost: divided by 2 to make the conversion from per year to per harvest season
    if C_h <0.01:
        C_h += 1 # add a small epsilon to avoid redundant harvesters when c_h = 0
    C_o = float(f.readline())  # operational cost
    C_r = float(f.readline())  # transportation cost
    C_penalty = float(f.readline())  # penalty cost (derived from average gross profit)

    M = int(f.readline()) * np.ones(len(F))  # maximum number of harvesters that can harvest one field in a given period
    M = M.tolist()
    M_cap = float(f.readline())  # harvesting capacity

    loc = int(f.readline())  # facility location & 29 is the default facility location

    A = [0] + list(map(float, f.readline().split()))  # acres for fields & 29 is the default facility location
    R = {}  # ready date
    D = {}  # due date
    for k in K:
        R[0, k], D[0, k] = (1, 3)
    for i, ss in enumerate(f.readline().split(), start=1):
        for k in K:
            R[i, k] = int(ss)
    for i, ss in enumerate(f.readline().split(), start=1):
        for k in K:
            D[i, k] = int(ss)

    ins = {}
    I = np.zeros((len(F), len(K) + 1))  # indicator
    for k in K:
        ins[k] = list(map(int, f.readline().split()))
        for idx in ins[k]:
            I[idx, k] = 1 # indicate which fields need to be harvested during season k

    # distance matrix
    d = np.zeros((len(F), len(F)))
    for i in F[1:]:
        for j, ss in enumerate(f.readline().split(), start=1):
            d[i, j] = float(ss)

    for i in F[1:loc] + F[loc + 1:]:
        d[i][i] = 1000  # add an extra cost for staying in the same field except the facility 
                        #i.e. enforce the harvester not to stay in the same field except the facility
de = np.zeros((len(F)+1, len(P)+1, len(K)+1))
              
for p in range(1,5):
    for k in  range(1,len(K)+1):
        for f in  ins[k]:
            de[f,p,k]= 400
# d[]
C_sh = 10 # shortage cost (derived from average sale price and costs)
C_In = 3  # inventory cost (derived from average operation costs)
hw = 56  # maximum capacity of working time for harvesters
Vha_h = 6.07  # harvesting speed of harvesters
Vtr_h = 5.76  # Transportation speed of harvesters

# Create the model
# solver settings
m = Model('multi stage Harvesting')
# m.params.LogFile = ('Harvest.log')
# setParam("LogToConsole", 0)
m.ModelSense = GRB.MINIMIZE
m.Params.MIPGap = 0.05    # 1%
# m.Params.TimeLimit = 300  # 5 minutes
m.setParam('Threads', 94)
# scenario tree for multistage programming 
# number of drought scenario in every harvest season k 
num_drought = 3
# probability of drought scenarios
o = [0.6, 0.3, 0.1] 
# number of scenarios
S = num_drought**len(K) 
# generating scenarios
#y_scen = np.zeros([G, S])
y_scen= np.zeros([len(K)+1, S])
G = np.zeros([len(F),S,len(K)+1])
p_scen = np.zeros([S])
zz = np.zeros([len(K)+1, S])
mn = np.zeros([len(K)+2, S])
# probability of each scenario
for s in range(S):
    p_scen[s] = o[int(math.floor(s/(num_drought**(0))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(1))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(2))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(3))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(4))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(5))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(6))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(7))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(8))))%num_drought]
       
# calculating the stochastic parameters
for s in range(S):
    y_scen[0, s]=12
for s in range(S):
    for k in K:
        if (int(math.floor(s/(num_drought**((len(K))-k))))%num_drought)==0:
            y_scen[k, s] = y_scen[k-1, s]* 1
        if (int(math.floor(s/(num_drought**((len(K))-k))))%num_drought)==1:
            y_scen[k, s] = y_scen[k-1, s]* 0.70
        if (int(math.floor(s/(num_drought**((len(K))-k))))%num_drought)==2:
            y_scen[k, s] = y_scen[k-1, s]* 0.30   

for s in range(S):
    for k in K:
        for f in ins[k]:
            G[f, s, k]= y_scen[k, s] * A[f]

# Variables
u = m.addVars([(h, k) for h in H for k in K], vtype=GRB.BINARY, name='u')
x = m.addVars([(f, p, k, s) for p in P for k in K for f in F for s in AA], vtype=GRB.BINARY, name='x')
y = m.addVars([(h, p, i, j, k, s) for k in K for h in H for p in P for i in F for j in F for s in AA], vtype=GRB.BINARY, name='y')
z = m.addVars([(h, f, p, k,s) for k in K for h in H for f in F for p in P for s in AA], vtype=GRB.BINARY, name='z')
alpha = m.addVars([(h, f, p, k, s) for k in K for h in H for f in F for p in P  for s in AA], lb=0, ub=1, vtype=GRB.CONTINUOUS,
                  name='alpha')
st = m.addVars([(f, k , s) for k in K for f in F for s in AA], lb=0, ub=len(P) + 1, vtype=GRB.INTEGER, name='s')
e = m.addVars([(f, k, s) for k in K for f in F for s in AA], lb=0, ub=len(P), vtype=GRB.INTEGER, name='e')
pe = m.addVars([(f, k, s) for k in K for f in F for s in AA], lb=0, ub=len(P) + 1, vtype=GRB.CONTINUOUS, name='pe')
q = m.addVars([(p, s, f, k) for p in P for s in AA for k in K for f in F ], lb=0 , vtype=GRB.CONTINUOUS, name='q')
In = m.addVars([(p, s, f, k) for p in P for s in AA for k in K for f in F ], lb=0 , vtype=GRB.CONTINUOUS, name='in')

#forward step
for s in range(S):
    for k in K:
        m.setObjective( quicksum(u[h,k]*C_h for h in H for k in K) +
            quicksum(p_scen[s]* C_o * alpha[h, f, p, k, s] * A[f] for f in F  for h in H for p in P ) +
            quicksum(p_scen[s]* y[h, p, i, j, k, s] * C_r * d[i][j] for i in F for j in F for p in P for h in H )+
            quicksum(p_scen[s]* C_penalty* pe[f, k, s] * A[f] * I[f, k] for f in ins[k] ) +
            quicksum(C_sh * p_scen[s]* q[p, s, f, k] for p in P for f in F)+ quicksum(C_In* In[p, s, f, k]* p_scen[s] for p in P for f in F))
        # Constraints for linearization of objective function
        m.addConstrs(pe[f, k, s] >= e[f, k, s] - D[f, k]  for f in F )


        # Constraints for harvesting
        m.addConstrs(z.sum(h, '*', p, k, s) <= 1 for h in H for p in P )
        m.addConstrs(z.sum('*', f, p, k, s) <= M[f] * x[f, p, k, s] for f in F for p in P )
        m.addConstrs(z[h, f, p, k, s] >= alpha[h, f, p, k, s] for h in H for f in F for p in P )
        m.addConstrs(z[h, f, p, k, s] <= alpha[h, f, p, k, s] + 0.999 for h in H for f in F for p in P )
        m.addConstrs(z[h, f, p, k, s] <= u[h, k] for h in H for f in F for p in P )
        m.addConstrs(z[h, f, p, k, s] <= x[f, p, k, s] for h in H  for f in F for p in P  )
        m.addConstrs(x[f, p, k, s] <= z.sum('*', f, p, k, s) for f in F for p in P )

        m.addConstrs(M_cap * u[h, k] >= quicksum(alpha.sum(h, f, '*', k, s) * A[f] for f in F) for h in H )
        m.addConstrs(alpha.sum('*', f, '*', k, s) == I[f,k] for f in F )
        m.addConstrs(x[f, p, k, s] == 0 for f in F  for p in range(R[f, k]))
        m.addConstrs(x[f, p, k, s] <= I[f, k] for f in F for p in P)
        m.addConstrs(x.sum(f, '*', k, s) >= I[f, k] for f in F )

        # Constraints for harvesting time period
        m.addConstrs(R[f, k] <= st[f, k, s] for f in F )
        m.addConstrs(st[f, k, s] <= p + len(P) * (1 - x[f, p, k, s]) for f in F for p in P )
        m.addConstrs(p * x[f, p, k, s] <= e[f, k, s] for f in F for p in P )
        m.addConstrs(st[f, k, s] <= e[f, k, s]  for f in F)
        m.addConstrs(x.sum(f, '*', k, s) <= e[f, k, s] - st[f, k, s] + 1 for f in F )

        # Constraints for routing
        m.addConstrs(u[h - 1, k] >= u[h, k] for h in H[1:] ) # choose smaller harvesters' indices with priority

        m.addConstrs(y.sum(h, 0, loc, '*', k, s) == u[h, k] for h in H)
        m.addConstrs(y.sum(h, len(P) - 1, '*', loc, k, s) == u[h, k] for h in H)

        m.addConstrs(y.sum(h, p - 1, '*', f, k, s) == y.sum(h, p, f, '*', k, s) for h in H for f in F for p in P[1:]) 

        m.addConstrs(z[h, f, p, k, s] <= y.sum(h, p, f, '*', k, s) for h in H for f in F for p in P)
        m.addConstrs(y.sum(h, p, '*', '*', k, s) <= u[h, k] for h in H for p in P)
        m.addConstrs(y.sum(h, p, f, '*', k, s) <= I[f, k] for h in H for f in F[:loc] + F[loc + 1:] for p in P )
        m.addConstrs(y.sum(h, p, '*', '*', k, s) <= 1 for h in H for p in P)

        # Constraint for uncertainty and available crop
        m.addConstrs(In[0 , s, f, k] == 0 for f in ins[k])
        m.addConstrs(quicksum(alpha[h, f, p, k, s] for h in H)* G[f, s, k] + q[p , s, f, k] >= de[f, p, k]  for p in P[1:] for f in ins[k])
        m.addConstrs(In[p , s, f, k] >= quicksum(alpha[h, f, p, k, s] for h in H) * G[f, s, k] - de[f, p, k] + In[p - 1, s, f, k] for p in P[1:] for f in ins[k])
        m.optimize()
        zz[k,s] = m.objVal
        print(zz[k,s])

   # Outputs
file = open('hh10stage.txt', 'w')

# output the optimal harvest time for each field
align = '{:^10} {:^10} {:^10} {:^10} {:^10}'
# starting and ending period

file.write(align.format('Season', 'Scenario', 'Field', 'Start_time', 'End_time',' '))
file.write('\n')
for k in K:
    for s in range(S):
        for f in ins[k]:
            file.write(align.format(k, s, f, int(st[f, k, s].x), int(e[f, k, s].x), ' '))
            file.write('\n')


# output the number of needed harvesters in each harvest seasons
align = '{:^10} {:^10} {:^15}'
file.write(align.format('Season','scenario', '#_of_harvesters', ' '))
file.write('\n')
for k in K:
    num = 0
    for h in H:
        if u[h, k].x > 0.001:
            num += 1
    file.write(align.format(k,s, num, ' '))
    file.write('\n')


# output how much to harvest in each field
align = '{:^10} {:^10} {:^10} {:^10} {:10} {:10}'
file.write(align.format('Season','scenario','Field', 'Harvester', 'period', 'Proportion',' '))
file.write('\n')
align = '{:^10} {:^10} {:^10} {:^10} {:^10} {:^10.2f}'
for k in K:
    for f in ins[k]:
        for h in H:
            for s in range(S):
                for p in P:
                    if alpha[h, f, p, k, s].x > 0.001:
                        file.write(align.format(k, s, f, h, p, alpha[h, f, p, k, s].x,' '))
                        file.write('\n')



# output the machinery scheduling in this harvest plan
align = '{:^10} {:^10} {:^10} {:^10} {:<10}'
file.write(align.format('Season', 'Harvester', 'Time','scenario', 'Route',' '))
file.write('\n')
for k in K:
    for h in H:
        for p in P:
            for i in ins[k]:
                 for j in ins[k]:
                    for s in range(S):
                        if y[h, p, i, j, k, s].x > 0.001:
                            file.write(align.format(k, h,  p, s, str(i)+' --> '+str(j)))
                            file.write('\n')
    
    
# output the remained harvested crop in this harvest plan
align = '{:^10} {:^10} {:^10} {:^10} {:<10}'
file.write(align.format('Season', 'period', 'farm','scenario', 'remained crop',' '))
file.write('\n')
for k in K:
    for p in P:
        for f in ins[k]:
            for s in range(S):
                if (In[p , s, f, k].x > 0 ):
                    file.write(align.format(k, p, f, s, In[p , s, f, k].x,' '))
                    file.write('\n')
    
# output the shortage and remained harvested crop in this harvest plan
align = '{:^10} {:^10} {:^10} {:^10} {:<10}'
file.write(align.format('Season', 'period', 'farm','scenario', 'shortage crop',' '))
file.write('\n')
for k in K:
    for p in P:
        for f in ins[k]:
            for s in range(S):
                for h in H:
                    if (alpha[h, f, p, k, s].x  > 0.01 ):
                        if (q[p , s, f, k].x > 0 ): 
                            file.write(align.format(k, p, f, s, round(q[p , s, f, k].x,2),' '))
                            file.write('\n')

file.write('Running time: {:.2f}s'.format(m.Runtime))
file.write('\n')
file.write('Total cost($1000): {:.2f}'.format(m.objVal/1000))
file.write('\n')

#outputs rent cost
file.write('expected rent costs for every season')
file.write('\n')
rentcosts=0
totalrent=0

for k in K:
    file.write(str(rentcosts))
    file.write('\n')
    rentcosts=0
    for f in ins[k]:
        for h in H:
            for s in range(S):
                for p in P:
                    rentcosts += C_h * u[h,k].x
                    #print(rentcosts) 
                    totalrent += C_h * u[h,k].x
file.write(str(rentcosts))
file.write('\n')
file.write('expected total rent cost is')
file.write('\n')
file.write(str(totalrent))
#outputs operation cost
file.write('\n')
file.write('expected operation costs for every season')
file.write('\n')
operationcost=0
totaloperationcosts=0

for k in K:
    file.write(str(operationcost))
    file.write('\n')
    operationcost=0
    for f in ins[k]:
        for h in H:
            for s in range(S):
                for p in P:
                    operationcost += p_scen[s]* C_o * A[f]* alpha[h, f, p, k, s].x
                    #print(rentcosts) 
                    totaloperationcosts += p_scen[s]* C_o * A[f]* alpha[h, f, p, k, s].x
file.write(str(operationcost))
file.write('\n')
file.write('expected total operation cost is')
file.write('\n')
file.write(str(totaloperationcosts))

#outputs transportation costs
file.write('\n')
file.write('expected transportation costs for every season')
file.write('\n')
transportationcost=0
totaltransportationcosts=0

for k in K:
    file.write(str(transportationcost))
    file.write('\n')
    transportationcost=0
    for i in ins[k]:
        for j in ins[k]:
            for h in H:
                for s in range(S):
                    for p in P:
                        transportationcost += p_scen[s]* C_r * d[i][j]* y[h,p,i,j,k,s].x
                        #print(rentcosts) 
                        totaltransportationcosts += p_scen[s]* C_r * d[i][j]* y[h,p,i,j,k,s].x
file.write(str(transportationcost))
file.write('\n')
file.write('expected total transportation cost is')
file.write('\n')
file.write(str(totaltransportationcosts))

#outputs penalty costs
file.write('\n')
file.write('expected penalty costs for every season')
file.write('\n')
penaltycost=0
totalpenaltycosts=0

for k in K:
    file.write(str(penaltycost))
    file.write('\n')
    penaltycost=0
    for f in ins[k]:
        for s in range(S):
            penaltycost += p_scen[s]* C_penalty * pe[f, k, s].x * A[f] * I[f, k]
                #print(rentcosts) 
            totalpenaltycosts += p_scen[s]* C_penalty * pe[f, k, s].x * A[f] * I[f, k]
file.write(str(penaltycost))
file.write('\n')
file.write('expected total penalty cost is')
file.write('\n')
file.write(str(totalpenaltycosts))
file.write('\n')
#outputs inventory costs
file.write('expected inventory costs for every season')
file.write('\n')
inventorycost=0
totalinventorycosts=0

for k in K:
    file.write(str(inventorycost))
    file.write('\n')
    inventorycost=0
    for f in ins[k]:
        for s in range(S):
            for p in P:
                inventorycost += p_scen[s] * C_In * In[p, s, f, k].x
                #print(rentcosts) 
                totalinventorycosts += p_scen[s] * C_In * In[p, s, f, k].x
file.write(str(inventorycost))
file.write('\n')
file.write('expected total inventory cost is')
file.write('\n')
file.write(str(totalinventorycosts))
file.write('\n')
#outputs shortage costs
file.write('\n')
file.write('expected shortage costs for every season')
file.write('\n')
shortagecost=0
totalshortagecosts=0

for k in K:
    file.write(str(shortagecost))
    file.write('\n')
    shortagecost=0
    for f in ins[k]:
        for s in range(S):
            for p in P:
                shortagecost += p_scen[s]* C_sh *q[p, s, f, k].x
                #print(rentcosts) 
                totalshortagecosts += p_scen[s]* C_sh *q[p, s, f, k].x
file.write(str(shortagecost))
file.write('\n')
file.write('expected total shortage cost is')
file.write('\n')
file.write(str(totalshortagecosts))
file.write('\n')
file.write('inv cost and shortagecost')
file.write('\n')
file.write(str(C_In))
file.write('\n')
file.write(str(C_sh))
file.write('\n')
file.close()

