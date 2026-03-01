#pragma once

#include "Game.h"
#include "Grid.h"

constexpr int pieceSize = 80;

enum ChessPiece
{
    None,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

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
    e_numBitboards
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

    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;
};