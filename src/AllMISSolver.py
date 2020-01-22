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

def recursive_call(graph, vertex):
    #print("called with vertex")
    #print(vertex)
    if graph.number_of_nodes() > 1:
        Gr = nx.Graph()
        Gr.add_nodes_from(graph)
        #print("edges:")
        #print(list(graph.edges))
        for edge in graph.edges:
            if edge[0] != vertex and edge[1] != vertex:
                Gr.add_edge(int(edge[0]), int(edge[1]))
        #print("Edges of Gr")
        #print(list(Gr.edges))
        #print("Graph size:")
        #print(Gr.number_of_nodes())
        #print(list(Gr.nodes))
        Gr.remove_node(vertex)
        #print("After the removal")
        #print(list(Gr.nodes))
        try:
            SM = nx.maximal_independent_set(Gr, Gr)
            #print("Found valid SM")
            #print(list(SM))
            if not any([ SM in final_SETs]):
                final_SETs.append(SM)
        except nx.NetworkXUnfeasible:
            #print("Not valid SM")
            for vertex in Gr:
                recursive_call(Gr, vertex)

def first_call(graph):
    for vertex in graph:
        recursive_call(graph, vertex)

first_call(G)

#WRITE A FILE WITH THE NUMBER OF FSMS AND THEIR NODES
f = open("final_FSMs.txt", "w")
f.write(str(len(final_SETs)))
f.write('\n')
for FSM in final_SETs:
    for region in FSM:
        f.write(str(region)+' ')
    f.write('\n')

f.close()


