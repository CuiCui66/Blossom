#include <vector>
#include <deque>
#include <assert.h>
#include <algorithm>
#include <iostream>

using uint = unsigned int;

#define ALL(v) (v).begin(),(v).end()

#ifdef NDEBUG
#define LOG(x)
#else
#define LOG(x) x
#endif

/// This class provides the whole blossom algorithm.
/// The graph must be input by using addEdge
class Graph{
    /// simple optional integer using - to represent None.
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


    /// adjacency list
    std::vector<std::vector<uint>> adj;
    /// union find parent.
    std::vector<OptIndex> parents;
    /// i matches with j means matchings[i] = j and matchings[j] = i
    std::vector<OptIndex> matchings;
    /// graph size (number of nodes)
    uint size;

    /// marks for nodes in alternating tree
    enum class Mark{
        NotSeen, ///< means not in the tree
        Before, ///< means in the tree before an edge of the matchings (has only one child)
        After ///< means in the tree after an edge of the matching (even distance from root)
    };
    /// for nodes in alternating tree that are Mark::After, the parent node in the tree
    std::vector<OptIndex> preAft;
    /// for nodes in alternating tree that are Mark::Before, the parent node in the tree
    std::vector<OptIndex> preBef;
    /// marking of nodes during exploration.
    std::vector<Mark> marked;
    /// value base[i] has meaning iff marked[i] = Mark::Before and i is contracted
    /// it the only edge of a blossom that is not in the tree.
    std::vector<std::pair<uint,uint>> base;
    /// list of nodes to be explored (Must be Mark::After in the tree)
    std::deque<uint> todo;
    /// root of alternating tree.
    OptIndex root;

    /// explore a Mark::After node
    /// O(number of edge adjacent of this node * log n).
    /// I didn't count contraction or augmentation functions that can be called in this function
    /// in the complexity.
    /// This funtion is called at most once on each node per call of augment.
    bool expAft(uint aftNode);

    /// explore a Mark::Before node
    /// O(1) except when augmentation occurs (there is only one augmentation during an augment
    /// so I count that separately)
    bool expBef(uint befNode);

    /// contract a odd cycle whose all edges are in the tree except nodel and noder.
    /// nodel and noder must be the nodes in the original graph.
    /// O(size of cycle * log n)
    /// the log n is for union find
    /// the sum of all cycle sizes during one augment is exactly (n + number of cycle),.
    /// Thus, during one augment, this function takes O(n).
    void contractTo(uint nodel, uint noder);

    /// node1 and node2 must be nodes of contracted graph and in the tree
    /// result is common ancestor in the tree
    /// O(sum of size of both branches * log n)
    uint findCommonAncestor(uint nodel, uint noder);

    /// Augment from "from" to the root following an alternating path
    /// O(length of path)
    void augmentFrom(uint from);

    /// Find an odd alternating path between from and to that have matching edge on both side
    /// from and to are node in original graph.
    /// the input invariant needed is described in the report
    /// reduce matching size by one.
    /// O(length of path)
    void augmentFromTo(uint from, uint to);

    /// follow a path in contracted graph starting from an after node.
    uint followPathAft(uint aftNode)const;
    /// follow a path in contracted graph starting from a before node.
    uint followPathBef(uint befNode)const;

    /// matchs to node in original graph.
    /// O(1)
    void match(uint i, uint j){
        LOG(std::cout << "matching " << i << " and " << j << std::endl);
        assert(i < size);
        assert(j < size);
        matchings[i] = j;
        matchings[j] = i;
    }

    /// non compressing but const union find call.
    uint getTopParent(uint vertex)const{
        if(parents[vertex].isNone()){
            return vertex;
        }
        else return getTopParent(parents[vertex]);
    }

    /// compressing union-find call.
    /// The root of each class is forced, so we do not have rank optimization.
    /// O(log n)
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



public:
    Graph(uint n) : adj(n), parents(n), matchings(n),
                    size(n), preAft(n), preBef(n), marked(n), base(n){
    }
    /// check that all invariant are maintained (very slow)
    /// one call may be serveral times longer than the whole blossom algorithm
    bool checkInvariants()const;
    void addEdge(uint i, uint j){
        assert(i < size);
        assert(j < size);
        adj[i].push_back(j);
        adj[j].push_back(i);
    }
    /// Augment matching size by one if possible.
    /// returns false if graph cannot be augmented
    /// o(m * log(n))
    bool augment();

    void printGraph(std::ostream& out, const std::string& s) const;

    uint unmatched(){
        assert(checkInvariants());
        uint res = 0;
        for(auto optint: matchings){
            if(optint.isNone()) ++res;
        }
        return res;
    }

    uint greedyMatch();
};


