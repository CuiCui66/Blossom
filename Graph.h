#include <vector>
#include <assert.h>
#include <algorithm>
#include <ostream>
#include <iostream>
// #include <variant>

using uint = unsigned int;

#define ALL(v) (v).begin(),(v).end()

#define LOG(x) x

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
    std::vector<Mark> marked;
    std::vector<bool> inStack;
    std::vector<uint> stack;
    enum class RetVal{
        /// Means that non end of augmenting path or contraction was found
        Continue,
        /// Means that augmentation has already been done : just returns to top.
        End ,
        /// Means that your are now part of a contracted node : you must set or have set
        /// both preBef and preAft.
        InContract};

    RetVal expAft(uint aftNode);
    RetVal expBef(uint befNode);
    // return true if end was found during contraction.
    bool contractTo(uint node, uint stackNode);
    // augment from "from" following alternatively preBef and preAft
    void augmentFrom(uint from);

    uint followPathAft(uint aftNode)const;
    uint followPathBef(uint befNode)const;

public:
    Graph(uint n) : snodes(n), parents(n), matchings(n),
                    originalSize(n), preAft(n), preBef(n), marked(n), inStack(n){
        stack.reserve(n);
    }
    bool checkInvariants()const;
    void addEdge(uint i, uint j){
        assert(i < size());
        assert(j < size());
        snodes[i].push_back(j);
        snodes[j].push_back(i);
    }
    void match(uint i, uint j){
        std::cout << "matching " << i << " and " << j << std::endl;
        assert(i < size());
        assert(j < size());
        matchings[i] = j;
        matchings[j] = i;
    }
    void contract(std::vector<uint> oddCycle);
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


