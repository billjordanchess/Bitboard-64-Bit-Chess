#include <stdlib.h>
#include <assert.h>
#include <intrin.h>
#include <cstdint>

using namespace std;

#include "globals.h"//29/10/26

BITBOARD bit_pawncaptures[2][64];
BITBOARD bit_left[2][64];
BITBOARD bit_right[2][64];

BITBOARD bit_moves[6][64];

BITBOARD bit_pieces[2][7];
BITBOARD bit_units[2];
BITBOARD bit_all;

BITBOARD bit_between[64][64];
BITBOARD bit_after[64][64];
BITBOARD bit_beyond[64][64];
BITBOARD bit_ray_edge[6][64];

BITBOARD mask_passed[2][64];
BITBOARD mask_path[2][64];

BITBOARD mask[64];
BITBOARD mask_cols[64];
BITBOARD mask_isolated[64];

BITBOARD mask_kingside;
BITBOARD mask_queenside;
BITBOARD not_a_file;
BITBOARD not_h_file;
BITBOARD mask_edges;

int pawnplus[2][64];
int pawndouble[2][64];
int pawnleft[2][64];
int pawnright[2][64];

int row2[2][64];

BITBOARD bit_rings[3][64][6];

static void SetMasks();
static void SetRanks();
static void SetBetweenVector();
static int GetEdge(int sq, int plus);

static void SetBit(BITBOARD& bb, int square);

static void SetPawnArrays();
static void SetPawnBits();
static void SetPassedBits();
static void SetBitAfter();

static void SetKnightMoves();
static void SetLinePieces();
static void SetKingMoves();
static void SetMaskEdges();
static void SetRayEdges();
static void SetBitRings();

int NextBit(BITBOARD bb);
void PrintBitBoard(BITBOARD bb);
void PrintCell(int x, BITBOARD bb);

static void SetBit(BITBOARD& bb, int square)
{
    bb |= (1ui64 << square);
}

void SetBits()
{
    memset(bit_pawncaptures, 0, sizeof(bit_pawncaptures));
    memset(bit_moves, 0, sizeof(bit_moves));

    SetRanks();
    SetMasks();
    SetPawnArrays();
    SetPawnBits();
    SetPassedBits();
    SetBetweenVector();
    SetBitAfter();
    SetMaskEdges();
    SetMoves();
    SetRayEdges();
    SetBitRings();
}

static void SetMasks()
{
    for (int x = 0; x < 64; x++)
    {
        SetBit(mask[x], x);
    }
    for (int x = 0; x < 64; x++)
    {
        if (col[x] < 2)
            SetBit(mask_queenside, x);
        if (col[x] > 5)
            SetBit(mask_kingside, x);
    }
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
            if (col[x] == col[y])
                SetBit(mask_cols[x], y);
        }
    not_a_file = ~mask_cols[0];
    not_h_file = ~mask_cols[7];
}

static void SetPawnArrays()
{
    for (int x = 0; x < 64; x++)
    {
        pawnleft[0][x] = -1;
        pawnleft[1][x] = -1;
        pawnright[0][x] = -1;
        pawnright[1][x] = -1;
        if (col[x] > 0)
        {
            if (row[x] < 7) { pawnleft[0][x] = x + 7; }
            if (row[x] > 0) { pawnleft[1][x] = x - 9; }
        }
        if (col[x] < 7)
        {
            if (row[x] < 7) { pawnright[0][x] = x + 9; }
            if (row[x] > 0) { pawnright[1][x] = x - 7; }
        }
        if (row[x] < 7)
            pawnplus[0][x] = x + 8;
        if (row[x] < 6)
            pawndouble[0][x] = x + 16;
        if (row[x] > 0)
            pawnplus[1][x] = x - 8;
        if (row[x] > 1)
            pawndouble[1][x] = x - 16;
    }
}

static void SetPawnBits()
{
    for (int x = 0; x < 64; x++)
    {
        if (col[x] > 0)
        {
            if (row[x] < 7)
            {
                SetBit(bit_pawncaptures[0][x], pawnleft[0][x]);
                SetBit(bit_left[0][x], pawnleft[0][x]);
            }
            if (row[x] > 0)
            {
                SetBit(bit_pawncaptures[1][x], pawnleft[1][x]);
                SetBit(bit_left[1][x], pawnleft[1][x]);
            }
        }
        if (col[x] < 7)
        {
            if (row[x] < 7)
            {
                SetBit(bit_pawncaptures[0][x], pawnright[0][x]);
                SetBit(bit_right[0][x], pawnright[0][x]);
            }
            if (row[x] > 0)
            {
                SetBit(bit_pawncaptures[1][x], pawnright[1][x]);
                SetBit(bit_right[1][x], pawnright[1][x]);
            }
        }
    }
}

static void SetPassedBits()
{
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
            if (abs(col[x] - col[y]) < 2)
            {
                if (row[x] < row[y] && row[y] < 7)
                    SetBit(mask_passed[WHITE][x], y);
                if (row[x] > row[y] && row[y] > 0)
                    SetBit(mask_passed[BLACK][x], y);
            }
            if (abs(col[x] - col[y]) == 1)
            {
                SetBit(mask_isolated[x], y);
            }
            if (col[x] == col[y])
            {
                if (row[x] < row[y])
                    SetBit(mask_path[WHITE][x], y);
                if (row[x] > row[y])
                    SetBit(mask_path[BLACK][x], y);
            }
        }
}

static void SetBetweenVector()
{
    int x, y, z;
    for (x = 0; x < 64; x++)
        for (y = 0; y < 64; y++)
        {
            if (row[x] == row[y])
            {
                if (y > x)
                    for (z = x + 1; z < y; z++)
                        SetBit(bit_between[x][y], z);
                else
                    for (z = y + 1; z < x; z++)
                        SetBit(bit_between[x][y], z);
            }
            if (col[x] == col[y])
            {
                if (y > x)
                    for (z = x + 8; z < y; z += 8)
                        SetBit(bit_between[x][y], z);
                else
                    for (z = y + 8; z < x; z += 8)
                        SetBit(bit_between[x][y], z);
            }
            if (nediag[x] == nediag[y])
            {
                if (y > x)
                    for (z = x + 7; z < y; z += 7)
                        SetBit(bit_between[x][y], z);
                else
                    for (z = y + 7; z < x; z += 7)
                        SetBit(bit_between[x][y], z);
            }
            if (nwdiag[x] == nwdiag[y])
            {
                if (y > x)
                    for (z = x + 9; z < y; z += 9)
                        SetBit(bit_between[x][y], z);
                else
                    for (z = y + 9; z < x; z += 9)
                        SetBit(bit_between[x][y], z);
            }
        }
}

static void SetBitAfter()
{
    int x, y, z;
    for (x = 0; x < 64; x++)
        for (y = 0; y < 64; y++)
        {
            if (x == y)
                continue;
            if (row[x] == row[y])
            {
                if (y > x)
                    for (z = y; z <= row[y] * 8 + 7; z++)
                        SetBit(bit_after[x][y], z);
                else
                    for (z = y; z >= row[y] * 8; z--)
                        SetBit(bit_after[x][y], z);
            }
            if (col[x] == col[y])
            {
                if (y > x)
                    for (z = y; z <= 56 + col[y]; z += 8)
                        SetBit(bit_after[x][y], z);
                else
                    for (z = y; z >= col[y]; z -= 8)
                        SetBit(bit_after[x][y], z);
            }
            if (nediag[x] == nediag[y])
            {
                if (y > x)
                    for (z = y; z <= GetEdge(x, 7); z += 7)
                        SetBit(bit_after[x][y], z);
                else
                    for (z = y; z >= GetEdge(x, -7); z -= 7)
                        SetBit(bit_after[x][y], z);
            }
            if (nwdiag[x] == nwdiag[y])
            {
                if (y > x)
                    for (int z = y; z <= GetEdge(x, 9); z += 9)
                        SetBit(bit_after[x][y], z);
                else
                    for (int z = y; z >= GetEdge(x, -9); z -= 9)
                        SetBit(bit_after[x][y], z);
            }
        }
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
            bit_after[x][y] = ~bit_after[x][y];
        }
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
        {
            if (~bit_after[x][y])
            {
                bit_beyond[x][y] = ~(bit_after[x][y] ^ mask[y]);
            }
            else
                bit_beyond[x][y] = 0;
        }
}

static void SetMaskEdges()
{
    mask_edges = 0;
    for (int x = 0; x < 64; x++)
    {
        if (col[x] == 0 || col[x] == 7 || row[x] == 0 || row[x] == 7)
        {
            mask_edges |= mask[x];
        }
    }
}

static void SetMoves()
{
    SetKnightMoves();
    SetLinePieces();
    SetKingMoves();
}

static void SetKnightMoves()
{
    for (int x = 0; x < 64; x++)
    {
        bit_moves[N][x] = 0;
        if (row[x] < 6 && col[x] < 7)
            bit_moves[N][x] |= mask[x + 17];
        if (row[x] < 7 && col[x] < 6)
            bit_moves[N][x] |= mask[x + 10];
        if (row[x] < 6 && col[x]>0)
            bit_moves[N][x] |= mask[x + 15];
        if (row[x] < 7 && col[x]>1)
            bit_moves[N][x] |= mask[x + 6];
        if (row[x] > 1 && col[x] < 7)
            bit_moves[N][x] |= mask[x - 15];
        if (row[x] > 0 && col[x] < 6)
            bit_moves[N][x] |= mask[x - 6];
        if (row[x] > 1 && col[x] > 0)
            bit_moves[N][x] |= mask[x - 17];
        if (row[x] > 0 && col[x] > 1)
            bit_moves[N][x] |= mask[x - 10];
    }
}

static void SetKingMoves()
{
    for (int x = 0; x < 64; x++)
    {
        if (col[x] > 0)
            bit_moves[K][x] |= mask[x - 1];
        if (col[x] < 7)
            bit_moves[K][x] |= mask[x + 1];
        if (row[x] > 0)
            bit_moves[K][x] |= mask[x - 8];
        if (row[x] < 7)
            bit_moves[K][x] |= mask[x + 8];
        if (col[x] < 7 && row[x] < 7)
            bit_moves[K][x] |= mask[x + 9];
        if (col[x] > 0 && row[x] < 7)
            bit_moves[K][x] |= mask[x + 7];
        if (col[x] > 0 && row[x] > 0)
            bit_moves[K][x] |= mask[x - 9];
        if (col[x] < 7 && row[x]>0)
            bit_moves[K][x] |= mask[x - 7];
    }
}

static void SetLinePieces()
{
    for (int x = 0; x < 64; x++)
    {
        bit_moves[B][x] = 0;
        bit_moves[R][x] = 0;
        bit_moves[Q][x] = 0;

        for (int y = 0; y < 64; y++)
        {
            if (x == y)
                continue;
            if (nwdiag[x] == nwdiag[y] || nediag[x] == nediag[y])
                bit_moves[B][x] |= mask[y];
            if (row[x] == row[y] || col[x] == col[y])
                bit_moves[R][x] |= mask[y];
            if (nwdiag[x] == nwdiag[y] || nediag[x] == nediag[y] || row[x] == row[y] || col[x] == col[y])
                bit_moves[Q][x] |= mask[y];
        }
    }
}

static void SetRayEdges()
{
    for (int x = 0; x < 64; x++)
    {
        bit_ray_edge[B][x] = bit_moves[B][x] & mask_edges;
    }
    for (int x = 0; x < 64; x++)
    {
        bit_ray_edge[R][x] = bit_moves[R][x] & mask_edges;
        if (col[x] == 0)
        {
            bit_ray_edge[R][x] &= ~bit_between[A1][A8];
        }
        else if (col[x] == 7)
        {
            bit_ray_edge[R][x] &= ~bit_between[H1][H8];
        }
        else if (row[x] == 0)
        {
            bit_ray_edge[R][x] &= ~bit_between[A1][H1];
        }
        else if (row[x] == 7)
        {
            bit_ray_edge[R][x] &= ~bit_between[A8][H8];
        }
    }

    bit_ray_edge[R][A1] = mask[A8] | mask[H1];
    bit_ray_edge[R][H8] = mask[A8] | mask[H1];
    bit_ray_edge[R][H1] = mask[A1] | mask[H8];
    bit_ray_edge[R][A8] = mask[A1] | mask[H8];

    for (int x = 0; x < 64; x++)
    {
        bit_ray_edge[Q][x] = bit_ray_edge[B][x] | bit_ray_edge[R][x];
    }
}

static int GetEdge(int sq, int plus)
{
    do
    {
        sq += plus;
    } while (col[sq] > 0 && col[sq] < 7 && row[sq] > 0 && row[sq] < 7);
    return sq;
}

static void SetRanks()
{
    for (int x = 0; x < 64; x++)
    {
        row2[WHITE][x] = row[x];
        row2[BLACK][x] = 7 - row[x];
    }
}

void SetBitRings()
{
    BITBOARD b3 = 0;
    for (int x = 0; x < 64; x++)
    {
        BITBOARD b1 = mask[x];
        b3 = 0;
        for (int d = 0; d < 6; d++)
        {
            int plus = d + 1;
            BITBOARD b2 = b1 >> plus | b1 << plus | b1 >> (8 * plus) | b1 << (8 * plus);
            bit_rings[1][x][d] = b2 & bit_moves[R][x] & ~bit_ray_edge[R][x];
            b3 |= bit_rings[1][x][d];
        }
    }

    for (int x = 0; x < 64; x++)
    {
        BITBOARD b1 = mask[x];
        b3 = 0;
        for (int d = 0; d < 6; d++)
        {
            int plus = d + 1;
            BITBOARD b2 = b1 >> (7 * plus) | b1 << (7 * plus) | b1 >> (9 * plus) | b1 << (9 * plus);
            bit_rings[0][x][d] = b2 & bit_moves[B][x] & ~bit_ray_edge[B][x];
            b3 |= bit_rings[0][x][d];
        }
    }
    for (int x = 0; x < 64; x++)
    {
        BITBOARD b1 = mask[x];
        for (int d = 0; d < 6; d++)
        {
            bit_rings[2][x][d] = bit_rings[0][x][d] | bit_rings[1][x][d];
        }
    }
}

int NextBit(BITBOARD bb)
{
    unsigned long sq;
    _BitScanForward64(&sq, bb);   // bb must be non-zero
    return sq;
}

void PrintBitBoard(BITBOARD bb)
{
    cout << endl;
    int x;
    for (x = 56; x < 64; x++)
        PrintCell(x, bb);
    for (x = 48; x < 56; x++)
        PrintCell(x, bb);
    for (x = 40; x < 48; x++)
        PrintCell(x, bb);
    for (x = 32; x < 40; x++)
        PrintCell(x, bb);
    for (x = 24; x < 32; x++)
        PrintCell(x, bb);
    for (x = 16; x < 24; x++)
        PrintCell(x, bb);
    for (x = 8; x < 16; x++)
        PrintCell(x, bb);
    for (x = 0; x < 8; x++)
        PrintCell(x, bb);
}

void PrintCell(int x, BITBOARD bb)
{
    if (mask[x] & bb)
        cout << " X";
    else
        cout << " -";
    if ((x + 1) % 8 == 0)
        cout << endl;
}


