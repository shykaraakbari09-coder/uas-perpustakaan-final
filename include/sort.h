#ifndef SORT_H
#define SORT_H

#include "types.h"

typedef enum {
    SORT_BY_ID,
    SORT_BY_TITLE,
    SORT_BY_YEAR,
    SORT_BY_PAGES,
    SORT_BY_STOCK
} SortField;

/* Compares two books (by index into library->books). Returns <0, 0, >0. */
int  compareBooks(const Library *library, int indexA, int indexB, SortField field);

/* Each sort rearranges the int index array "order"; the Book structs never move.
 * descending: 0 = ascending, nonzero = descending. Ties keep index order. */
void bubbleSort(const Library *library, int *order, int count, SortField field, int descending);
void selectionSort(const Library *library, int *order, int count, SortField field, int descending);
void insertionSort(const Library *library, int *order, int count, SortField field, int descending);

#endif
