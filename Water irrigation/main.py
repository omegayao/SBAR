# import modules
import numpy as np
from gurobipy import *

# <----------------------- Read Data ----------------------->
#path = (r"/Users/mehdi/Documents/irrigation/irrigation.txt")
C_p = {}
C_fhi = {}
C_s = {}
C_sh = {}
C_wl = {}
C_fi = {}
gamma = {}
P = {}
ETm = {}
Nr = {}
De = {}
bb = {}
K = {}
C_w ={}
K[1]= 0.5
K[2]= 0.78
K[3]= 0.94
Mn=1000
ETm22= {}
#with open(path) as f:
with open('/home/u20/mahdimahdavi/irrigation4.txt', 'r') as f:
 # Sets and indices
    F = list(range(1, int(f.readline()) + 1))  # fields
    C = list(range(1, int(f.readline()) + 1))  # crops
    T = list(range(1, int(f.readline()) + 2))  # growing seasons
    L = list(range(1, int(f.readline()) + 1))  # defecit levels
    I = list(range(1, int(f.readline()) + 1))  # Irrigation methods 
    J = list(range(1, int(f.readline()) + 1))  # Irrigation district 


    for c in C:
        C_p[c] = float(f.readline())  # operational cost
    for c in C:
        C_fhi[c] = float(f.readline())  # fertlizer cost
    for c in C:
        C_s[c] = float(f.readline())  # seed cost    
    for c in C:
        C_sh[c] = float(f.readline())  # operational cost
    C_w = float(f.readline())  # labour cost per acre
    C_wl = float(f.readline())  # watersupply cost per acre-foot
    for i in I:
        C_fi[i] = float(f.readline())  # waterirrigation setup cost per acre
    for i in I:
        gamma[i] = float(f.readline())
    for c in C:
        P[c] = float(f.readline())
    for c in C:
        ETm[c] = float(f.readline())    # acre-foot
    for c in C:
        bb[c] = int(f.readline())
    for c in C:    
        Nr[c] = 1    
    A = [0, 216.83, 246.78, 171.9, 194.53, 194.95, 211.25, 211.42, 248, 178.58, 221.32, 203.24, 185.48, 163.13, 182.14, 237.44, 246.8, 161.46, 250, 247, 138.63, 190.7, 220.17, 219.73, 249, 220.07, 217.84, 192.87, 171.24, 249.26, 247, 234.91, 221.39, 245.07, 249.91, 235.4, 248, 245.73]
    
    #A = [0] + list(map(float, f.readline().split()))  # acres for fields & 29 is the default facility location
D={}
de = np.zeros([len(T)+2,len(C)+1])

for t in T:
        de[t, 1]= 7500
        de[t, 2]= 5707
        de[t, 3]= 5707

landa= 0.06
Innf = 0.005
Inns = 0.02   
for l in L:
    D[l]=l/len(L)
Ym={}    
Ym[1]=12
Ym[2]=1.7     #1.7
Ym[3]=2.8      #2.8
le={}
for f in F:
    le[f]=100
B=10000

# Create the model
# solver settings
m = Model('multistageirrigation')
# m.params.LogFile = ('Harvest.log')
# setParam("LogToConsole", 0)
m.ModelSense = GRB.MAXIMIZE
#m.Params.MIPGap = 0.05    # 1%
# m.Params.TimeLimit = 300  # 5 minutes
m.setParam('Threads', 94)
# scenario tree for multistage programming 
# number of drought scenario in every harvest season k 
num_drought = 3
# probability of drought scenarios
o = [0.6, 0.3, 0.1] 
# number of scenarios
S = num_drought**len(T) 
# generating scenarios
#y_scen = np.zeros([G, S])
Wa = np.zeros([len(T)+1,len(J)+1, S])
p_scen = np.zeros([S])
xx = np.zeros([len(F)+1,len(C)+1])

# probability of each scenario
for s in range(S):
    p_scen[s] = o[int(math.floor(s/(num_drought**(4))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(3))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(2))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(1))))%num_drought]*\
                o[int(math.floor(s/(num_drought**(0))))%num_drought]    
# calculating the stochastic parameters
# o[int(math.floor(s/(num_drought**(5))))%num_drought]*\
 #o[int(math.floor(s/(num_drought**(4))))%num_drought]*\
for s in range (S):
    Wa[0,1,s]= 853150 #acre-foot
for s in range (S):
    for t in T:
        for j in J:
            if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==0:
                Wa[t,j ,s] = Wa[t-1,j ,s]* 1
            if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==1:
                Wa[t,j ,s] = Wa[t-1,j ,s]* 0.8
            if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==2:
                Wa[t,j ,s] = Wa[t-1,j ,s]* 0.6  
                
R = {}
for s in range (S):
    for f in F:
        R[f,0, s]= 0.5 * A[f]  #m3
for s in range (S):
    for t in T:
        for j in J:
            for f in F:
                if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==0:
                    R[f, t, s] = R[f, 0, s]* 1
                if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==1:
                    R[f, t, s] = R[f, 0, s]* 0.85
                if (int(math.floor(s/(num_drought**((len(T))-t))))%num_drought)==2:
                    R[f, t, s] = R[f, 0, s]* 0.7

# Variables
u = m.addVars([(f,c, i, t, s) for c in C for f in F for i in I for t in T for s in range(S)], vtype=GRB.CONTINUOUS, name='u')
n = m.addVars([(f, i, t, s) for f in F for i in I for t in T for s in range(S)], vtype=GRB.BINARY, name='n')
x = m.addVars([(f, c, t) for f in F for c in C for t in T], vtype=GRB.BINARY, name='x')
z = m.addVars([(f,c, l, t, s) for f in F for c in C for l in L for t in T for s in range(S)], vtype=GRB.BINARY, name='z')
w = m.addVars([(f, i, t, s) for f in F for i in I for t in T for s in range(S)], vtype=GRB.BINARY, name='w')
#q = m.addVars([(c, t, s) for c in C for t in T for s in range(S)], lb=0, vtype=GRB.CONTINUOUS,
                 # name='q')
y = m.addVars([(c, f, l, t, s) for c in C for f in F for l in L for t in T for s in range(S) ], lb=0, vtype=GRB.CONTINUOUS,
                  name='y')
ETa= m.addVars([(c, f, t,s) for c in C for f in F for t in T for s in range(S) ], lb=0, vtype=GRB.CONTINUOUS,
                  name='eta')

# Constraints for deficit irrigation
m.addConstrs((ETa[c, f, t, s] / ETm[c]) >= (quicksum (D[l] * z[f,c, l,t,s] for l in L)) for f in F for t in T for c in C for s in range(S))

m.addConstrs(y[c, f, l, t, s] - (x[f, c, t]* Ym[c]* (1-K[c]*(1-D[l]))) <= Mn* ( 1- z[f, c, l,t,s] ) for f in F for l in L for c in C for s in range(S) for t in T)
m.addConstrs(y[c, f, l, t, s] - (x[f, c, t]* Ym[c]* (1-K[c]*(1-D[l]))) >= Mn* ( z[f, c, l,t,s] - 1 ) for f in F for l in L for c in C for s in range(S) for t in T)
m.addConstrs(quicksum (z[f,c, l,t,s] for l in L) == 1 for f in F for c in C for t in T for s in range(S))
m.addConstrs(y[c, f, l, t, s] <= Mn * z[f,c, l,t,s] for l in L for f in F for c in C for s in range(S) for t in T)
m.addConstrs(y[c, f, l, t, s] <= x[f, c, t]* Ym[c] for l in L for f in F for c in C for s in range(S) for t in T)
m.addConstrs(u[f,c, i, t, s] <= Mn * z[f,c, l,t,s] for l in L for f in F for c in C for s in range(S) for t in T)

# Constraints for water irrigation balance
m.addConstrs(R[f, t, s] + 0.9* quicksum (gamma[i] * u[f,c, i, t, s] for i in I) - ETa[c, f, t, s]* A[f] == 0 for c in C for f in F for t in T for s in range(S))
m.addConstrs(R[f, t, s] + 0.9* quicksum (gamma[i] * u[f,c, i, t, s] for i in I) <= 1000000000 for c in C for f in F for t in T for s in range(S))

# Constraints for water availibility
m.addConstrs(quicksum( (u[f,c, i, t, s]/ gamma[i]) for i in I for f in F) <= Wa[t,1,s] for c in C for s in range(S) for t in T)
m.addConstrs(u[f,c, i, t, s] <= Mn * w[f, i, t, s] for c in C for f in F for i in I for s in range(S) for t in T)
m.addConstrs(quicksum (w[f, i, t, s] for i in I) == 1 for f in F  for t in T for s in range(S))
m.addConstrs(n[f, i, t, s] >= 0 for f in F for i in I for s in range(S) for t in T)
m.addConstrs(n[f, i, 1 , s] >= 1 for f in F for i in I for s in range(S))
m.addConstrs(w[f, i, t+1, s] - w[f, i, t, s] <= n[f, i, t, s] for f in F for i in I for s in range(S) for t in T[:2])

# Constraints for crop rotation
m.addConstrs(quicksum(x[f, c, t] for c in C) == 1 for f in F for t in T)
#m.addConstrs(o[c]* x[f, c, 1] - quicksum (x[f, c, t] for t in range(1,bb[c])) <= 0 for f in F for c in C)

#m.addConstrs(quicksum(x[f, c, ttt] for ttt in range (1, t+bb[c])) >= bb[c]*(x[f, c, t] - x[f, c, t-1]) for f in F for c in C for t in range(2, len(T) - bb[c] + 1))
#m.addConstrs(quicksum(x[f, c, ttt] - (x[f, c, t] - x[f, c, t-1]) for ttt in T)>= 0 for f in F for c in C for t in range(len(T)-l[c]+1, len(T)))
             
#m.addConstrs(quicksum(x[f, c, t] - (x[f, c, t] - x[f, c, t-1]) for t in T) >= 0 for f in F for c in C for t in T)
#m.addConstrs(quicksum(x[f, c, ttt] - (x[f, c, t] - x[f, c, t-1]) for ttt in T) >= 0 for f in F for c in C for t in range(len(T)-l[c]+1, len(T)))
#m.addConstrs(quicksum(x[f, c, ttt] for ttt in T) <= Nr[c]*l[c] for f in F for c in C for t in T[1:Nr[c]*l[c]])
#m.addConstrs(y[c, f, l, t, s] <= Mn * x[f, c, t] for t in T for c in C for f in F for s in range(S) for l in L)

m.addConstrs(quicksum(y[c, f, l, t, s] * A[f] for l in L for f in F) >= de[t,c] for t in T for c in C for s in range(S))

# Constraint (4)
for c in C:
    for t in T:
        for f in F:
            Const_4 = quicksum(x[f, c, t] for c in C)
            m.addConstr(Const_4 == 1)
            Const_4 = None

# Constraint (5a)
for c in C:
    for f in F:
        Const_5a = bb[c] * x[f, c, 1]
        for t in range(1, bb[c] + 1):
            Const_5a -= x[f, c, t]
        m.addConstr(Const_5a <= 0)
        Const_5a = None

# Constraint (5b)
for c in C:
    for f in F:
        for t in range(2, len(T) - bb[c] + 2):
            SumUp5b = quicksum(x[f, c, t2] for t2 in range(t, t + bb[c]))
            Const_5b = SumUp5b - (bb[c] * (x[f, c, t] - x[f, c, t - 1]))
            m.addConstr(Const_5b >= 0)
            SumUp5b = None
            Const_5b = None

# Constraint (5c)
for c in C:
    for f in F:
        for t in range(len(T) - bb[c] + 2, len(T) + 1):
            Const_5c = quicksum(x[f, c, t2] - (x[f, c, t] - x[f, c, t-1]) for t2 in range(t, len(T) + 1))
            m.addConstr(Const_5c >= 0)
            Const_5c = None


for t in range(1, len(T) - (Nr[c] * bb[c]) + 1):
    m.addConstrs(x[f, c, t2] <= bb[c] * Nr[c] for c in C for f in F for t2 in range(t, t + (bb[c] * Nr[c]) + 1))

#m.addConstrs(x[f, c, t] + x[f, c, t+1] == 1 for c in C[2:] for f in F for t in range(1, len(T)))
    
for t in range(1,5):
    for s in range(0,729):
        for s1 in range(0,729):
            if (Wa[t,1,s] == Wa[t,1,s1] and s!=s1 and s1>s):
                m.addConstrs(u[f,c, i, t, s]  == u[f,c, i, t, s1] for f in F for i in I)
               # m.addConstrs(z[f,c, l, t, s]  == z[f,c, l, t, s1] for f in F for l in L for c in C)
                #m.addConstrs(q[c, t, s] == q[c, t, s1] for c in C)
              
# Solve the model
m.optimize()

file = open('irrigation output4newnewstage.txt', 'w')

# output the optimal irrigation water amount for each field
                
align = '{:^10} {:^10} {:^10} {:^20} {:^15} {:^10}'

file.write(align.format('Seasons', 'Scenario', 'Field', 'Irrigation Method', 'Water Amount', ' '))
file.write('\n')

for t in T:
    for s in range(0, 81):
        for f in F:
            for i in I:
                for c in C:
                    if u[f,c, i, t, s].x > 0:
                        file.write(align.format(str(t), str(s), str(f), str(i), str(u[f,c, i, t, s].x), ' '))
                        file.write('\n')
file.write(align.format('Seasons', 'Scenario', 'Field', 'crop', 'deficit', 'yield', ' '))
file.write('\n')

for t in T:
    for s in range(0, 81):
        for f in F:
            for c in C:
                for l in L:
                    if y[c, f, l, t, s].x > 0:
                        file.write(align.format(str(t), str(s), str(f), str(c), str(l), str(y[c, f, l, t, s].x), ' '))
                        file.write('\n')
                        
# output the planting of crops in each harvest seasons
align = '{:^10} {:^10} {:^15} {:^15}'
file.write(align.format('Seasons', 'crop', 'farm', 'planted or not', ' '))
file.write('\n')
for t in T:
    for c in C:
        for f in F:
            if x[f, c, t].x > 0 :
                file.write(align.format(t, c, f, x[f, c, t].x, ' '))
                file.write('\n')


# output how much to harvest in each field
align = '{:^10} {:^10} {:^10} {:^10} {:^10}'
file.write(align.format('Seasons','scenario','Field', 'irrigation method','used or not',' '))
file.write('\n')
for t in T:
    for f in F:
        for i in I:
            for s in range(0,81):
                if w[f, i, t, s].x > 0:
                    file.write(align.format(t, s, f, i,  w[f, i, t, s].x,' '))
                    file.write('\n')

# output the shortage and remained harvested crop in this harvest plan
#align = '{:^10} {:^10} {:^10} {:^10} {:<10}'
#file.write(align.format('Seasons','crop','scenario', 'shortage crop',' '))
#file.write('\n')
#for t in T:
 #   for c in C:
  #      for s in range(0,81):
   #         if (q[c, t, s].x):
    #            file.write(align.format(t, c , s, round(q[c, t, s].x,2),' '))
     #           file.write('\n')

file.write('Running time: {:.2f}s'.format(m.Runtime))
file.write('\n')
file.write('Total cost($1000): {:.2f}'.format(m.objVal/1000))
file.write('\n')

#outputs revenue
file.write('expected revenue for every season')
file.write('\n')
Revenue=0
totalrevenue=0
for t in T:
    file.write(str(Revenue))
    file.write('\n')
    Revenue=0
    for f in F:
        for c in C:
            for s in range(0,81):
                for l in L:
                    Revenue += P[c]* A[f] * y[c, f, l, t, s].x * (1+ Inns)**(t+1)
                    totalrevenue += P[c]* A[f] * y[c, f, l, t, s].x * (1+ Inns)**(t+1)
file.write(str(Revenue))
file.write('\n')
file.write('expected total revenue is')
file.write('\n')
file.write(str(totalrevenue))

#outputs operation cost
file.write('\n')
file.write('operation costs for every season')
file.write('\n')
operationcost=0
totaloperationcosts=0

for t in T:
    file.write(str(operationcost))
    file.write('\n')
    operationcost=0
    for f in F:
        for c in C:
            operationcost += x[f, c, t].x * A[f] * C_p[c]
            #print(rentcosts) 
            totaloperationcosts += x[f, c, t].x * A[f] * C_p[c]
file.write(str(operationcost))
file.write('\n')
file.write('expected total operation cost is')
file.write('\n')
file.write(str(totaloperationcosts))

#outputs water costs
file.write('\n')
file.write('expected water costs for every season')
file.write('\n')
watercost=0
totalwatercosts=0

for t in T:
    file.write(str(watercost))
    file.write('\n')
    watercost=0
    for f in F:
        for i in I:
            for s in range(0,81):
                for c in C:
                    watercost += p_scen[s] * (C_wl * (u[f, c, i, t, s].x / gamma[i])) + C_fi[i] * A[f] * n[f, i, t, s].x + C_w * A[f] * x[f, c, t].x
                     #print(rentcosts) 
                    totalwatercosts += p_scen[s] * (C_wl * (u[f, c, i, t, s].x / gamma[i])) + C_fi[i]* A[f]  * n[f, i, t, s].x + C_w * A[f] * x[f, c, t].x
file.write(str(watercost))
file.write('\n')
file.write('expected total water cost is')
file.write('\n')
file.write(str(totalwatercosts))



#outputs shortage costs
file.write('\n')
file.write('expected seedfert costs for every season')
file.write('\n')
seedfertcost=0
totalseedfertcost=0

for t in T:
    file.write(str(seedfertcost))
    file.write('\n')
    shortagecost=0
    for c in C:
        for s in range(0,81):
            seedfertcost += (C_s[c] + C_fhi[c]) * x[f, c, t].x * A[f]
            #print(rentcosts) 
            totalseedfertcost += (C_s[c] + C_fhi[c]) * x[f, c, t].x * A[f]
file.write(str(seedfertcost))
file.write('\n')
file.write('expected total seedfertcost cost is')
file.write('\n')
file.write(str(totalseedfertcost))
file.write('\n')

