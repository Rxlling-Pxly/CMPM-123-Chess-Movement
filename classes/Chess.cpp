#include "Chess.h"
#include <limits>
#include <cctype>
#include <cmath>

Chess::Chess()
{
    setNumberOfPlayers(2);

    _grid = new Grid(8, 8);
    _grid->initializeChessSquares(PIECE_SIZE, "boardsquare.png");

    generateKnightMoveBitboardArray();
}
void Chess::generateKnightMoveBitboardArray()
{
    struct Offset { int dx; int dy; };
    static constexpr Offset offsets[] = { {1,2}, {2,1}, {2,-1}, {1,-2}, {-1,-2}, {-2,-1}, {-2,1}, {-1,2} };

    for (int fromIndex = 0; fromIndex < 64; fromIndex++)
    {
        Bitboard bitboard;
        int fromRank = fromIndex / 8;
        int fromFile = fromIndex % 8;

        for (auto [df, dr] : offsets)
        {
            int toRank = fromRank + dr;
            int toFile = fromFile + df;

            if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7) continue;

            int toIndex = toRank * 8 + toFile;
            bitboard |= 1ULL << toIndex;
        }

        _knightMoveBitboardArray[fromIndex] = bitboard;
    }
}

Chess::~Chess()
{
    delete _grid;
}

void Chess::setUpBoard()
{
    fenToBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
}
void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y)
        { square->destroyBit(); });
}

std::string Chess::initialStateString()
{
    return stateString();
}
std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y)
        { s += pieceNotation( x, y ); });
    return s;
}
void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y)
    {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber)
            square->setBit(pieceForPlayer(playerNumber - 1, Pawn));
        else
            square->setBit(nullptr);
    });
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::pieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(PIECE_SIZE, PIECE_SIZE);

    return bit;
}

void Chess::fenToBoard(const std::string& fen) {
    int rank = 7; // row/y
    int file = 0; // column/x
    for (char c : fen)
    {
        if (c == '/')
        {
            rank--;
            file = 0;
        }
        else if (isdigit(c))
        {
            int digit = c - '0';
            file += digit;
        }
        else // c is alphabetic
        {
            int playerNumber = isupper(c) ? 0 : 1;

            ChessPiece piece;
            switch(toupper(c))
            {
                case 'P':   piece = Pawn;     break;
                case 'N':   piece = Knight;   break;
                case 'B':   piece = Bishop;   break;
                case 'R':   piece = Rook;     break;
                case 'Q':   piece = Queen;    break;
                case 'K':   piece = King;     break;
            }

            int gameTag = isupper(c) ? piece : piece + 128;

            Bit *bit = pieceForPlayer(playerNumber, piece);
            bit->setGameTag(gameTag);
            ChessSquare *square = _grid->getSquare(file, rank);
            bit->setParent(square);
            bit->setPosition(square->getPosition());
            square->setBit(bit);

            file++;
        }
    }
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return true;
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}
