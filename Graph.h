#include <vector>
#include <deque>
#include <assert.h>
#include <algorithm>
#include <ostream>
#include <iostream>
// #include <variant>

using uint = unsigned int;

#define ALL(v) (v).begin(),(v).end()

#define LOG(x)

//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
//template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Graph{
    class OptIndex{
        uint val;
    public:
        OptIndex(uint val = -1) :val(val){}
        inline uint get()const{
            assert(val != uint(-1));
            return val;
        }
        inline uint& get(){
            assert(val != uint(-1));
            return val;
        }
        inline bool isNone()const{
            return val == uint(-1);
        }
        inline operator uint() const{
            return get();
        }
        inline operator uint&(){
            return get();
        }
        inline uint data()const {
            return val;
        }
        inline void setNone(){
            val = -1;
        }
    };
    using SimpleNode = std::vector<uint>;

    std::vector<SimpleNode> snodes;
    std::vector<OptIndex> parents;
    std::vector<OptIndex> matchings;
    uint originalSize;

    enum class Mark{NotSeen, Before, After};
    std::vector<OptIndex> preAft;
    std::vector<OptIndex> preBef;
    // value base[i] has meaning iff marked[i] = Mark::Before and i is contracted
    std::vector<Mark> marked;
    std::vector<std::pair<uint,uint>> base;
    std::deque<uint> todo;
    OptIndex root;

    bool expAft(uint aftNode);
    bool expBef(uint befNode);
    // return true if end was found during contraction.
    void contractTo(uint nodel, uint noder);
    // augment from "from" following alternatively preBef and preAft
    void augmentFrom(uint from);
    void augmentFromTo(uint from, uint to);

    uint followPathAft(uint aftNode)const;
    uint followPathBef(uint befNode)const;
    bool findPathAft(uint aftNode, uint nodeToFind)const;
    bool findPathBef(uint befNode, uint nodeToFind)const;

    /// node1 and node2 must be node of contracted graph.
    uint findCommonAncestor(uint nodel, uint noder);

public:
    Graph(uint n) : snodes(n), parents(n), matchings(n),
                    originalSize(n), preAft(n), preBef(n), marked(n), base(n){
    }
    bool checkInvariants()const;
    void addEdge(uint i, uint j){
        assert(i < size());
        assert(j < size());
        snodes[i].push_back(j);
        snodes[j].push_back(i);
    }
    void match(uint i, uint j){
        LOG(std::cout << "matching " << i << " and " << j << std::endl);
        assert(i < size());
        assert(j < size());
        matchings[i] = j;
        matchings[j] = i;
    }
    size_t size() const{
        assert(parents.size() == originalSize);
        assert(matchings.size() == originalSize);
        return originalSize;
    }
    /// returns false if graph cannot be augmented
    bool augment();

    uint getTopParent(uint vertex)const{
        if(parents[vertex].isNone()){
            return vertex;
        }
        else return getTopParent(parents[vertex]);
    }
    uint getTopParent(uint vertex){
        if(parents[vertex].isNone()){
            return vertex;
        }
        else{
            uint val = getTopParent(parents[vertex]);
            parents[vertex] = val;
            return val;
        }
    }

    void printGraph(std::ostream& out, const std::string& s) const;

    uint unmatched(){
        assert(checkInvariants());
        uint res = 0;
        for(auto optint: matchings){
            if(optint.isNone()) ++res;
        }
        return res;
    }
};


