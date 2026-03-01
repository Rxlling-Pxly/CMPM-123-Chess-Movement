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
    E_NUM_BITBOARDS
};

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    Grid *getGrid() override { return _grid; }

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
    std::array<Bitboard, 64> _knightMovesBitboardArray;
    std::array<Bitboard, 64> _kingMovesBitboardArray;
    std::array<int, 128> _pieceNotationToBitboardLookup;

    int _currentPlayer;
    Bitboard _bitboards[E_NUM_BITBOARDS];
    std::vector<BitMove> _moves;

    std::array<Bitboard, 64> generateKnightMovesBitboardArray();
    std::array<Bitboard, 64> generateKingMovesBitboardArray();
    std::array<int, 128> generatePieceNotationToBitboardLookup();

    void fenToBoard(const std::string &fen);
    Bit *pieceForPlayer(const int playerNumber, ChessPiece piece);

    std::vector<BitMove> generateMoves();
    void generatePawnMoves(std::vector<BitMove> &moves, const Bitboard pawns, const Bitboard noPieces, const Bitboard opponentPieces);
    void addPawnBitboardMovesToList(std::vector<BitMove> &moves, const Bitboard bitboard, const int shift);
    void generateKnightMoves(std::vector<BitMove> &moves, const Bitboard knights, const Bitboard playerPieces);
    void generateKingMoves(std::vector<BitMove> &moves, const Bitboard king, const Bitboard playerPieces);

    char pieceNotation(int x, int y) const;

    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
};