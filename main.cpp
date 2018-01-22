#include "Graph.h"
#include <iostream>

//Only for testing purposes
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

using namespace std;

uint boostMatchSize(const vector<vector<uint>>& adj) {
    using namespace boost;
    typedef adjacency_list<vecS, vecS, undirectedS> boost_graph;
    boost_graph g(adj.size());
    for(uint i = 0; i < adj.size(); ++i) {
        for(uint j : adj[i]) {
            add_edge(i, j, g);
        }
    }

    std::vector<graph_traits<boost_graph>::vertex_descriptor> mate(adj.size());

    clock_t t = clock();
#if 1
    bool success = checked_edmonds_maximum_cardinality_matching(g, &mate[0]);
    assert(success);
#else
    edmonds_maximum_cardinality_matching(g, &mate[0]);
#endif
    t = clock() - t;
    cout << "Boost time: " << double(t)/CLOCKS_PER_SEC << endl;

    return matching_size(g, &mate[0]);
}

int main(){
    cout.sync_with_stdio(false);
    cin.sync_with_stdio(false);
    uint n,m;
    cin >> n >> m;
    Graph g(n);
    vector<vector<uint>> adj(n);

    for(uint i = 0 ; i < m ; ++i){
        uint n1,n2;
        cin >> n1 >> n2;
        g.addEdge(n1, n2);
        adj[n1].push_back(n2);
        adj[n2].push_back(n1);
    }

    LOG(g.printGraph(cerr, "Ginit"));
    clock_t t = clock();
    uint greedySize = g.greedyMatch();
    while(g.augment())
        ;
    t = clock() - t;
    cout << "Time: " << double(t)/CLOCKS_PER_SEC << endl;
    LOG(g.printGraph(cerr, "Gend"));
    if(!g.checkInvariants()){
        cout << "Wrong invariants, you should enable assertion and debug" << endl;
    }
    uint matchingSize = (adj.size()-g.unmatched())/2;
    cout << "Matching size after greedy: " << greedySize << endl;
    cout << "Matching size: " << matchingSize << endl;
    cout << "Unmatched nodes: " << g.unmatched() << endl;
    if(boostMatchSize(adj) != matchingSize) {
        cout << "Errors in matching, you should enable assertion and debug" << endl;
    }
}
