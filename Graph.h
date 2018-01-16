#include <vector>
#include <set>

class Graph{
    class Node {
        std::vector<int> cycle;
    };
    std::vector<Node*> nodes;
    std::vector<std::set<int>> adj;
    std::vector<int> parents; // minus one for nothing
    std::vector<int> matchings; // minus one for nothing

};


