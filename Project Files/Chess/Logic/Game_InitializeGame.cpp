#include"Game.hpp"

Game::Game(){
    AssignMagicNumbers();
    GenerateKingAttackBoards();
    GenerateKnightAttackBoards();
    SetupGame("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");
    PrintBoard();
}
