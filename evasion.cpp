#include "globals.h"//29/10/26

void Evasion(const int s, const int xs, const int checker);
void CaptureChecker(const int s, const int xs, const int to, const int king);
void AddCapture(const int x, const int sq);
void AddMove(const int x, const int sq, const int flags, const int);
void AddPromotion(const int from, const int to, const int flags);
void AddPromoteCapture(const int x, const int sq);
void GenEp(const int s);
bool IsPinned(const int s, const int sq, const int king, const BITBOARD occ);
bool Attack2(const int s, const int sq, const BITBOARD occ);
int GetChecker(const int s, const int sq);

extern int move_count;

const int DOUBLE_CHECK = 100;

int GetChecker(const int s, const int sq)
{
    int checker = -1;
    int n = 0;
    BITBOARD b1 = bit_pawncaptures[s ^ 1][sq] & bit_pieces[s][P];
    if (b1)
    {
        checker = NextBit(b1);
        n++;
    }
    b1 = bit_moves[N][sq] & bit_pieces[s][N];
    if (b1)
    {
        checker = NextBit(b1);
        n++;
    }
    b1 = bit_moves[R][sq] & (bit_pieces[s][R] | bit_pieces[s][Q]);
    b1 |= (bit_moves[B][sq] & (bit_pieces[s][B] | bit_pieces[s][Q]));
    while (b1)
    {
        int from = NextBit(b1);
        if (!(bit_between[from][sq] & bit_all))
        {
            checker = from;
            n++;
        }
        b1 &= b1 - 1;
    }
    if (n > 1)
        checker = DOUBLE_CHECK;
    return checker;
}

void Evasion(const int s, const int xs, const int checker)
{
    int from, to;

    BITBOARD b1, b2;

    int king = NextBit(bit_pieces[s][K]);

    move_count = first_move[ply];

    if (checker != DOUBLE_CHECK)
    {
        if (board[checker] == P)
            GenEp(s);
        CaptureChecker(s, xs, checker, king);
    }
    b1 = bit_moves[K][king] & bit_units[xs];
    while (b1)
    {
        to = NextBit(b1);
        b1 &= b1 - 1;
        if (!Attack2(xs, to, bit_all & ~mask[king]))
            AddCapture(king, to);
    }
    b1 = bit_moves[K][king] & ~bit_all;
    while (b1)
    {
        to = NextBit(b1);
        b1 &= b1 - 1;
        if (!Attack2(xs, to, bit_all & ~mask[king]))
            AddMove(king, to, 0, 0);
    }
    BITBOARD between = bit_between[checker][king];
    if (between && checker != DOUBLE_CHECK)
    {
        if (s == 0)
        {
            b1 = bit_pieces[s][P] & (between >> 8);
        }
        else
        {
            b1 = bit_pieces[s][P] & (between << 8);
        }
        if (row2[xs][king] == 4)
        {
            if (s == 0)
            {
                b2 = bit_pieces[s][P] & (between << 16);
            }
            else
            {
                b2 = bit_pieces[s][P] & (between << 16);
            }
            while (b2)
            {
                from = NextBit(b2);
                b2 &= b2 - 1;
                if (row2[s][from] == 1 && board[pawnplus[s][from]] == EMPTY)
                {
                    AddMove(from, pawndouble[s][from], 0, 0);
                }
            }
        }
        while (b1)
        {
            from = NextBit(b1);
            b1 &= b1 - 1;
            if (row2[s][from] == 6)
                AddPromotion(from, pawnplus[s][from], PROMOTE);
            else
                AddMove(from, pawnplus[s][from], 0, 0);
        }
        b1 = bit_pieces[s][N];
        while (b1)
        {
            from = NextBit(b1);
            b1 &= b1 - 1;
            b2 = bit_moves[N][from] & between;
            while (b2)
            {
                to = NextBit(b2);
                b2 &= b2 - 1;
                AddMove(from, to, 0, 0);
            }
        }
        for (int piece = B; piece <= Q; piece++)
        {
            b1 = bit_pieces[s][piece];
            while (b1)
            {
                from = NextBit(b1);
                b1 &= b1 - 1;
                b2 = bit_moves[piece][from] & between;
                while (b2)
                {
                    to = NextBit(b2);
                    b2 &= b2 - 1;
                    if (!(bit_between[from][to] & bit_all))
                    {
                        AddMove(from, to, 0, 0);
                    }
                    b2 &= bit_after[from][to];
                }
            }
        }
    }
    first_move[ply + 1] = move_count;
}

void CaptureChecker(const int s, const int xs, const int to, const int king)
{
    if (bit_left[xs][to] & bit_pieces[s][P])
    {
        int from = pawnleft[xs][to];
        if (row2[s][from] == 6)
            AddPromoteCapture(from, to);
        else
            AddCapture(from, to);
    }
    if (bit_right[xs][to] & bit_pieces[s][P])
    {
        int from = pawnright[xs][to];
        if (row2[s][from] == 6)
            AddPromoteCapture(from, to);
        else
            AddCapture(from, to);
    }
    BITBOARD b1 = bit_moves[N][to] & bit_pieces[s][N];
    while (b1)
    {
        int from = NextBit(b1);
        AddCapture(from, to);
        b1 &= b1 - 1;
    }
    for (int piece = B; piece < K; piece++)
    {
        b1 = bit_moves[piece][to] & bit_pieces[s][piece];
        while (b1)
        {
            int from = NextBit(b1);
            if (!(bit_between[from][to] & bit_all))
            {
                AddCapture(from, to);
            }
            b1 &= b1 - 1;
        }
    }
}