#include "globals.h"

int LowestAttacker(const int s, const int xs, const int sq, const BITBOARD occ);
bool Attack2(const int s, const int sq, const BITBOARD occ);
bool IsPinned(const int s, const int sq, const int king, const BITBOARD occ);
bool IsLineAttack(const int s, const int sq, const BITBOARD occ);
int GetChecker(const int s, const int sq);

bool Attack(const int s, const int sq)
{
    if (bit_pawncaptures[s ^ 1][sq] & bit_pieces[s][P])
        return true;
    if (bit_moves[N][sq] & bit_pieces[s][N])
        return true;

    BITBOARD b1 = bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]);
    b1 |= (bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q]));
    while (b1)
    {
        int from = NextBit(b1);
        if (!(bit_between[from][sq] & bit_all))
            return true;
        b1 &= b1 - 1;
    }
    if (bit_moves[K][sq] & bit_pieces[s][K])
        return true;
    return false;
}

bool Attack2(const int s, const int sq, const BITBOARD occ)
{
    if (bit_pawncaptures[s ^ 1][sq] & bit_pieces[s][P])
        return true;
    if (bit_moves[N][sq] & bit_pieces[s][N])
        return true;

    BITBOARD b1 = bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]);
    b1 |= (bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q]));
    while (b1)
    {
        int from = NextBit(b1);
        if (!(bit_between[from][sq] & occ))
            return true;
        b1 &= b1 - 1;
    }
    if (bit_moves[K][sq] & bit_pieces[s][K])
        return true;
    return false;
}

int LowestAttacker(const int s, const int xs, const int sq, const BITBOARD occ)
{
    const int king = NextBit(bit_pieces[s][K]);

    if (bit_left[xs][sq] & bit_pieces[s][P] & occ)
    {
        int from = pawnleft[xs][sq];
        if (!IsPinned(xs, from, king, occ))
            return from;
    }

    if (bit_right[xs][sq] & bit_pieces[s][P] & occ)
    {
        int from = pawnright[xs][sq];
        if (!IsPinned(xs, from, king, occ))
            return from;
    }

    BITBOARD b1;
    b1 = bit_moves[N][sq] & bit_pieces[s][N] & occ;
    while (b1)
    {
        int from = NextBit(b1);
        if (!IsPinned(xs, from, king, occ))
            return from;
        b1 &= b1 - 1;
    }

    for (int piece = B; piece < K; piece++)
    {
        b1 = bit_moves[piece][sq] & bit_pieces[s][piece] & occ;
        while (b1)
        {
            int from = NextBit(b1);
            if (!(bit_between[from][sq] & occ))
            {
                if (!IsPinned(xs, from, king, occ))
                    return from;
            }
            b1 &= b1 - 1;
        }
    }
    b1 = bit_moves[K][sq] & bit_pieces[s][K];
    if (b1)
    {
        if (!(Attack2(xs, sq, occ)))
            return NextBit(b1);
    }
    return -1;
}

bool IsPinned2(const int s, const int sq, const int to, const int king, const BITBOARD occ)
{
    BITBOARD b1 = bit_moves[R][king] & bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]) & occ;
    b1 |= (bit_moves[B][king] & bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q])) & occ;
    while (b1)
    {
        int from = NextBit(b1);
        if (from != to && !(bit_between[from][king] & occ & ~mask[sq]))
            return true;
        b1 &= b1 - 1;
    }
    return false;
}

bool IsPinned(const int s, const int sq, const int king, const BITBOARD occ)
{
    BITBOARD b1 = bit_moves[R][king] & (bit_pieces[s][R] | bit_pieces[s][Q]) & occ;
    b1 |= (bit_moves[B][king] & (bit_pieces[s][B] | bit_pieces[s][Q])) & occ;
    while (b1)
    {
        int from = NextBit(b1);
        if (!(bit_between[from][king] & occ & ~mask[sq]))
            return true;
        b1 &= b1 - 1;
    }
    return false;
}

bool IsLineAttack(const int s, const int sq, const BITBOARD occ)
{
    BITBOARD b1 = bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]);
    b1 |= (bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q]));
    while (b1)
    {
        int from = NextBit(b1);
        if (!(bit_between[from][sq] & occ))
            return true;
        b1 &= b1 - 1;
    }
    return false;
}


