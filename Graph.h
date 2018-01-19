#include <vector>
#include <set>
#include <assert.h>
#include <algorithm>
// #include <variant>

using uint = unsigned int;

#define ALL(v) (v).begin(),(v).end()


// template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Graph{
    class OptIndex{
        uint val;
    public:
        OptIndex(uint val = -1) :val(val){}
        uint get()const{
            assert(val != uint(-1));
            return val;
        }
        uint& get(){
            assert(val != uint(-1));
            return val;
        }
        inline bool isNone()const{
            return val == uint(-1);
        }
        operator uint() const{
            return get();
        }
        operator uint&(){
            return get();
        }
        uint data()const {
            return val;
        }
    };
    using SimpleNode = std::set<uint>; // adjacency set
    using ContractedNode = std::vector<uint>; // node in contraction

    // index of snodes[i] is i, index of cnodes[i] is snodes.size() +i
    std::vector<SimpleNode> snodes;
    std::vector<ContractedNode> cnodes;
    std::vector<OptIndex> parents;
    std::vector<OptIndex> matchings;
    uint originalSize;

    template<typename F>
    void for_each_adj(uint v, F&& f){
        static_assert(std::is_invocable_v<F, uint>);
        static_assert(std::is_same_v<std::invoke_result_t<F, uint>, void>);
        if (v < originalSize){
            for(uint i : snodes[v]){
                f(i);
            }
        }
        else{
            for(uint i : cnodes[v - originalSize]){
                for_each_adj(i,f);
            }
        }
    }
    template<typename F>
    void for_each_adj(uint v, F&& f) const{
        static_assert(std::is_invocable_v<F, uint>);
        static_assert(std::is_same_v<std::invoke_result_t<F, uint>, void>);
        if (v < originalSize){
            for(uint i : snodes[v]){
                f(i);
            }
        }
        else{
            for(uint i : cnodes[v-originalSize]){
                for_each_adj(i,f);
            }
        }
    }


public:
    Graph(uint n) : snodes(n), parents(n), matchings(n), originalSize(n){
    }
    bool checkInvariants()const;
    void addEdge(uint i, uint j){
        assert(i < snodes.size());
        assert(j < snodes.size());
        snodes[i].insert(j);
        snodes[j].insert(i);
    }
    void match(uint i, uint j){
        assert(i < size());
        assert(j < size());
        // TODO add more verifications
        matchings[i] = j;
        matchings[j] = i;
    }
    void contract(std::vector<uint> oddCycle);
    size_t size() const{
        size_t sizev = originalSize + cnodes.size();
        assert(parents.size() == sizev);
        assert(matchings.size() == sizev);
        return sizev;
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

    /// uncontract all nodes and rebuild the matching
    void unfold();
};


