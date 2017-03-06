#include <stdlib.h>
#include <stdbool.h>

#include "array2d.h"

// need this to keep an array of error codes and prompt messages
static char* errors[4] = {"No Error", "***ERROR: Array Pointer is NULL! Check memory allocation!", "***ERROR: The indices given are out of bound! Not safe to use!", "***ERROR: This index is not initialized! Not safe to use before set!"};


char* array2d_errorMessage(int code) {
	if(code < 0 || code > 3) {
		return "THIS SHOULD NEVER HAPPEN! CHECK ERROR CODE!";
	}
	// return the corresponding error message
	// the code is set up to be equal to the index in the array
	return errors[code];
}

bool array2d_rowBoundCheck(const array2d arr, int row) {
	if(arr == NULL) {
		return false;
	}
	
	if(row >= arr->numRows || row < 0) {
		return false;
	}
	return true;
}

bool array2d_columnBoundCheck(const array2d arr, int col) {
	if(arr == NULL) {
		return false;
	}
	
	if(col >= arr->numCols || col < 0) {
		return false;
	}
	return true;
}

bool array2d_boundCheck(array2d arr, int row, int col) {
	return array2d_rowBoundCheck(arr, row) && array2d_columnBoundCheck(arr, col);
}

array2d array2d_new(int row, int col) {
	// allocate space for the basic information of the 2-d array
	array2d arr = (array2d) malloc(sizeof(struct array2D));
	if(arr == NULL) {
		// out of memory
		return NULL;
	}
	// allocate space for the data
	arr->data = (payload_t) malloc(sizeof(payload_t) * row * col);
	if(arr->data == NULL) {
		return NULL;
	}
	arr->numRows = row;
	arr->numCols = col;
	// initialize the boolean array to false
	arr->visited = (bool*) malloc(sizeof(bool) * row * col);
	for(int i = 0; i < row * col ; i++) {
		arr->visited[i] = false;
	}
	return arr;
}

int array2d_get(array2d arr, payload_t* payload_ptr, int row, int col) {

	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	// if visited[index] is false, it's not safe to get the value
	if(!arr->visited[row * arr->numCols + col]) {
		return E_USEBEFORESET;
	}
	if(array2d_boundCheck(arr, row, col)) {
		// get the data
		payload_t *data = arr->data;
		// store the pointer into payload_ptr
		*payload_ptr = data[row * arr->numCols + col];
		return E_SUCCESS;
	}
	return E_OUTOFBOUND;
}

int array2d_update(array2d arr, payload_t value, int row, int col) {
	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	// bound check
	if(array2d_boundCheck(arr, row, col)) {
		// get the data
		payload_t *data = arr->data;
		// if there's an old value, free the memory first
		if(arr->visited[row * arr->numCols + col]) {
			free(data[row * arr->numCols + col]);
		}
		// change the value
		data[row * arr->numCols + col] = value;
		arr->visited[row * arr->numCols + col] = true;
		return E_SUCCESS;
	}
	return E_OUTOFBOUND;
}

int array2d_swap(array2d arr, int row1, int col1, int row2, int col2) {
	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	// visited check: not safe to swap items not visited
	if(!arr->visited[row1 * arr->numCols + col1] || !arr->visited[row2 * arr->numCols + col2]) {
		return E_USEBEFORESET;
	}
	// bound check and 
	if(array2d_boundCheck(arr, row1, col1) && array2d_boundCheck(arr, row2, col2)) {
		// get the data
		payload_t *data = arr->data;
		// save a temp value
		payload_t temp = data[row1 * arr->numCols + col1];
		data[row1 * arr->numCols + col1] = data[row2 * arr->numCols + col2];
		data[row2 * arr->numCols + col2] = temp;
		return E_SUCCESS;
	}	
	return E_OUTOFBOUND;
}

int array2d_destroy(array2d arr, PayloadFreeFnPtr payload_free_function) {
	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	// get the data
	payload_t *data = arr->data;
        // free each payload
	for(int i = 0; i < arr->numRows; i++) {
		for(int j =0 ; j < arr->numCols; j++) {
			// client provides this free function
			payload_free_function(data[i * arr->numCols + j]);
		}
	}
	// free the data memory
	free(arr->data);
	// free the boolean array
	free(arr->visited);
	// free the 2-d array
	free(arr);
	return E_SUCCESS;
}

int array2d_deserialize(array2d *testArray, json_t *json, json_t **arrayValue) {
	// information of the array
	int arrayRow;
	int arrayCol;
	// unpack json file
	json_unpack(json, "{s:i, s:i, s:o}", "rows", &arrayRow, "columns", &arrayCol, "data", arrayValue);
	// create new array
  *testArray = array2d_new(arrayRow, arrayCol);
	if(*testArray == NULL) {
		return E_NULLPOINTER;
	}
	return E_SUCCESS;
}

json_t* array2d_serialize(array2d testArray, json_t *array) {
	// packing to file
	json_t* packed = json_pack("{sisiso}", "rows", testArray->numRows, "columns", testArray->numCols, "data", array);
	return packed;
}

void array2d_serializeAndDump(array2d testArray, const char* filepath, json_t *array) {
	json_t* packed = array2d_serialize(testArray, array);
	// output json file
	json_dump_file(packed, filepath, 0);
	// deference json
	json_decref(packed);
}

int array2d_getRowSize(array2d arr) {
	if(arr == NULL) {
		// Do not return E_NULLPOINTER here, E_NULLPOINTER == 1, which can be confusing
		// if this an error ONE or size ONE
		return -1;
	}
	return arr->numRows;
}

int array2d_getColumnSize(array2d arr) {
	if(arr == NULL) {
		// Do not return E_NULLPOINTER here, E_NULLPOINTER == 1, which can be confusing
		// if this an error ONE or size ONE
		return -1;
	}
	return arr->numCols;
}

int array2d_getTotalSize(array2d arr) {
	if(arr == NULL) {
		// Do not return E_NULLPOINTER here, E_NULLPOINTER == 1, which can be confusing
		// if this an error ONE or size ONE
		return -1;
	}
	return array2d_getRowSize(arr) * array2d_getColumnSize(arr);
}


int array2d_getRow(array2d arr, int row, payload_t* rtnArray) {
	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	if(!array2d_rowBoundCheck(arr, row)) {
		return E_OUTOFBOUND;
	}
	// get the data
	payload_t *data = arr->data;
	// load the array
	for(int i=0; i < arr->numCols; i++) {
		rtnArray[i] = data[row * arr->numCols + i];
	}
	return E_SUCCESS;
}

int array2d_getColumn(array2d arr, int col, payload_t* rtnArray) {
	if(arr == NULL) {
		return E_NULLPOINTER;
	}
	if(!array2d_columnBoundCheck(arr, col)) {
		return E_OUTOFBOUND;
	}
	// get the data
	payload_t *data = arr->data;
	// load the array
	for(int i=0; i < arr->numRows; i++) {
		rtnArray[i] = data[col + i * arr->numCols];
	}
	return E_SUCCESS;
}


payload_t* getAround(array2d arr, int row, int col) {
	payload_t* around = (payload_t*) malloc(sizeof(payload_t*) * 8);
	if(around == NULL) {
		// out of space
		return NULL;
	}
	// get the data
	payload_t *data = arr->data;
	// construct the array
	around[0] = (array2d_boundCheck(arr, row-1, col-1)) ? data[(row-1) * arr->numCols + col-1] : NULL;
	around[1] = (array2d_boundCheck(arr, row-1, col)) ? data[(row-1) * arr->numCols + col] : NULL;
	around[2] = (array2d_boundCheck(arr, row-1, col+1)) ? data[(row-1) * arr->numCols + col+1] : NULL;
	around[3] = (array2d_boundCheck(arr, row, col-1)) ? data[(row) * arr->numCols + col-1] : NULL;
	around[4] = (array2d_boundCheck(arr, row, col+1)) ? data[(row) * arr->numCols + col+1] : NULL;
	around[5] = (array2d_boundCheck(arr, row+1, col-1)) ? data[(row+1) * arr->numCols + col-1] : NULL;
	around[6] = (array2d_boundCheck(arr, row+1, col)) ? data[(row+1) * arr->numCols + col] : NULL;
	around[7] = (array2d_boundCheck(arr, row+1, col+1)) ? data[(row+1) * arr->numCols + col+1] : NULL;
	return around;
}


int array2d_print(array2d arr, PayloadPrintFnPtr payload_print_function) {
	if(arr == NULL) {
		return E_NULLPOINTER;	
	}
	int numRows = arr->numRows;
	int numCols = arr->numCols;

	printf("Array is %i x %i\n", numRows, numCols);

	for (int i = 0; i < numRows; i++) {
		printf("[");
	        payload_t payload;
		array2d_get(arr,&payload, i, 0);
		payload_print_function(payload);
	  	for (int j = 1; j < numCols; j++) {
	        	printf(", ");
	   		payload_print_function(arr->data[i * arr->numCols + j]);
		}
	 	printf("]\n");
	}
	return E_SUCCESS;
}
