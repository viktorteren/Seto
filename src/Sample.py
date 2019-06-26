import networkx as nx
import matplotlib.pyplot as plt
#READ THE DATA RELATED TO THE GRAPH -> REGIONS AND OVERLAPS BETWEEN REGIONS AND CREATE THE GRAPH WITH THE LIBRARY
f = open("Graph.dimacs", "r")
#print(f.readline())
str = f.readline().split()
num_vertices = str[1]
num_edges = str[2]


G = nx.Graph()
#set values from 1 to number of regions
G.add_nodes_from([1, str[1]])

#create edges
for x in f:
    temp = x.split()
    G.add_edge(temp[1], temp[2])

#plt.subplot(121)
#nx.draw(G, with_labels=False)

#G = nx.petersen_graph()
#plt.subplot(121)
#nx.draw(G, with_labels=True, font_weight='bold')
#plt.subplot(122)
#nx.draw_shell(G, nlist=[range(5, 10), range(5)], with_labels=True, font_weight='bold')

#FIND ALL FSMS (2 STEPS)
Reduced_G = G

first_FSM = nx.maximal_independent_set(G)
other_incomplete_FSMs = []

while (list(Reduced_G.nodes) > 0):
    FSM = nx.maximal_independent_set(Reduced_G)
    other_incomplete_FSMs.append(FSM)
    for node in FSM:
        Reduced_G.remove_node(node)

final_FSMs = []
final_FSMs.append(first_FSM)
for FSM in other_incomplete_FSMs:
    final_FSMs.append(nx.maximal_independent_set(G, FSM))

#WRITE A FILE WITH THE NUMBER OF FSMS AND THEIR NODES
f = open("final_FSMs.txt", "w")
#f.write(final_FSMs.count())
f.write("Woops! I have deleted the content!")
f.close()
