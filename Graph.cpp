#include "Graph.h"
#include <iostream>
#include <sstream>

using namespace std;

#define passert(EXPR) do {if(!(EXPR)) {                                 \
        cerr << "Graph::checkInvariants failed: " << #EXPR << ", file " \
             << __FILE__ << ", line " << __LINE__ << endl;              \
        return false;                                                   \
        }                                                               \
    } while(0)

bool Graph::checkInvariants() const {

    static uint val = 0;
    ++val;
    LOG({
            stringstream s;
            s << "G" << val;
            printGraph(cerr,s.str());
        })

    // edge reciprocity
    for(uint i = 0 ; i < size; ++i){
        for(uint adji : adj[i]){
            passert(count(ALL(adj[adji]),i));
        }
    }

    // matching coherence
    for(uint i = 0 ; i < size; ++i){
        if(!matchings[i].isNone()){
            // matchings is involutive
            passert(matchings[matchings[i]].get() == i);

            // matching is a real edge
            passert(count(ALL(adj[i]),matchings[i].get()));
        }
    }

    if(root.isNone()) return true; // not in augment.


    passert(preBef.size() == size);
    passert(preAft.size() == size);
    passert(marked.size() == size);
    passert(base.size() == size);

    for(uint i = 0 ; i < size ; ++i){
        if(i == root) continue;
        //cout << "checking " << i << endl;
        switch(marked[i]){
            case Mark::NotSeen:
                passert(preAft[i].isNone());
                passert(preBef[i].isNone());
                break;
            case Mark::After:
                passert(!preAft[i].isNone());
                passert(preBef[i].isNone());
                passert(followPathAft(i) == root);
                passert(matchings[i] == preAft[i]);
                break;
            case Mark::Before:
                passert(preAft[i].isNone());
                passert(!preBef[i].isNone());
                passert(followPathBef(i) == root);
                if( i != getTopParent(i)){
                    auto [u,v] = base[i];
                    passert(marked[getTopParent(u)] == Mark::After);
                    passert(getTopParent(u) == getTopParent(v));
                }
                break;
        }

    }

    for(auto i : preAft){
        if(!i.isNone()) passert(marked[i] == Mark::Before);
    }

    return true;
}

uint Graph::followPathAft(uint aftNode)const{
    if(preAft[aftNode].isNone()) return aftNode;
    else return followPathBef(preAft[aftNode]);
}
uint Graph::followPathBef(uint befNode)const{
    assert(!preBef[befNode].isNone());
    return followPathAft(getTopParent(preBef[befNode]));
}

bool Graph::expAft(uint aftNode) {
    LOG(cout << "exploring after node " << aftNode << endl);
    assert(marked[getTopParent(aftNode)] == Mark::After);
    assert(checkInvariants());
    for(uint befNode: adj[aftNode]){
        uint befNodep = getTopParent(befNode);
        LOG(cout << "testing " << befNode << " in " << befNodep <<  endl);
        if(befNodep == getTopParent(aftNode)) continue;
        switch(marked[befNodep]){
            case Mark::Before:
                continue;
            case Mark::After:
                LOG(cout << "reached " << befNode << " in " << befNodep
                    << ": odd cycle" << endl);
                // odd cycle: contract
                contractTo(befNode, aftNode);
                continue;
            case Mark::NotSeen:
                break;
        }
        assert(befNode == befNodep);
        LOG(cout << "setting preBef of " << befNode << " to " << aftNode << endl);
        assert(preBef[befNode].isNone());
        preBef[befNode] = aftNode;
        marked[befNode] = Mark::Before;
        LOG(cout << "following " << befNode << endl);
        if(expBef(befNode)) return true;
    }
    LOG(cout << "ending exploration of after node " << aftNode << " in "
        << getTopParent(aftNode) << endl);
    return false;
}

void Graph::contractTo(uint nodel, uint noder) {
    uint nodelp = getTopParent(nodel);
    uint noderp = getTopParent(noder);
    LOG(cout << "contracting with edge " << nodel << " in " << nodelp
        << " -- " << noder << " in " << noderp << endl);

    assert(marked[nodelp] == Mark::After);
    assert(marked[noderp] == Mark::After);

    uint ancestor = findCommonAncestor(nodelp, noderp);

    uint curl = nodelp;
    while(curl != ancestor){
        parents[curl] = ancestor;
        uint befNode = preAft[curl];
        base[befNode] = pair(nodel,noder);
        todo.push_back(befNode);
        parents[befNode] = ancestor;
        curl = getTopParent(preBef[befNode]);
    }
    uint curr = noderp;
    while(curr != ancestor){
        parents[curr] = ancestor;
        uint befNode = preAft[curr];
        base[befNode] = pair(noder,nodel);
        todo.push_back(befNode);
        parents[befNode] = ancestor;
        curr = getTopParent(preBef[befNode]);
    }
}

uint Graph::findCommonAncestor(uint nodel, uint noder) {
    vector<bool> seen(size, false);
    seen[nodel] = seen[noder] = true;

    uint curl = nodel, curr = noder;
    while(true){
        if(curl != root) curl = getTopParent(preBef[preAft[curl]]);
        if(curr != root) curr = getTopParent(preBef[preAft[curr]]);
        if(curl == curr) return curl;
        if(seen[curl] and curl != root) return curl;
        if(seen[curr] and curr != root) return curr;
        seen[curl] = true;
        seen[curr] = true;
    }
}

bool Graph::expBef(uint befNode) {
    LOG(cout << "exploring before node " << befNode << endl);
    assert(checkInvariants());
    assert(befNode == getTopParent(befNode));
    assert(!preBef[befNode].isNone());
    if(matchings[befNode].isNone()){
        LOG(cout << "reached end of augmenting path in " << befNode << endl);
        augmentFrom(befNode);
        return true;
    }
    uint matched = matchings[befNode];

    LOG(cout << "setting preAft of " << matched << " to " << befNode << endl);
    assert(preAft[matched].isNone());
    preAft[matched] = befNode;
    marked[matched] = Mark::After;
    todo.push_back(matched);
    LOG(cout << "end of exploring before node " << befNode << endl);
    return false;
}

void Graph::augmentFrom(uint from) {
    LOG(cout << "starting augmentation in" << from << endl);
    assert(checkInvariants());
    assert(marked[from] == Mark::Before);
    match(from,preBef[from]);
    from = preBef[from];
    while(true){

        // each loop, we go smaller in blossom heirachy. Since the inclusion is a good order,
        // the loop will terminate.
        // This also ensure that the path won't go twice at the same place
        while(marked[from] == Mark::Before){
            //go up in from's blossom. (first parent).
            auto [baseNode, oppositeBaseNode] = base[from];
            augmentFromTo(baseNode,from);
            // baseNode must have been disconnected
            assert(matchings[matchings[baseNode]] != baseNode);
            match(baseNode,oppositeBaseNode);
            from = oppositeBaseNode;
        }
        if(from == root) return;
        // we need from to be Mark::After here.
        uint befNode = preAft[from];
        from = preBef[befNode];
        // befNode must have been disconnected
        assert(matchings[matchings[befNode]] != befNode);
        match(from, befNode);
    }


}

void Graph::augmentFromTo(uint from, uint to) {
    LOG(cout << "augmenting from" << from << " to " << to << endl);
    assert(marked[to] == Mark::Before);

    while(true){
        // each loop, we go smaller in blossom heirachy. Since the inclusion is a good order,
        // the loop will terminate.
        // This also ensure that the path won't go twice at the same place
        while(marked[from] == Mark::Before){
            //go up in from's blossom. (first parent).
            auto [baseNode, oppositeBaseNode] = base[from];
            augmentFromTo(baseNode,from);
            // baseNode must have been disconnected
            assert(matchings[matchings[baseNode]] != baseNode);
            match(baseNode,oppositeBaseNode);
            from = oppositeBaseNode;
        }
        // we need from to be Mark::After here.
        uint befNode = preAft[from];
        if(befNode == to) return;
        from = preBef[befNode];
        // befNode must have been disconnected
        assert(matchings[matchings[befNode]] != befNode);
        match(from, befNode);
    }
}

bool Graph::augment(){
    LOG(cout << endl << endl << "augment !!!" << endl);
    for(uint i = 0 ; i < size ; ++i){
        if(matchings[i].isNone()){
            LOG(cout << endl << "start on exposed node " << i << endl);
            marked[i] = Mark::After;
            root = i;
            todo.push_back(i);
            while(!todo.empty() and !expAft(todo.front())) todo.pop_front();
            parents.assign(size,{});
            preBef.assign(size,{});
            preAft.assign(size,{});
            marked.assign(size, Mark::NotSeen);
            LOG(cout << "after augmenting" << endl);
            assert(checkInvariants());
            if(!todo.empty()){
                todo.clear();
                root.setNone();
                return true;
            }
        }
    }
    return false;
}




void Graph::printGraph(std::ostream& out, const std::string& s) const {

    LOG(cout << "see graph " << s <<endl);
    out << "digraph " << s << "{" << endl;
    out << "\tlabelloc=\"t\"" << endl;
    out << "\tlabel=\"" << s <<"\"" << endl;
    for (uint i = 0 ; i < size ; ++i){
        out << i << "[label=\"" << i << " in " << getTopParent(i) << "\"]" << endl;
    }
    // normal edges
    out << "\tsubgraph Normal{" << endl << "\t\tedge [dir=none]" << endl;
    for(uint i = 0; i < size ; ++i){
        for(uint j : adj[i]){
            if (i < j && matchings[i].data() != j)
                out << "\t\t" << i << " -> " << j << endl;
        };
        if(!matchings[i].isNone()){
            if(i < matchings[i]) out << "\t\t" << i << " -> "
                                     << matchings[i] << "[style=bold]" << endl;
        }
    }
    out << "\t}" << endl;

    // preAft edges
    out << "\tsubgraph PreAft{" << endl << "\t\tedge [color=red]" << endl;
    for(uint i = 0; i < size ; ++i){
        if(preAft[i].isNone()) continue;
        out << i << " -> " << preAft[i] << endl;
    }
    out << "\t}" << endl;

    // preBef edges
    out << "\tsubgraph PreAft{" << endl << "\t\tedge [color=blue]" << endl;
    for(uint i = 0; i < size ; ++i){
        if(preBef[i].isNone()) continue;
        out << i << " -> " << preBef[i] << endl;
    }
    out << "\t}" << endl;
    out << "}" << endl;


}

uint Graph::greedyMatch() {
    uint nEdges = 0;
    for(uint i = 0; i < adj.size(); ++i) {
        nEdges += adj[i].size();
    }
    vector<pair<uint, pair<uint, uint>>> edges;
    edges.reserve(nEdges);

    for(uint i = 0; i < adj.size(); ++i) {
        for(uint j : adj[i]) {
            if(i < j) {
                edges.push_back(make_pair(adj[i].size() + adj[j].size(), pair(i, j)));
            }
        }
    }
    sort(ALL(edges));

    uint res = 0;
    for(const auto& edge : edges) {
        uint u = edge.second.first;
        uint v = edge.second.second;
        if(matchings[u].isNone() && matchings[v].isNone()) {
            matchings[u] = v;
            matchings[v] = u;
            res += 1;
        }
    }

    return res;
}
