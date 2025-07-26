#ifndef PLACEDB_H
#define PLACEDB_H
#include "objects.h"
// #include "plot.h"


// Custom hash for std::pair<std::string, std::string>
struct PairStringHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        // Simple combination of hashes. Boost::hash_combine is more robust.
        return h1 ^ (h2 << 1);
    }
};
struct P2PHash {
    std::size_t operator()(const std::pair<Pin*, Pin*>& p) const {
        std::size_t h1 = std::hash<Pin*>{}(p.first);
        std::size_t h2 = std::hash<Pin*>{}(p.second);
        return h1 ^ (h2 << 1);  // 可用 boost::hash_combine for better result
    }
};

struct P2PEqualUnordered {
    bool operator()(const std::pair<Pin*, Pin*>& a,
                    const std::pair<Pin*, Pin*>& b) const {
        return (a.first == b.first && a.second == b.second) ||
               (a.first == b.second && a.second == b.first);
    }
};



class PlaceDB
{
public:
    PlaceDB()
    {
        layerCount = -1;
        moduleCount = -1;
        dbMacroCount = 0;
        commonRowHeight = -1;
        dbNodes.clear();
        dbTerminals.clear();
        dbPins.clear();
        dbNets.clear();
        dbSiteRows.clear();
        dbTiers.clear();
        coreRegion = CRect();
        chipRegion = CRect();
        totalRowArea = 0;
        nodesLocationRegister.clear();
    };
    int layerCount;  // how many layers? this is for 3dic
    int moduleCount; // number of modules
    int dbMacroCount;
    int netCount;
    int pinCount;
    double commonRowHeight; //! rowHeight that all(most of the times) rows share

    CRect coreRegion;   // In 2d placement the core region is just the rectangle that encloses all placement rows(see setCoreRegion), in 3d it might be shrunk. coreRegion should be smaller than the whole chip
    CRect chipRegion;   // Chip Region is obtained with coreRegion and all terminal locations. adapect1 is a good example. This should only be used for plot.
    float totalRowArea; //! area of all placement rows, equal or less than coreRegion area, usually equals coreRegion area. calculated in setCoreRegion

    //! dbXxs: vector for storing Xxs
    vector<Module *> dbNodes; // nodes include std cells and macros
    vector<Module *> dbTerminals;
    vector<Pin *> dbPins;
    vector<Net *> dbNets;
    vector<SiteRow> dbSiteRows;
    vector<Tier *> dbTiers;

    unordered_map<pair<string,string>, pair<Module*, Pin *>, PairStringHash> NodePinMap; // map node name to node pointer
    unordered_map<std::pair<Pin*, Pin*>, float, P2PHash, P2PEqualUnordered> P2PWeightMap;
    map<string, Module *> moduleMap; // map module name to module pointer(module include nodes and terminals)

    vector<POS_3D> nodesLocationRegister;

    void setCoreRegion();
    void init_tiers();

    Module *addNode(int index, string name, float width, float height); // (frank) 2022-05-13 consider terminal_NI
    Module *addTerminal(int index, string name, float width, float height, bool isFixed, bool isNI);
    void addNet(Net *);
    int addPin(Module *, Net *, string, float, float);

    void allocateNodeMemory(int);
    void allocateTerminalMemory(int);
    void allocateNetMemory(int);
    void allocatePinMemory(int);

    void resetP2Pweights() {
        P2PWeightMap.clear();
    }
    float getP2Pweight(Pin *pin1, Pin *pin2)
    {
        auto it = P2PWeightMap.find(make_pair(pin1, pin2));
        if (it != P2PWeightMap.end())
        {
            return it->second;
        }
        return 1.0f; // or throw an exception
    }
    void updateP2Pweight(string node1name, string pin1name, string node2name, string pin2name, float slack , float WNS)
    {
        auto [node1, pin1] = NodePinMap[make_pair(node1name, pin1name)];
        auto [node2, pin2] = NodePinMap[make_pair(node2name, pin2name)];
        auto curP2P = make_pair(pin1, pin2);
        float curWeight = getP2Pweight(pin1, pin2); // check if pin1 and pin2 exist
        P2PWeightMap[curP2P] = curWeight + abs(slack) / WNS; // insert or update
    }


    Module *getModuleFromName(string);

    void InitializeNodePinMap(); // initialize NodePinMap, should be called after all nodes and pins are added
    void setModuleInitialLocation_2D(Module *, float, float); // set module location in 2D
    void setModuleLocation_2D(Module *, float, float);
    void setModuleLocation_2D(Module *, POS_3D);
    void setModuleCenter_2D(Module *, float, float);
    void setModuleCenter_2D(Module *, POS_3D);
    void setModuleCenter_2D(Module *, VECTOR_3D);
    void setModuleOrientation(Module *, int);
    void setModuleLocation_2D_random(Module *);
    void moveModule_2D(Module *, VECTOR_2D);
    void moveModule_2D(Module *, VECTOR_2D_INT);
    void randomPlacment();
    void addNoise();
    void saveNodesLocation();
    void loadNodesLocation();
    CRect getOptimialRegion(Module *); // for legalized results, normally called in detailed placement

    POS_3D getValidModuleCenter_2D(Module *module, float x, float y);

    double calcHPWL();
    double calcWA_Wirelength_2D(VECTOR_2D);
    double calcLSE_Wirelength_2D(VECTOR_2D);
    double calcNetBoundPins();
    double calcModuleHPWL(Module *);
    // double calcModuleHPWLunsafe(Module *);
    double calcModuleHPWLfast(Module *);

    void moveNodesCenterToCenter(); // used for initial 2D quadratic placement

    void removeBlockedSite(); // calculate intervals of siterows considering macros and terminals that block sites, see void RemoveFixedBlockSite() and void RemoveMacroSite() in ntuplace

    void setChipRegion_2D();

    void showDBInfo();
    void showRows();

    void outputBookShelf(string, bool);

    int y2RowIndex(float);
    bool isConnected(Module *, Module *);

    // void plotCurrentPlacement(string);
private:
    void outputAUX();
    void outputNodes();
    void outputPL();
    void outputNets();
    void outputSCL();
};
#endif