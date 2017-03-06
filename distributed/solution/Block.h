#ifndef __BLOCK_H
#define __BLOCK_H

class Block {
  public:
    Block(int row, int col, int height, int colSize);
    ~Block();
    // the lowest row represented in this Block
    int row;
    // the column where the template is
    int col;
    // the row above the highest row represented in this FireInfo
    int height;
    // holds rows of gaps
    int* gapRows;
    // number of gaps
    int numGapRows;
    // merge blocks on the same column
    void merge(Block* other);

    int inGaps(int row);
};

#endif // __BLOCK_H
