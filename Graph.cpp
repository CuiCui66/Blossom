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
    for(uint i = 0 ; i < originalSize; ++i){
        for(uint adj : snodes[i]){
            passert(count(ALL(snodes[adj]),i));
        }
    }

    // matching coherence
    for(uint i = 0 ; i < originalSize; ++i){
        if(!matchings[i].isNone()){
            // matchings is involutive
            passert(matchings[matchings[i]].get() == i);

            // matching is a real edge
            passert(count(ALL(snodes[i]),matchings[i].get()));
        }
    }

    if(stack.empty()) return true; // not in augment.

    std::vector<bool> inStack2(originalSize);
    for(auto v : stack){
        //cout << v <<endl;
        passert(v == getTopParent(v));
        inStack2[v] = true;
    }
    passert(inStack2 == inStack);


    passert(preBef.size() == originalSize);
    passert(preAft.size() == originalSize);
    passert(marked.size() == originalSize);

    for(uint i = 0 ; i < originalSize ; ++i){
        if(i == stack[0]) continue;
        //cout << "checking " << i << endl;
        if(!parents[i].isNone()) {
            passert(!preAft[i].isNone());
            passert(!preBef[i].isNone());
        }
        if(!preAft[i].isNone()) passert(followPathAft(i) == stack[0]);
        if(!preBef[i].isNone()) passert(followPathBef(i) == stack[0]);
        if(marked[i] == Mark::After) passert(!preAft[i].isNone());
        if(marked[i] == Mark::Before) passert(!preBef[i].isNone());
        if(!preAft[i].isNone()) passert(matchings[i] == preAft[i]);
    }



    return true;
}

uint Graph::followPathAft(uint aftNode)const{
    if(preAft[aftNode].isNone()) return aftNode;
    else return followPathBef(preAft[aftNode]);
}
uint Graph::followPathBef(uint befNode)const{
    assert(!preBef[befNode].isNone());
    return followPathAft(preBef[befNode]);
}

Graph::RetVal Graph::expAft(uint aftNode) {
    LOG(cout << "exploring after node " << aftNode << endl);
    assert(checkInvariants());
    for(uint befNode: snodes[aftNode]){
        uint befNodep = getTopParent(befNode);
        LOG(cout << "testing " << befNode << " in " << befNodep <<  endl);
        if(befNodep == getTopParent(aftNode)) continue;
        switch(marked[befNodep]){
            case Mark::Before:
                continue;
            case Mark::After:
                LOG(cout << "reached " << befNodep << ": odd cycle" << endl);
                // odd cycle: contract
                contractTo(befNode, aftNode);
                continue;
            case Mark::NotSeen:
                break;
        }
        assert(befNode == befNodep);
        LOG(cout << "1setting preBef of " << befNode << " to " << aftNode << endl);
        preBef[befNode] = aftNode;
        marked[befNode] = Mark::Before;
        inStack[befNode] = true;
        stack.push_back(befNode);
        LOG(cout << "following " << befNode << endl);
        switch(expBef(befNode)){
            case RetVal::Continue:
                inStack[stack.back()] = false;
                stack.pop_back();
                continue;
            case RetVal::End:
                return RetVal::End;
            case RetVal::InContract:
                if(aftNode != getTopParent(aftNode)){
                    LOG(cout << "2setting preBef of " << aftNode << " to " << befNode << endl);
                    preBef[aftNode] = befNode;
                }
                continue;
        }
    }
    if(preBef[aftNode].isNone()){
        LOG(cout << "ending exploration of after node " << aftNode << " normally" << endl);
        return RetVal::Continue;
    }
    else{
        LOG(cout << "ending exploration of after node " << aftNode << " by contracting" << endl);
        return RetVal::InContract;
    }
    /*explore for parent node.
      for all edges to v:
      check if v(after taking the parent) is marked
      if marked bef : do nothing on this edge
      if marked aft : do contraction immediately (and return contract when finished)
      and set our preBef
      if marked nothing continue
      set preBef and then call expBef(v)
      if id returns fail, continue for
      if it returns contract, continue for but switch to mode contract.
      set out PreBef.
      if it returns augment, return augment.
    */
}

bool Graph::contractTo(uint node, uint stackNode) {
    LOG(cout << "contracting to " << node << " in " << getTopParent(node) << endl);
    // computing common ancestor
    uint ancestor = getTopParent(node);
    while (!inStack[ancestor]){
        assert(!preAft[ancestor].isNone());
        assert(!preBef[preAft[ancestor]].isNone());
        ancestor = getTopParent(preBef[preAft[ancestor]]);
    }
    assert(ancestor == getTopParent(ancestor));
    LOG(cout << "ancestor is " << ancestor <<  endl);

    if (stackNode != ancestor and preBef[stackNode].isNone()){
        LOG(cout << "3setting preBef of " << stackNode << " to " << node << endl);
        preBef[stackNode] = node;
    }
    if (node != ancestor and preBef[node].isNone()){
        LOG(cout << "3setting preBef of " << stackNode << " to " << node << endl);
        preBef[stackNode] = node;
    }
    // merging on stack side
    while(stack.back() != ancestor){
        parents[stack.back()] = ancestor;
        inStack[stack.back()] = false;
        LOG(cout << "merging stack " << stack.back() << endl);
        stack.pop_back();
        assert(!stack.empty());
    }

    // setting pre aft and preBef on stack side
    uint current = getTopParent(stackNode);
    while (current != ancestor){
        assert(!preAft[current].isNone());
        uint befNode = preAft[current];
        assert(preAft[befNode].isNone());
        preAft[befNode] = current;
        assert(!preBef[befNode].isNone());
        current = preBef[befNode];
        assert(preBef[current].isNone());
        preBef[current] = befNode;
        current = getTopParent(current);
    }

    //majority of cases:
    if(getTopParent(node) == ancestor) return false;

    // merging on non-stack side
    current = node;
    LOG(cout << "contracting to2 " << node << " in " << getTopParent(node) << endl);
    while (getTopParent(current) != ancestor){
        LOG(cout << "merging non-stack " << current << " by parent " << getTopParent(current) << endl);
        parents[getTopParent(current)] = ancestor;
        assert(!preAft[current].isNone());
        current = preAft[current];
        LOG(cout << "merging non-stack " << current << " by parent " << getTopParent(current) << endl);
        parents[getTopParent(current)] = ancestor;
        assert(!preBef[current].isNone());
        current = preBef[current];
    }

    // calling expAft on non-after explored node on non stack-size and setting expBef/ expAft
    current = node;
    uint befNode = stackNode;
    while (current != ancestor){
        if (preBef[current].isNone()){
            LOG(cout << "4setting preBef of " << current << " to " << stackNode << endl);
            preBef[current] = befNode;
        }

        assert(!preAft[current].isNone());
        uint befNode = preAft[current];
        if(preAft[befNode].isNone()){
            LOG(cout << "setting preAft of " << befNode << " to " << current << endl);
            preAft[befNode] = current;
            switch(expAft(befNode)){
                case RetVal::End:
                    return true;
                default:
                    break;
            }
        }
        assert(!preBef[befNode].isNone());
        current = preBef[befNode];
    }
    return false;
}

Graph::RetVal Graph::expBef(uint befNode) {
    LOG(cout << "exploring before node " << befNode << endl);
    assert(checkInvariants());
    assert(befNode == getTopParent(befNode));
    assert(!preBef[befNode].isNone());
    if(matchings[befNode].isNone()){
        LOG(cout << "reached end of augmenting path in " << befNode << endl);
        augmentFrom(befNode);
        return RetVal::End;
    }
    uint matched = matchings[befNode];

    LOG(cout << "setting preAft of " << matched << " to " << befNode << endl);
    preAft[matched] = befNode;
    marked[matched] = Mark::After;
    inStack[matched] = true;
    stack.push_back(matched);
    switch(expAft(matched)){
        case RetVal::Continue:
            inStack[stack.back()] = false;
            stack.pop_back();
            return RetVal::Continue;
        case RetVal::End:
            return RetVal::End;
        case RetVal::InContract:
            LOG(cout << "reexploring before node " << befNode << "as after node" << endl);

            LOG(cout << "setting preAft of " << befNode << " to " << matched << endl);
            preAft[befNode] = matched;
            switch(expAft(befNode)){
                case RetVal::Continue:
                    return RetVal::InContract;
                case RetVal::InContract:
                    return RetVal::InContract;
                case RetVal::End:
                    return RetVal::End;
            }
    }
    LOG(cout << "end of exploring before node " << befNode << endl);
}

void Graph::augmentFrom(uint from) {
    LOG(cout << "before augmenting" << endl);
    assert(checkInvariants());
    OptIndex node = from;
    do{
        assert(!preBef[node].isNone());
        match(node,preBef[node]);
        node = preAft[preBef[node]];
    } while(!node.isNone());
}

bool Graph::augment(){
    LOG(cout << endl << "augment !!!" << endl);
    for(uint i = 0 ; i < originalSize ; ++i){
        if(matchings[i].isNone()){
            marked[i] = Mark::After;
            inStack[i] = true;
            stack.push_back(i);
            RetVal res = expAft(i);
            parents.assign(originalSize, {});
            preBef.assign(originalSize,{});
            preAft.assign(originalSize,{});
            marked.assign(originalSize, Mark::NotSeen);
            stack.resize(0);
            inStack.assign(originalSize, false);
            LOG(cout << "after augmenting" << endl);
            assert(checkInvariants());
            switch (res){
                case RetVal::InContract:
                    assert(!"unreachable");
                    exit(1);
                case RetVal::End:
                    return true; // the graph has been augmented.
                case RetVal::Continue:
                    continue;
            }
        }
    }
    return false;
}


/*
  if unmatched, do augmentation, then return Augment
  if matched, set preAft on it and then call expAft on it
  if Returns fail return fail.
  if Returns augment, return augment
  if returns contract
  set preAft on self toward after.
  Explore as after: call expAft on yourself.
  if it returns contract, return contract
  if it returns fail, return contract.
  if it returns augment. return augment.
*/





void Graph::printGraph(std::ostream& out, const std::string& s) const {

    LOG(cout << "see graph " << s <<endl);
    out << "digraph " << s << "{" << endl;
    out << "\tlabelloc=\"t\"" << endl;
    out << "\tlabel=\"" << s <<"\"" << endl;
    for (uint i = 0 ; i < originalSize ; ++i){
        out << i << "[label=\"" << i << " in " << getTopParent(i) << "\"]" << endl;
    }
    // normal edges
    out << "\tsubgraph Normal{" << endl << "\t\tedge [dir=none]" << endl;
    for(uint i = 0; i < originalSize ; ++i){
        for(uint j : snodes[i]){
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
    for(uint i = 0; i < originalSize ; ++i){
        if(preAft[i].isNone()) continue;
        out << i << " -> " << preAft[i] << endl;
    }
    out << "\t}" << endl;

    // preBef edges
    out << "\tsubgraph PreAft{" << endl << "\t\tedge [color=blue]" << endl;
    for(uint i = 0; i < originalSize ; ++i){
        if(preBef[i].isNone()) continue;
        out << i << " -> " << preBef[i] << endl;
    }
    out << "\t}" << endl;
    out << "}" << endl;


}
