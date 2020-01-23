# This file is subject to the terms and conditions defined in
# file 'LICENSE.txt', which is part of this source code package.

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
#Reduced_G.add_nodes_from(range(1, int(num_vertices)+1))

#create edges
for x in f:
    temp = x.split()
    G.add_edge(int(temp[1]), int(temp[2]))
    #Reduced_G.add_edge(int(temp[1]), int(temp[2]))

f.close()

final_SETs = []
recursively_called = []
to_call = []

def call():
    Gr = to_call.pop(0)
    recursively_called.append(Gr)
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
        for vertex in Gr:
            G = nx.Graph()
            G.add_nodes_from(Gr)
            for edge in Gr.edges:
                if edge[0] != vertex and edge[1] != vertex:
                    G.add_edge(int(edge[0]), int(edge[1]))
            G.remove_node(vertex)
            exit = False
            for elem in recursively_called:
                if list(G) == list(elem):
                    exit = True
                    break
            if not exit:
                to_call.append(G)

to_call.append(G)
while len(to_call) > 0:
    call()

#WRITE A FILE WITH THE NUMBER OF FSMS AND THEIR NODES
f = open("final_FSMs.txt", "w")
f.write(str(len(final_SETs)))
f.write('\n')
for FSM in final_SETs:
    for region in FSM:
        f.write(str(region)+' ')
    f.write('\n')

f.close()


