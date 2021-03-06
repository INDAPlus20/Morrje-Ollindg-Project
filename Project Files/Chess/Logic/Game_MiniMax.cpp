#include "Game.hpp"

void Game::AiMove(search_parameters params, Move& m, bool& done){
    int depth;
    int nodes;
    int maxEval;
    int beta;
    int alpha;
    int eval;
    uint64_t time;
    if(mode == NORMAL){
        depth = 4;
        nodes = -1;
        time = -1;
    }
    else if(mode == UCI)
    {
        depth = params.depth;
        nodes = params.nodes;
        time = params.moveTime;
        if(time != -1){
            time = getTime() + (uint64_t)params.moveTime; //Calculate what the time will be when we should stop searching
        }
        if(depth == -1) depth = 500;
    }


    std::pair<int,int> bestMove;
    Piece promotion;

    std::set<std::pair<int, int>> allMoves;
    MakeAllPseudoMoves(allMoves);
    std::vector<std::pair<int, std::pair<int,int>>> moveVector; //moveVector holds the evaluation of a move, and the move itself. Used for iterative deepening.

    for(auto move : allMoves){
        moveVector.push_back(std::make_pair(-1, move));
    }

    for(int i = 1; i <= depth && !stop; i++){ //Search first at depth 1, then depth 2, and so on
        maxEval = (i+1) * inf + 1;
        beta = maxEval;
        alpha = -maxEval;
        for(int j = 0; j < moveVector.size() && !stop; j++){
            auto& move = moveVector[j];
            if(MakeGameMove(move.second.first, move.second.second)){ //Move.second is the move. Move.second.first is origin square.
                if(gameState.awaitingPromotion){
                    for(auto p : {Queen,Knight,Rook,Bishop}){
                        Piece promotionPiece = ConvertToOppositeColoredPiece(p);
                        MakeBoardMove(-1, -1, promotionPiece);
                        eval = -miniMax(i-1,nodes,time,-beta,-alpha);
                        move.first = eval;
                        if(eval > alpha && !stop){
                            alpha = eval;
                            bestMove = move.second;
                            promotion = promotionPiece;
                            m = moveStack.top();
                        }
                        gameState.awaitingPromotion = true;
                    }
                    gameState.awaitingPromotion = false;
                }
                else{
                    eval = -miniMax(i-1,nodes,time,-beta,-alpha);
                    move.first = eval;
                    
                    if(eval > alpha && !stop){
                        alpha = eval;
                        bestMove = move.second;
                        m = moveStack.top();
                    }
                }
                UnmakeMove();
            }
            else{ //The move wasn't legal. At which case we remove it from the moveVector
                moveVector.erase(moveVector.begin()+j);
                j--;
            }
        }
        std::sort(moveVector.begin(), moveVector.end(), std::greater<std::pair<int,std::pair<int,int>>>()); //Make sure that the best move will be checked first next iteration
    }

    MakeGameMove(bestMove.first,bestMove.second);
    if(gameState.awaitingPromotion){
        MakeBoardMove(-1, -1, promotion);
    }

    done = true;
    return;
}

int Game::miniMax(int depth, int& nodes, long long int endTime, int alpha, int beta){
    int eval;
    nodes--;

    if(endTime != -1){
        if(getTime() > endTime) stop = true;
    } 

    if(stop) return 0; //stop means to abort completely so don't bother calculating any score. The best found move so far has already been saved anyways
    if(depth == 0 || nodes == 0) return Quiescent(alpha, beta);
    
    bool madeMove = false;
    std::set<std::pair<int,int>> allMoves;
    MakeAllPseudoMoves(allMoves);
    for(auto move : allMoves){
        if(MakeGameMove(move.first,move.second)){
            madeMove = true;

            if(gameState.awaitingPromotion){
                for(auto p : {Queen,Knight,Rook,Bishop}){
                    MakeBoardMove(-1, -1, ConvertToOppositeColoredPiece(p));
                    eval = -miniMax(depth-1,nodes,endTime,-beta,-alpha);
                    if (eval >= beta){
                        UnmakeMove();
                        return eval+1; //The plus one makes sure this move gets placed after the best move in the moveVector
                    }
                    if(eval > alpha) alpha = eval;
                    gameState.awaitingPromotion = true;
                }
                gameState.awaitingPromotion = false;
            }
            else{
                eval = -miniMax(depth-1,nodes,endTime,-beta,-alpha);
                if (eval >= beta){
                    UnmakeMove();
                    return eval+1; //The plus one makes sure this move gets placed after the best move in the moveVector
                }
                if(eval > alpha) alpha = eval;
            }
            UnmakeMove();
        }
    }
    if(madeMove){
        return alpha;
    }
    else {
        return evaluatePosition()*(depth+1) * gameState.currentPlayer;
    }
}

int Game::Quiescent(int alpha, int beta){
    int eval = evaluatePosition() * gameState.currentPlayer;
    if(eval >= beta) return beta;
    if(alpha < eval) alpha = eval;          //This ensures that non-capture score will be returned if capture is bad (i.e. all captures yield scores less than alpha)

    std::set<std::pair<int, int>> captureMoves;
    MakeAllCaptureMoves(captureMoves);

    for(auto move : captureMoves){
        int see = SEE(move.first, move.second);
        if(see <= 0){
            continue;      //If the static exchange evaluation is not beneficial for current player then no need to check that capture
        }

        if(MakeGameMove(move.first, move.second)){
            int score = eval + see;
            UnmakeMove();

            if(score >= beta){
                return beta;
            }

            if(score > alpha){
                alpha = score;
            }
        }
    }
    return alpha;
}

//This function tells me it would be nice to have bitwise operator overloading for Bitboard class... may add later
Bitboard Game::attackAndDefend(Bitboard occupancy, int originSquare){
    Bitboard attackAndDefend;
    attackAndDefend.bitBoard |= (((1ULL << (originSquare + 9)) & (1ULL << (originSquare + 7)) & (1ULL << (originSquare - 9)) & (1ULL << (originSquare - 7))) & (pieceBitboards[WhitePawn].bitBoard | pieceBitboards[BlackPawn].bitBoard));
    attackAndDefend.bitBoard |= KnightAttacks[originSquare].bitBoard & (pieceBitboards[WhiteKnight].bitBoard | pieceBitboards[BlackKnight].bitBoard);
    attackAndDefend.bitBoard |= GetAttackBishopBoardFromMagic(originSquare, occupancy).bitBoard & (pieceBitboards[WhiteBishop].bitBoard | pieceBitboards[BlackBishop].bitBoard | pieceBitboards[WhiteQueen].bitBoard | pieceBitboards[BlackQueen].bitBoard);
    attackAndDefend.bitBoard |= GetAttackRookBoardFromMagic(originSquare, occupancy).bitBoard & (pieceBitboards[WhiteRook].bitBoard | pieceBitboards[BlackRook].bitBoard | pieceBitboards[WhiteQueen].bitBoard | pieceBitboards[BlackQueen].bitBoard);
    attackAndDefend.bitBoard |= KingAttacks[originSquare].bitBoard & (pieceBitboards[WhiteKing].bitBoard | pieceBitboards[BlackKing].bitBoard);

    attackAndDefend.bitBoard &= occupancy.bitBoard;
    return attackAndDefend;
}

//SEE stands for static exchange evaluation and evaluates the best score for current player after several trades on a single square has been made
int Game::SEE(int originSquare, int destinationSquare){

    int currentPiece = Board[originSquare];
    int sideToMove = currentPiece % 2 == 1 ? 0 : 1;
    Bitboard pieceBitboard (1ULL << originSquare);

    Bitboard occupancy (whitePiecesBB.bitBoard | blackPiecesBB.bitBoard);   //All the pieces
    Bitboard attAndDef = attackAndDefend(occupancy, destinationSquare);
    int iteration = 0;

    int scoreGains[32] = {0};         //This array keeps track of the score gains for players after all trades
    scoreGains[0] = PieceValue(Board[destinationSquare]);       //The first score gain is capturing the piece at destinationSquare

    //Then check all pieces that attack the current square, using the same method used before to check if king is in check
    while(pieceBitboard.bitBoard) {
        iteration++;
        scoreGains[iteration] = PieceValue(currentPiece) - scoreGains[iteration - 1];           //The score gain for the current player is the potential capture subtracted by the score gain the opposite player has gotten so far
        if (std::max(-scoreGains[iteration-1], scoreGains[iteration]) < 0) break;
        occupancy.bitBoard ^= pieceBitboard.bitBoard;          //Remove the attacking piece to allow for x-rays

        if(currentPiece != BlackKing && currentPiece != WhiteKing && currentPiece != BlackKnight && currentPiece != WhiteKnight){     //If the attacking piece was R,B,P or Q then it may have allowed x-ray
            //Update x-rays
            attAndDef.bitBoard |= GetAttackBishopBoardFromMagic(destinationSquare, occupancy).bitBoard & (pieceBitboards[WhiteBishop].bitBoard | pieceBitboards[BlackBishop].bitBoard | pieceBitboards[WhiteQueen].bitBoard | pieceBitboards[BlackQueen].bitBoard);
            attAndDef.bitBoard |= GetAttackRookBoardFromMagic(destinationSquare, occupancy).bitBoard & (pieceBitboards[WhiteRook].bitBoard | pieceBitboards[BlackRook].bitBoard | pieceBitboards[WhiteQueen].bitBoard | pieceBitboards[BlackQueen].bitBoard);     //Update possible x-rays
        }

        attAndDef.bitBoard &= occupancy.bitBoard;

        sideToMove = !sideToMove;
        pieceBitboard.bitBoard = 0;
        for(currentPiece = WhitePawn + sideToMove; currentPiece <= BlackKing; currentPiece+=2){
            uint64_t tempBitboard = pieceBitboards[currentPiece].bitBoard & attAndDef.bitBoard;
            if(tempBitboard){           //True if one of the pieces is among the attacking pieces
                pieceBitboard.bitBoard = tempBitboard & -tempBitboard;     //Get the LSB
                break;
            }
        }
    }
    //Each entry in scoreGains denotes the score gain for a player: scoreGains[0] is gain for currentPlayer, scoreGains[1] is gain for opposite player, scoreGains[2] is gain for currentPlayer and so on
    //-scoreGains[iteration-1] is the score if currentPlayer chooses not to capture at that point, while scoreGains[iteration] is the score if the currentPlayer does choose to capture at that point
    //The currentPlayer will of course choose the option that maximizes their score, and we add a sign in front of the expression because the gain for one player is the loss for the other player
    while(--iteration) {
        scoreGains[iteration-1] = -std::max(-scoreGains[iteration-1], scoreGains[iteration]);
    }
    return scoreGains[0];
}
