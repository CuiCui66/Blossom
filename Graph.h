#include <vector>
#include <set>
#include <assert.h>
#include <algorithm>
#include <variant>

using uint = unsigned int;

#define ALL(v) (v).begin(),(v).end()


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
    using Node = std::variant<SimpleNode,ContractedNode>;
    template<typename F>
    void for_each_adj(uint v, F&& f){
        static_assert(std::is_invocable_v<F, uint>);
        static_assert(std::is_same_v<std::invoke_result_t<F, uint>, void>);
        std::visit(overloaded{
                [&](SimpleNode& sn){
                    for(uint i : sn){
                        f(i);
                    }
                },
                    [&](ContractedNode& cn){
                        for(uint i : cn){
                            for_each_adj(i,f);
                        }
                    }
            },nodes[v]);
    }
    template<typename F>
    void for_each_adj(uint v, F&& f) const {
        static_assert(std::is_invocable_v<F, uint>);
        static_assert(std::is_same_v<std::invoke_result_t<F, uint>, void>);
        std::visit(overloaded{
                [&](const SimpleNode& sn){
                    for(uint i : sn){
                        f(i);
                    }
                },
                    [&](const ContractedNode& cn){
                        for(uint i : cn){
                            for_each_adj(i,f);
                        }
                    }
            },nodes[v]);
    }

    std::vector<Node> nodes;
    std::vector<OptIndex> parents;
    std::vector<OptIndex> matchings;
public:
    Graph(uint n) : nodes(n), parents(n), matchings(n){
    }
    bool checkInvariants()const;
    void addEdge(uint i, uint j){
        assert(i < nodes.size());
        assert(nodes[i].index() == 0);
        assert(j < nodes.size());
        assert(nodes[j].index() == 0);
        std::get<SimpleNode>(nodes[i]).insert(j);
        std::get<SimpleNode>(nodes[j]).insert(i);
    }
    void match(uint i, uint j){
        assert(i < nodes.size());
        assert(j < nodes.size());
        // TODO add more verifications
        matchings[i] = j;
        matchings[j] = i;
    }
    void contract(std::vector<uint> oddCycle);
    size_t size() const{
        size_t sizev = nodes.size();
        assert(parents.size() == sizev);
        assert(matchings.size() == sizev);
        return nodes.size();
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
};


