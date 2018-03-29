#include <boost/graph/edge_list.hpp>
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

typedef std::pair<int,int> Edge;

using namespace std;
using namespace boost;

typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;

int main()
{
    // Open the file:
    std::ifstream fin("input.txt");

    assert(fin);
// Declare variables:
    int num_stati, num_etichette, stato_iniziale;

// Read defining parameters:
    fin >> num_stati ;
    fin >> num_transazioni;
    fin >> stato_iniziale;

    cout << num_stati << endl;
    cout << num_etichette << endl;
    cout << stato_iniziale << endl;

    for(int i = 0; i < num_etichette;i++){

    }

    // declare a graph object
    Graph g(num_stati);

    typedef std::pair<int, int> Edge;
    Edge edge_array[] = new Edge[num_transazioni];


    //aggiungo gli archi al grafo
    for (int i = 0; i < num_transazioni; ++i)
        add_edge(edge_array[i].first, edge_array[i].second, g);

    fin.close();
}


/*int main () {
    string line;
    fstream myfile ("input.txt");
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            cout << line << '\n';
        }
        myfile.close();
    }
    else cout << "Unable to open file";

    return 0;
}*/

/*int main () {
    char data[100];

    // open a file in write mode.
    ofstream outfile;
    outfile.open("afile.dat");

    cout << "Writing to the file" << endl;
    cout << "Enter your name: ";
    cin.getline(data, 100);

    // write inputted data into the file.
    outfile << data << endl;

    cout << "Enter your age: ";
    cin >> data;
    cin.ignore();

    // again write inputted data into the file.
    outfile << data << endl;

    // close the opened file.
    outfile.close();

    // open a file in read mode.
    ifstream infile;
    infile.open("afile.dat");

    cout << "Reading from the file" << endl;
    infile >> data;

    // write the data at the screen.
    cout << data << endl;

    // again read the data from the file and display it.
    infile >> data;
    cout << data << endl;

    // close the opened file.
    infile.close();

    return 0;
}*/