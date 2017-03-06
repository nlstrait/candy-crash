#ifndef __ARRAY2D_H
#define __ARRAY2D_H

#include <stdbool.h>  // for bool type (true, false)
#include <jansson.h>  // for serialization and deserialization

// definition:
// _error_code is an enum to help the client debug. It contains integer from
// 0 to 3 representing different possible erors. The library functions will 
// return the correspoding integer when they execute successfully or meet an 
// error
enum _error_code {
	E_SUCCESS = 0,
	E_NULLPOINTER = 1,
	E_OUTOFBOUND = 2,
	E_USEBEFORESET = 3
};


// definition:
// A payload_t is a pointer provided by the client that points to
// the space allocated for one payload. The client is reponsbile for
// allocating the space
typedef void* payload_t;


// definition:
// array2D represents all the information of a two-dimensional array,
// including the number of rows, the number of columns, and a pointer
// to the start (the first) payload pointer
typedef struct array2D {
	payload_t *data;
	bool *visited;
	int numRows;
	int numCols;
} * array2d;


// definition:
// The client code is responsible for providing a free function to free
// the payload of the array
typedef void(*PayloadFreeFnPtr)(payload_t ptr);


// definition:
// The client code is responsible for providing a print function to print
// the playload of the array
typedef void(*PayloadPrintFnPtr)(payload_t ptr);


// Given an error code, return the corresponding message
//
// Arguments:
// 
// -error: the error code
// 
// Return:
// 
// Error message
char* array2d_errorMessage(int error);


// Check if the row number specified is within the bound of the array
//
// Arguments:
// 
// -arr: the bound of the array to check
// -row: the index of row
// 
// Return:
// 
// true if the index is within the bound of the array; false otherwise
bool array2d_rowBoundCheck(const array2d arr, int row);


// Check if the column number specified is within the bound of the array
//
// Arguments:
// 
// -arr: the bound of the array to check
// -col: the index of column
// 
// Return:
// 
// true if the index is within the bound of the array; false otherwise
bool array2d_columnBoundCheck(const array2d arr, int col);


// Check if the indices specified are within the bound of the array
//
// Arguments:
// 
// -arr: the bound of the array to check
// -row: the index of row
// -col: the index of col
// 
// Return:
// 
// true if the index [row, col] is within the bound of the array; false
// otherwise
bool array2d_boundCheck(const array2d arr, int row, int col);


// Allocate memeory space for a row * col two-dimensional array and return
// a pointer to the struct array2D
//
// Arguments:
// 
// -row: the number of rows in the 2-d array
// -col: the number of columns in the 2-d array
// 
// Return:
// 
// a pointer to the struct array2D
array2d array2d_new(int row, int col);


// Get the value at the specified index by storing it into the payload 
// pointer
//
// Arguments:
// 
// -arr: the 2-d array to get the value from
// -payload_ptr: a "return parameter" through which the payload is returned.
// -row: the index of row
// -col: the index of column
// 
// Returns:
// 
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_get(array2d arr, payload_t* payload_ptr, int row, int col);


// Update the value at the specified indices
//
// Arguments:
//
// -arr: the 2-d array to update the value from
// -value: the value to update
// -row: the index of row
// -col: the index of column
// 
// Returns:
// 
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_update(array2d arr, payload_t value, int row, int col);


// Swap the two values given two indices
//
// Arguments:
// 
// -arr: the 2-d array to swap the values from
// -row1: the index of row of the first value
// -col1: the index of column the first value
// -row2: the index of row of the second value
// -col2: the index of column the second value
// 
// Returns:
// 
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_swap(array2d arr, int row1, int col1, int row2, int col2);


// Destroy the two-dimensional array by freeing memeory
//
// Arguments:
// 
// -arr: the 2-d array to be destroyed
// -payload_free_function: a function provided by the client to free the
// space that stores the value
// 
// Returns:
// 
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_destroy(array2d arr, PayloadFreeFnPtr payload_free_function);


// Deserialize the given json file
//
// Arguments:
// 
// -testArray: a pointer to an array2d object, the space will be allocated
// 			   in this method
// -json: a json_t* object, used to "return" the value read from
// 		  file
// -arrayValue: a pointer to json_t* object representing an json_array. Need
//			    this for the client to deserialize individual elements
// 
// Returns:
//
// 0 when exit successfully. Corresponding error code otherwise.	
int array2d_deserialize(array2d *testArray, json_t *json, json_t **arrayValue);


// Serialize the two-dimensional array into json_t* object
//
// Arguments:
// 
// -testArray: the 2-d array to be serialized
// -array: Client is responsible to provide this array which *HAS BEEN LOADED*
// 		   with value
//
// Returns:
//
// a json_t* object with all information serialized
// 
// NOTE:
// 
// If the client uses this method, *make sure to call json_decref() to free space!*
json_t* array2d_serialize(array2d testArray, json_t *array);


// Serialize the two-dimensional array into json_t* object, and output a JSON
// file.
//
// Arguments:
// 
// -testArray: the 2-d array to be serialized
// -filepath: Path to output the JSON file
// -array: Client is responsible to provide this array which *HAS BEEN LOADED*
// 		   with value
//
// 
// NOTE:
// 
// If the client uses this method, NO NEED TO CALL json_decref() again! (has been freed)
void array2d_serializeAndDump(array2d testArray, const char* filepath, json_t *array);


// Return the size of row of the given array
//
// Arguments:
// 
// -arr: the given array
//
// Returns:
//
// the number of rows in the array; -1 if arr pointer is NULL
int array2d_getRowSize(array2d arr);


// Return the size of column of the given array
//
// Arguments:
// 
// -arr: the given array
//
// Returns:
//
// the number of columns in the array; -1 if arr pointer is NULL
int array2d_getColumnSize(array2d arr);


// Return the total size of the given array
//
// Arguments:
// 
// -arr: the given array
//
// Returns:
//
// the total size of the array; -1 if arr pointer is NULL
int array2d_getTotalSize(array2d arr);


// Return the specific row of the given array
//
// Arguments:
// 
// -arr: the given array
// -row: the specific row number
// -rtnArray: the "rtn" pointer to store the row
//
// Returns:
//
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_getRow(array2d arr, int row, payload_t* rtnArray);


// Return the specific column of the given array
//
// Arguments:
// 
// -arr: the given array
// -column: the specific column number
// -rtnArray: the "rtn" pointer to store the column
//
// Returns:
//
// 0 when exit successfully. Corresponding error code otherwise.
int array2d_getColumn(array2d arr, int col, payload_t* rtnArray);

// Return an array of the 8 payload pointers at the indices "around"
// the given index
//
// Arguments:
//
// -arr: the given array
// -row: the given row number
// -col: the given column number
//
// Returns:
//
// an array of length 8 that contains the 8 payload pointers "around"
// the given index. If the index is out of bound, put NULL
//
// NOTE:
//
// The client needs to free the space pointed by the return pointer
// after using this function
payload_t* getAround(array2d arr, int row, int col);

// Prints the dimesnsions and contents of an array2d
//
// Arguments:
//
// -arr: the array2d to be printed
// -payload_print_function: pointer to function for printing client's data
//
// Returns:
//
// 0 if executed successfully, corresponding error code otherwise
int array2d_print(array2d arr, PayloadPrintFnPtr payload_print_function);

#endif  // __ARRAY2D_H
