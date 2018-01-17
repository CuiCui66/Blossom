#include <iostream>
#include "Graph.h"

using namespace std;

int main(){
    uint n,m;
    cin >> n >> m;
    Graph g(n);

    for(uint i = 0 ; i < m ; ++i){
        uint n1,n2;
        cin >> n1 >> n2;
        g.addEdge(n1, n2);
    }
    while(g.augment());
    g.printGraph(cout, "G");
}
