#include <iostream>
#include "Block.h"

Block::Block(int row, int col, int height, int colSize) {
  this->row = row;
  this->col = col;
  this->height = height;
  this->numGapRows = 0;
  gapRows = (int*) malloc(colSize * sizeof(int));
}

Block::~Block() {
  if(gapRows != NULL) {
    free(gapRows);
    gapRows = NULL;
  }
}

int Block::inGaps(int row) {
  for(int i = 0; i < numGapRows; i++) {
    if(row == gapRows[i]) {
      return i ;
    }
  }
  return -1;
}

void Block::merge(Block* other) {
  // not in the same col
  if (this->col != other->col) {
    return;
  }
   
  // this block is lower than other block
  if (this->row < other->row) {
    // if other block lies in the gap  
    int idx;
    if((idx = inGaps(other->row)) != -1) {
      // shift array values
      for(int i = idx + other->height; i < numGapRows; i++) {
        gapRows[i - other->height] = gapRows[i]; 
      }
      numGapRows -= other->height;
      return;
    }
    int gap = other->row - (this->row + this->height);
    if (gap > 0) {
      if (this->gapRows == NULL) {
        std::cout << "gapRows ptr is NULL!!!" << std::endl;
      }
      int thisRow = this->row + this->height;
      while (thisRow < other->row) {
        gapRows[numGapRows++] = thisRow++;
      }
      for (int i = 0; i < other->numGapRows; i++) {
        gapRows[numGapRows++] = other->gapRows[i];
      }
    }
    this->height = other->row - this->row + other->height;
  } else {
    int temp = numGapRows;
    int *arr = (int*)malloc(sizeof(int) * temp);
    for(int i = 0; i < temp; i++) {
      arr[i] = gapRows[i];
    }
    // NEVER RUN!
    for (int i = 0; i < other->numGapRows; i++) {
      gapRows[i] = other->gapRows[i];
    }
    numGapRows = other->numGapRows; 
    int gap = this->row - (other->row + other->height);
    if (gap > 0) {
      if (this->gapRows == NULL) {
        std::cout << "gapRows ptr is NULL!!!" << std::endl;
      }
      int thisRow = other->row + other->height;
      while (thisRow < this->row) {
        gapRows[numGapRows++] = thisRow++;
      }
      // copy the original back
      for(int i =0; i < temp; i++) {
        gapRows[numGapRows++] = arr[i];
      }
    } 
    this->height = this->row - other->row + this->height;
    // new low row
    this->row = other->row;
    // free
    free(arr);
    arr = NULL;
  }
}
