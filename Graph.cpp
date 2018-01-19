#include "Graph.h"
#include <iostream>

using namespace std;

#define passert(EXPR) do {if(!(EXPR)) {                                 \
        cerr << "Graph::checkInvariants failed: " << #EXPR << ", file " \
             << __FILE__ << ", line " << __LINE__ << endl;              \
        return false;                                                   \
        }                                                               \
    } while(0)

bool Graph::checkInvariants() const {
    size_t sizev = size();

    // matching coherence
    for(uint i = 0 ; i < size(); ++i){
        if(parents[i].isNone() && !matchings[i].isNone()){
            // matchings is involutive
            passert(matchings[matchings[i]].get() == i);
        }
    }

    return true;
}

void Graph::contract(std::vector<uint> oddCycle) {
    assert(checkInvariants());
    assert(oddCycle.size() % 2);
    auto matching = matchings[oddCycle[0]];
    uint current = size();
    cnodes.push_back(ContractedNode());
    parents.push_back(OptIndex());
    matchings.push_back(matching);
    for(uint i : oddCycle) {
        parents[i] = current;
    }
    if(!matching.isNone()){
        assert(matching < current);
        assert(count(ALL(oddCycle),matchings[matching]));
        matchings[matching] = current;
    }
    cnodes.back() = ContractedNode{move(oddCycle)};
    assert(checkInvariants());
}

bool Graph::augment() {
    cout << "augment " << endl;
    struct  DFS{
        enum class Mark{NotSeen, Before, After};
        vector<Mark> marked;
        Graph& g;
        OptIndex returnTo; // when contracting
        std::vector<uint> cycle; // when contracting
        DFS(Graph& g) : marked(g.size(), Mark::NotSeen), g(g){}
        enum class RetVal{Fail, Augment, Contract};

        RetVal exp(uint afterNode){
            cout << "Exploring after " << afterNode << endl;
            assert(marked[afterNode] == Mark::After);
            try {
                g.for_each_adj(afterNode,[this,afterNode](uint befNode){
                        befNode = g.getTopParent(befNode);
                        if(befNode == afterNode) return;
                        cout << "testing " << befNode << endl;
                        switch(marked[befNode]){
                            case Mark::Before:
                                return;
                            case Mark::After:
                                cout << afterNode << " goes to "
                                     << befNode << ": odd Cycle !" << endl;
                                returnTo = befNode;
                                cycle.push_back(afterNode);
                                throw RetVal::Contract;
                            default:
                                {}
                        }
                        if(g.matchings[befNode].isNone()){// found end of augmenting path
                            cout << "Reached end of augmenting cycle " << befNode << endl;
                            g.match(afterNode,befNode);
                            throw RetVal::Augment;
                        }
                        else {
                            cout << "following " << befNode << endl;
                            marked[befNode] = Mark::Before;
                            marked[g.matchings[befNode]] = Mark::After;
                            switch(exp(g.matchings[befNode])){
                                case RetVal::Fail:
                                    return;
                                case RetVal::Augment:
                                    cout << "Going back augmenting, linking " << afterNode
                                         << " and " << befNode;
                                    g.match(afterNode,befNode);
                                    throw RetVal::Augment;
                                case RetVal::Contract:
                                    cycle.push_back(befNode);
                                    cycle.push_back(afterNode);
                                    if(afterNode == returnTo){
                                        cout << "ending contraction in " << afterNode << endl;
                                        g.contract(move(cycle));
                                        cycle.clear(); // to be conformant with standard
                                        returnTo = OptIndex();
                                        marked.push_back(Mark::After);
                                        cout << "node " << g.size() -1 << " is built" << endl;
                                        throw exp(g.size()-1);
                                    }
                            }
                        }
                    });
            }
            catch(RetVal v){
                cout << "ending exploration of " << afterNode << endl;
                return v;
            }
            cout << "ending exploration of " << afterNode << " with failure" << endl;
            return RetVal::Fail;
        }
    };
    DFS dfs(*this);
    uint sizev = size();
    for(uint i = 0 ; i < sizev ; ++i){
        if(parents[i].isNone() && matchings[i].isNone()){
            dfs.marked[i] = DFS::Mark::After;
            switch (dfs.exp(i)){
                case DFS::RetVal::Contract:
                    assert(!"unreachable");
                    exit(1);
                case DFS::RetVal::Augment:
                    return true; // the graph has been augmented.
                case DFS::RetVal::Fail:
                    continue;
            }
        }
    }
    return false; // the graph cannot be augmented.
}

void Graph::printGraph(std::ostream& out, const std::string& s) const {
    assert(checkInvariants());
    out << "graph " << s << "{" << endl;
    for(uint i = size(); i != uint(-1) ; --i){
        if(parents[i].isNone()){
            out << i << endl;
            for_each_adj(i,[&](uint j){
                    if (i < j && matchings[i].data() != j)
                        out << i << " -- " << getTopParent(j) << endl;
                });
            if(!matchings[i].isNone()){
                if(i < matchings[i]) out << i << " -- " << matchings[i] << "[style=bold]" << endl;
            }
        }
    }
    out << "}" << endl;
}

void Graph::unfold() {
    for(uint i = size() -1 ; i > originalSize ; ++i){
        if(matchings[i].isNone()){
            
        }
    }
}
