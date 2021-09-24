#pragma once
#include<string>
#include<vector>
#include"time.h"

struct test{
std::string testName;
std::string fen;
std::vector<long long int> expected;
};

std::vector<test> tests = {
    {
        "Initial position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {1,20,400,8902,197281,4865609}
    },
    {
        "Position 2",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        {1,48,2039,97862,4085603,193690690}
    },
    {
        "Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
        {1,14,191,2812,43238,674624}
    },
    {
        "Position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {1,6,264,9467,422333,15833292}
    },
    {
        "Position 4, mirrored",
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        {1,6,264,9467,422333,15833292}
    },
    {
        "Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {1,44,1486,62379,2103487,89941194}
    },
    {
        "Position 6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -",
        {1,46,2079,89890,3894594,164075551}
    }
};


void runTests(int depth){
    for(test t : tests){
        
        clock_t tStart = clock();
        Game* g = new Game();
        g->SetupGame(t.fen);
        std::cout << "Running test: " << t.testName << std::endl;
        for(int i = 0; i <= depth; i++){
            int ans = g->SumOfAllMoves(i, false);
            if(ans == t.expected[i]){
                std::cout << t.testName << " depth " << i << " PASS"<<std::endl;
                //printf("Time taken: %.2fm\n", ((double)(clock() - tStart) / CLOCKS_PER_SEC)/60);
            }
            else{
                std::cout << t.testName << " depth " << i << " FAIL"<< "expected " << t.expected[i] << " but got "<<ans <<std::endl;
                //printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
            }
        }
        std::cout << std::endl;
        delete g;
    }
}