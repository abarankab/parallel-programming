#include "../benchmark.h"
#include "../boruvka.h"
#include "../graph.h"
#include "../sequential_mst.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Please specify path to graph\n";
        return 0;
    }

    Graph G = load_graph(argv[1]);

    BoruvkaMST boruvka;
    SequentialMST sequential_mst;

    u64 weight_to_check = 0;
    u64 weight_correct = 0;
    
    {
        auto mst = boruvka.calculate_mst(G);
        for (u32 i = 0; i < mst.size(); ++i) weight_to_check += mst[i].weight;
    }
    
    {
        auto mst = sequential_mst.calculate_mst(G);
        for (u32 i = 0; i < mst.size(); ++i) weight_correct += mst[i].weight;
    }

    if (weight_to_check != weight_correct) {
        std::cerr << "Weights don't match!\nCorrect: " << weight_correct << "\nIncorrect: " << weight_to_check << "\n";
        exit(-1);
    }
    else {
        std::cout << "OK\n";
    }
    return 0;
}
