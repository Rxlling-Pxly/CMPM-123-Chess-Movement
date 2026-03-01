#pragma once

#include <array>
#include <vector>
#include "Bitboard.h"
#include "Game.h"
#include "Grid.h"

constexpr int PIECE_SIZE = 80;
constexpr int WHITE = 0;
constexpr int BLACK = 1;

enum Bitboards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,
    WHITE_PIECES,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
    BLACK_PIECES,
    NO_PIECES,
    ALL_PIECES,
    NUM_BITBOARDS
};

class Chess : public Game
{
public:
    Grid *getGrid() override { return _grid; }

    Chess();
    ~Chess();

    void setUpBoard() override;
    void stopGame() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

private:
    Grid *_grid;
    std::array<Bitboard, 64> _knightMoveBitboardArray;

    int _currentPlayer;
    Bitboard _bitboards[NUM_BITBOARDS];
    int _bitboardLookup[128];
    std::vector<BitMove> _moves;

    std::array<Bitboard, 64> generateKnightMoveBitboardArray();

    void fenToBoard(const std::string &fen);
    Bit *pieceForPlayer(const int playerNumber, ChessPiece piece);

    std::vector<BitMove> generateMoves();
    void generateKnightMoves(std::vector<BitMove> &moves, Bitboard knightBoard, Bitboard allPiecesBoard);

    char pieceNotation(int x, int y) const;

    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
};