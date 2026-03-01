#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif
#include <iostream>

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

class Bitboard {
  public:
    Bitboard() : _data(0) {}
    Bitboard(uint64_t data) : _data(data) {}

    uint64_t getData() const { return _data; }
    void setData(uint64_t data) { _data = data; }

    Bitboard &operator|=(const uint64_t other) {
        _data |= other;
        return *this;
    }

    template <typename Func>
    void forEachBit(Func func) const {
        if (_data != 0) {
            uint64_t tempData = _data;
            while (tempData) {
                int index = bitScanForward(tempData);
                func(index);
                tempData &= tempData - 1;
            }
        }
    }

    void printBitboard() {
        std::cout << "\n  a b c d e f g h\n";
        for (int rank = 7; rank >= 0; rank--) {
            std::cout << (rank + 1) << " ";
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                if (_data & (1ULL << square)) {
                    std::cout << "X ";
                } else {
                    std::cout << ". ";
                }
            }
            std::cout << (rank + 1) << "\n";
            std::cout << std::flush;
        }
        std::cout << "  a b c d e f g h\n";
        std::cout << std::flush;
    }

private:
    uint64_t _data;

    inline int bitScanForward(uint64_t bb) const {
#if defined(_MSC_VER) && !defined(__clang__)
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
#else
        return __builtin_ffsll(bb) - 1;
#endif
    };
};

struct BitMove {
    uint8_t from;
    uint8_t to;
    uint8_t piece;

    BitMove() : from(0), to(0), piece(None) {}
    BitMove(int from, int to, ChessPiece piece) : from(from), to(to), piece(piece) {}
        
    bool operator==(const BitMove& other) const {
        return from == other.from && to == other.to && piece == other.piece;
    }
};