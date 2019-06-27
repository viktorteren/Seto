import networkx as nx
import copy
#READ THE DATA RELATED TO THE GRAPH -> REGIONS AND OVERLAPS BETWEEN REGIONS AND CREATE THE GRAPH WITH THE LIBRARY
f = open("Graph.dimacs", "r")
line = f.readline().split()
num_vertices = line[1]
num_edges = line[2]

G = nx.Graph()
Reduced_G = nx.Graph()
#set values from 1 to number of regions
G.add_nodes_from(range(1, int(num_vertices)+1))
Reduced_G.add_nodes_from(range(1, int(num_vertices)+1))

#create edges
for x in f:
    temp = x.split()
    G.add_edge(int(temp[1]), int(temp[2]))
    Reduced_G.add_edge(int(temp[1]), int(temp[2]))

f.close()

#FIND ALL FSMS (2 STEPS)
#Reduced_G = copy.copy(G)

first_FSM = nx.maximal_independent_set(G)
other_incomplete_FSMs = []
for node in first_FSM:
    Reduced_G.remove_node(node)

while (len(list(Reduced_G.nodes)) > 0):
    FSM = nx.maximal_independent_set(Reduced_G)
    other_incomplete_FSMs.append(FSM)
    for node in FSM:
        Reduced_G.remove_node(node)

final_FSMs = []
final_FSMs.append(first_FSM)
for FSM in other_incomplete_FSMs:
    #FSM_int = map(int, FSM)
    final_FSMs.append(nx.maximal_independent_set(G, FSM))

#WRITE A FILE WITH THE NUMBER OF FSMS AND THEIR NODES
f = open("final_FSMs.txt", "w")
f.write(str(len(final_FSMs)))
f.write('\n')
for FSM in final_FSMs:
    for region in FSM:
        f.write(str(region)+' ')
    f.write('\n')

f.close()
