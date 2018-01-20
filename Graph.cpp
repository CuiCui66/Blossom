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

    for(auto v : stack){
        cout << v <<endl;
        passert(v == getTopParent(v));
    }


    passert(preBef.size() == originalSize);
    passert(preAft.size() == originalSize);
    passert(marked.size() == originalSize);

    for(uint i = 0 ; i < originalSize ; ++i){
        if(!preAft[i].isNone()) passert(followPathAft(i) == stack[0]);
        if(!preBef[i].isNone()) passert(followPathBef(i) == stack[0]);
        if(marked[i] == Mark::After) passert(!preAft[i].isNone());
        if(marked[i] == Mark::Before) passert(!preBef[i].isNone());
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
    //cout << "exploring after node " << aftNode << endl;
    for(uint befNode: snodes[aftNode]){
        uint befNodep = getTopParent(befNode);
        //cout << "testing " << befNode << " in " << befNodep <<  endl;
        if(befNodep == getTopParent(aftNode)) continue;
        switch(marked[befNodep]){
            case Mark::Before:
                continue;
            case Mark::After:
                //cout << "reached " << befNodep << ": odd cycle" << endl;
                // odd cycle: contract
                contractUntil(befNodep);
                preBef[aftNode] = befNode;
                continue;
            case Mark::NotSeen:
                break;
        }
        assert(befNode == befNodep);
        preBef[befNode] = aftNode;
        marked[befNode] = Mark::Before;
        stack.push_back(befNode);
        //cout << "following " << befNode << endl;
        switch(expBef(befNode)){
            case RetVal::Continue:
                stack.pop_back();
                continue;
            case RetVal::End:
                return RetVal::End;
            case RetVal::InContract:
                if(aftNode != getTopParent(aftNode)) preBef[aftNode] = befNode;
                continue;
        }
    }
    if(preBef[aftNode].isNone()){
        //cout << "ending exploration of after node" << aftNode << " normally" << endl;
        return RetVal::Continue;
    }
    else{
        //cout << "ending exploration of after node" << aftNode << " by contracting" << endl;
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

void Graph::contractUntil(uint until) {
    while(stack.back() != until){
        parents[stack.back()] = until;
        stack.pop_back();
    }
}

Graph::RetVal Graph::expBef(uint befNode) {
    //cout << "exploring before node " << befNode << endl;
    assert(befNode == getTopParent(befNode));
    assert(!preBef[befNode].isNone());
    if(matchings[befNode].isNone()){
        //cout << "reached end of augmenting path in " << befNode << endl;
        augmentFrom(befNode);
        return RetVal::End;
    }
    uint matched = matchings[befNode];
    preAft[matched] = befNode;
    marked[matched] = Mark::After;
    stack.push_back(matched);
    switch(expAft(matched)){
        case RetVal::Continue:
            stack.pop_back();
            return RetVal::Continue;
        case RetVal::End:
            return RetVal::End;
        case RetVal::InContract:
            //cout << "reexploring before node " << befNode << "as after node" << endl;
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
    //cout << "end of exploring before node " << befNode << endl;
}

void Graph::augmentFrom(uint from) {
    OptIndex node = from;
    do{
        assert(!preBef[node].isNone());
        match(node,preBef[node]);
        node = preAft[preBef[node]];
    } while(!node.isNone());
}

bool Graph::augment(){
    //cout << "augment !!!" << endl;
    for(uint i = 0 ; i < originalSize ; ++i){
        if(matchings[i].isNone()){
            marked[i] = Mark::After;
            stack.push_back(i);
            RetVal res = expAft(i);
            parents.assign(originalSize, {});
            preBef.assign(originalSize,{});
            preAft.assign(originalSize,{});
            marked.assign(originalSize, Mark::NotSeen);
            stack.resize(0);
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
    assert(checkInvariants());


    out << "graph " << s << "{" << endl;
    for(uint i = size(); i != uint(-1) ; --i){
        if(parents[i].isNone()){
            out << i << endl;
            for(uint j : snodes[i]){
                    if (i < j && matchings[i].data() != j)
                        out << i << " -- " << getTopParent(j) << endl;
                };
            if(!matchings[i].isNone()){
                if(i < matchings[i]) out << i << " -- " << matchings[i] << "[style=bold]" << endl;
            }
        }
    }
    out << "}" << endl;
}
