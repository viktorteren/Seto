# This file is subject to the terms and conditions defined in
# file 'LICENSE.txt', which is part of this source code package.

import networkx as nx
#READ THE DATA RELATED TO THE GRAPH -> REGIONS AND OVERLAPS BETWEEN REGIONS AND CREATE THE GRAPH WITH THE LIBRARY
f = open("Graph.dimacs", "r")
line = f.readline().split()
num_vertices = int(line[1])
num_edges = line[2]
vertices = set()
for i in range(1, num_vertices):
    vertices.add(i)

G = nx.Graph()
#set values from 1 to number of regions
G.add_nodes_from(range(1, num_vertices+1))
#Reduced_G.add_nodes_from(range(1, int(num_vertices)+1))

#create edges
for x in f:
    temp = x.split()
    G.add_edge(int(temp[1]), int(temp[2]))
    #Reduced_G.add_edge(int(temp[1]), int(temp[2]))

f.close()

final_SETs = []
recursively_called = set()
to_call = []
#number_of_calls = 0

def call():
    Gr = to_call.pop(0)
    if frozenset(Gr.nodes) in recursively_called:
        return
    recursively_called.add(frozenset(Gr.nodes))
    #global number_of_calls
    #number_of_calls+=1
    #print("Called")
    #print(list(Gr))
    try:
        SM = nx.maximal_independent_set(Gr, Gr)
        #print("Found valid SM")
        #print(list(SM))
        exit = False
        for elem in final_SETs:
            if list(set(SM).intersection(elem)) == SM:
                exit = True
                break
        if not exit:
            final_SETs.append(SM)
            #print("Added")
            #print(SM)
    except Exception as e:
        #print("Not valid SM")
        #print(e)
        #print("recursively called content:")
        #print(recursively_called)
        for vertex in Gr:
            G = nx.Graph()
            G.add_nodes_from(Gr)
            G.add_edges_from(Gr.edges)
            G.remove_node(vertex) #automatically removes adjacent edges
            to_call.append(G)

to_call.append(G)
while len(to_call) > 0:
    call()

#print("Number of calls")
#print(number_of_calls)

#WRITE A FILE WITH THE NUMBER OF FSMS AND THEIR NODES
f = open("final_FSMs.txt", "w")
f.write(str(len(final_SETs)))
f.write('\n')
for FSM in final_SETs:
    for region in FSM:
        f.write(str(region)+' ')
    f.write('\n')

f.close()


