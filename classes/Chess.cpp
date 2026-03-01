#include "Chess.h"
#include <limits>
#include <cctype>
#include <cmath>

Chess::Chess()
{
    setNumberOfPlayers(2);

    _grid = new Grid(8, 8);
    _grid->initializeChessSquares(PIECE_SIZE, "boardsquare.png");

    _knightMovesBitboardArray = generateKnightMovesBitboardArray();
    _kingMovesBitboardArray = generateKingMovesBitboardArray();
    _pieceNotationToBitboardLookup = generatePieceNotationToBitboardLookup();
}
std::array<Bitboard, 64> Chess::generateKnightMovesBitboardArray()
{
    struct Offset { int dx; int dy; };
    static constexpr Offset offsets[] = { {1,2}, {2,1}, {2,-1}, {1,-2}, {-1,-2}, {-2,-1}, {-2,1}, {-1,2} };

    std::array<Bitboard, 64> result;
    for (int fromIndex = 0; fromIndex < 64; fromIndex++)
    {
        int fromRank = fromIndex / 8;
        int fromFile = fromIndex % 8;

        for (auto [df, dr] : offsets)
        {
            int toRank = fromRank + dr;
            int toFile = fromFile + df;

            if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7) continue;

            int toIndex = toRank * 8 + toFile;
            result[fromIndex] |= 1ULL << toIndex;
        }
    }
    return result;
}
std::array<Bitboard, 64> Chess::generateKingMovesBitboardArray()
{
    struct Offset { int dx; int dy; };
    static constexpr Offset offsets[] = { {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1} };

    std::array<Bitboard, 64> result;
    for (int fromIndex = 0; fromIndex < 64; fromIndex++)
    {
        int fromRank = fromIndex / 8;
        int fromFile = fromIndex % 8;

        for (auto [df, dr] : offsets)
        {
            int toRank = fromRank + dr;
            int toFile = fromFile + df;

            if (toRank < 0 || toRank > 7 || toFile < 0 || toFile > 7) continue;

            int toIndex = toRank * 8 + toFile;
            result[fromIndex] |= 1ULL << toIndex;
        }
    }
    return result;
}
std::array<int, 128> Chess::generatePieceNotationToBitboardLookup()
{
    std::array<int, 128> result;
    result['P'] = WHITE_PAWNS;
    result['N'] = WHITE_KNIGHTS;
    result['B'] = WHITE_BISHOPS;
    result['R'] = WHITE_ROOKS;
    result['Q'] = WHITE_QUEENS;
    result['K'] = WHITE_KING;
    result['p'] = BLACK_PAWNS;
    result['n'] = BLACK_KNIGHTS;
    result['b'] = BLACK_BISHOPS;
    result['r'] = BLACK_ROOKS;
    result['q'] = BLACK_QUEENS;
    result['k'] = BLACK_KING;
    result['0'] = NO_PIECES;
    return result;
}

Chess::~Chess()
{
    delete _grid;
}

void Chess::setUpBoard()
{
    fenToBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    _currentPlayer = WHITE;
    _moves = generateMoves();

    startGame();
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
Bit *Chess::pieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char *pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit *bit = new Bit();
    // should possibly be cached from player class?
    const char *pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(PIECE_SIZE, PIECE_SIZE);

    return bit;
}
std::vector<BitMove> Chess::generateMoves()
{
    std::vector<BitMove> result;
    result.reserve(32);
    std::string state = stateString();

    for (int i = 0; i < E_NUM_BITBOARDS; i++)
        _bitboards[i] = 0;

    for (int i = 0; i < 64; i++)
    {
        char c = state[i];
        uint64_t oneBit = 1ULL << i;

        _bitboards[_pieceNotationToBitboardLookup[c]] |= oneBit;
        if (c != '0')
        {
            _bitboards[ALL_PIECES] |= oneBit;
            _bitboards[isupper(c) ? WHITE_PIECES : BLACK_PIECES] |= oneBit;
        }
    }

    int offset = _currentPlayer == WHITE ? WHITE_PAWNS : BLACK_PAWNS;
    generatePawnMoves(result, _bitboards[WHITE_PAWNS + offset], _bitboards[NO_PIECES], _bitboards[_currentPlayer == WHITE ? BLACK_PIECES : WHITE_PIECES]);
    generateKnightMoves(result, _bitboards[WHITE_KNIGHTS + offset], _bitboards[WHITE_PIECES + offset]);
    generateKingMoves(result, _bitboards[WHITE_KING + offset], _bitboards[WHITE_PIECES + offset]);

    return result;
}
void Chess::generatePawnMoves(std::vector<BitMove> &moves, const Bitboard pawns, const Bitboard noPieces, const Bitboard opponentPieces)
{
    if (pawns.getData() == 0) return;

    static constexpr uint64_t RANK3(0x0000000000FF0000ULL);
    static constexpr uint64_t RANK6(0x0000FF0000000000ULL);
    static constexpr uint64_t NOT_A_FILE(0xFEFEFEFEFEFEFEFEULL);
    static constexpr uint64_t NOT_H_FILE(0x7F7F7F7F7F7F7F7FULL);

    const bool isWhite = _currentPlayer == WHITE;
    const uint64_t pawnsData = pawns.getData();
    const uint64_t noPiecesData = noPieces.getData();
    const uint64_t opponentPiecesData = opponentPieces.getData();

    Bitboard singleMoves = isWhite
        ? (pawnsData << 8) & noPiecesData
        : (pawnsData >> 8) & noPiecesData;

    // must use singleMoves and forward shift once more instead of pawnsData and double forward shift
    // because we need to check each forward rank that there are no pieces in the way
    Bitboard doubleMoves = isWhite
        ? ((singleMoves.getData() & RANK3) << 8) & noPiecesData
        : ((singleMoves.getData() & RANK6) >> 8) & noPiecesData;

    Bitboard leftCaptures = isWhite
        ? ((pawnsData & NOT_A_FILE) << 7) & opponentPiecesData
        : ((pawnsData & NOT_A_FILE) >> 9) & opponentPiecesData;

    Bitboard rightCaptures = isWhite
        ? ((pawnsData & NOT_H_FILE) << 9) & opponentPiecesData
        : ((pawnsData & NOT_H_FILE) >> 7) & opponentPiecesData;

    const int forwardShift = isWhite ? 8 : -8;
    const int doubleForwardShift = isWhite ? 16 : -16;
    const int leftCaptureShift = isWhite ? 7 : -9;
    const int rightCaptureShift = isWhite ? 9 : -7;

    addPawnBitboardMovesToList(moves, singleMoves, forwardShift);
    addPawnBitboardMovesToList(moves, doubleMoves, doubleForwardShift);
    addPawnBitboardMovesToList(moves, leftCaptures, leftCaptureShift);
    addPawnBitboardMovesToList(moves, rightCaptures, rightCaptureShift);
}
void Chess::addPawnBitboardMovesToList(std::vector<BitMove> &moves, const Bitboard bitboard, const int shift)
{
    if (bitboard.getData() == 0) return;

    bitboard.forEachBit([&](int toIndex)
    {
        int fromIndex = toIndex - shift;
        moves.emplace_back(fromIndex, toIndex, Pawn);
    });
}
void Chess::generateKnightMoves(std::vector<BitMove> &moves, const Bitboard knights, const Bitboard playerPieces)
{
    knights.forEachBit([&](int fromIndex)
    {
        Bitboard validMoves = _knightMovesBitboardArray[fromIndex].getData() & ~playerPieces.getData();
        validMoves.forEachBit([&](int toIndex)
            { moves.emplace_back(fromIndex, toIndex, Knight); });
    });
}
void Chess::generateKingMoves(std::vector<BitMove> &moves, const Bitboard king, const Bitboard playerPieces)
{
    king.forEachBit([&](int fromIndex)
    {
        Bitboard validMoves = _kingMovesBitboardArray[fromIndex].getData() & ~playerPieces.getData();
        validMoves.forEachBit([&](int toIndex)
            { moves.emplace_back(fromIndex, toIndex, King); });
    });
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

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor != currentPlayer) return false;

    ChessSquare *square = (ChessSquare *)&src;
    int squareIndex = square->getSquareIndex();
    for (auto move : _moves)
        if (move.from == squareIndex) return true;
    return false;
}
bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare *srcSquare = (ChessSquare *)&src;
    ChessSquare *dstSquare = (ChessSquare *)&dst;
    int srcSquareIndex = srcSquare->getSquareIndex();
    int dstSquareIndex = dstSquare->getSquareIndex();

    for (auto move : _moves)
        if (move.from == srcSquareIndex && move.to == dstSquareIndex) return true;
    return false;
}
void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    _currentPlayer = _currentPlayer == WHITE ? BLACK : WHITE;
    _moves = generateMoves();
    endTurn();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}
bool Chess::checkForDraw()
{
    return false;
}
