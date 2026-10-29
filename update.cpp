#include "globals.h"

void UpdatePiece(const int s, const int piece, const int start, const int to);
void RemovePiece(const int s, const int piece, const int sq);
void AddPiece(const int s, const int piece, const int sq);

bool IsLineAttack(const int s, const int sq, const BITBOARD occ);
bool Attack2(const int s, const int sq, const BITBOARD occ);
bool IsPinned2(const int s, const int from, const int to, const int king, const BITBOARD occ);

void UpdatePiece(const int s, const int piece, const int from, const int to)
{
    assert(piece < 6);
    AddKey(s, piece, from);
    AddKey(s, piece, to);
    board[to] = piece;
    board[from] = EMPTY;
    bit_pieces[s][piece] &= ~mask[from];
    bit_pieces[s][piece] |= mask[to];
    bit_units[s] &= ~mask[from];
    bit_units[s] |= mask[to];
    bit_all = bit_units[WHITE] | bit_units[BLACK];
}

void RemovePiece(const int s, const int piece, const int sq)
{
    assert(piece < 6);
    AddKey(s, piece, sq);
    board[sq] = EMPTY;
    bit_pieces[s][piece] &= ~mask[sq];
    bit_units[s] &= ~mask[sq];
    bit_all = bit_units[WHITE] | bit_units[BLACK];
}

void AddPiece(const int s, const int piece, const int sq)
{
    assert(piece < 6);
    board[sq] = piece;
    AddKey(s, piece, sq);
    bit_pieces[s][piece] |= mask[sq];
    bit_units[s] |= mask[sq];
    bit_all = bit_units[0] | bit_units[1];
}

int MakeMove(move_data m)
{
    const int from = m.from;
    const int to = m.to;
    const int flags = m.flags;

    if (board[from] != K)
    {
        int king = NextBit(bit_pieces[side][K]);
        if (IsPinned2(xside, from, to, king, (bit_all & ~mask[from]) | mask[to]))
        {
            return false;
        }
    }
    else
    {
    if (flags & CASTLE)
    {
        if (Attack(xside, from) || Attack(xside, to))
            return false;
        if (to == G1)
        {
            if (Attack(xside, F1))
                return false;
            UpdatePiece(side, R, H1, F1);
        }
        else if (to == C1)
        {
            if (Attack(xside, D1))
                return false;
            UpdatePiece(side, R, A1, D1);
        }
        else if (to == G8)
        {
            if (Attack(xside, F8))
                return false;
            UpdatePiece(side, R, H8, F8);
        }
        else if (to == C8)
        {
            if (Attack(xside, D8))
                return false;
            UpdatePiece(side, R, A8, D8);
        }
    }
}

    game_data* g = &game_list[hply];
    g->from = from;
    g->to = to;
    g->flags = flags;
    g->capture = board[to];
    g->fifty = fifty;
    g->hashkey = currentkey;
    g->lock = currentlock;
    g->castle = castle;
    castle &= castle_mask[from] & castle_mask[to];
    ply++;
    hply++;
    fifty++;

    if (board[from] == P)
    {
        fifty = 0;
        if (flags & EP)
        {
            RemovePiece(xside, P, pawnplus[xside][to]);
        }
    }
    if (flags & CAPTURE)
    {
        fifty = 0;
        RemovePiece(xside, board[to], to);
    }
    if (flags & PROMOTE)
    {
        RemovePiece(side, P, from);
        AddPiece(side, Q, to);
    }
    else
    {
        UpdatePiece(side, board[from], from, to);
    }

    side ^= 1;
    xside ^= 1;
    return true;
}

void UnMakeMove()
{
    side ^= 1;
    xside ^= 1;
    ply--;
    hply--;

    game_data* m = &game_list[hply];
    int from = m->from;
    int to = m->to;
    int flags = m->flags;

    fifty = m->fifty;
    castle = m->castle;

    if (flags & EP)
        AddPiece(xside, P, pawnplus[xside][to]);
    if (flags & PROMOTE)
    {
        AddPiece(side, P, from);
        RemovePiece(side, board[to], to);
    }
    else
        UpdatePiece(side, board[to], to, from);
    if (flags & CAPTURE)
        AddPiece(xside, m->capture, to);
    if (flags & CASTLE)
    {
        if (to == G1)
            UpdatePiece(side, R, F1, H1);
        else if (to == C1)
            UpdatePiece(side, R, D1, A1);
        else if (to == G8)
            UpdatePiece(side, R, F8, H8);
        else if (to == C8)
            UpdatePiece(side, R, D8, A8);
    }
}

