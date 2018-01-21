#include "Graph.h"
#include <iostream>

using namespace std;

int main(){
    cout.sync_with_stdio(false);
    cin.sync_with_stdio(false);
    uint n,m;
    cin >> n >> m;
    Graph g(n);

    for(uint i = 0 ; i < m ; ++i){
        uint n1,n2;
        cin >> n1 >> n2;
        g.addEdge(n1, n2);
    }

    g.printGraph(cerr, "Ginit");
    while(g.augment());
    cout << "finished !!" << endl;
    g.printGraph(cerr, "Gend");
    //cout << g.unmatched() << endl;
}
