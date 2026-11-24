#include "sort.h"
#include "utils.h"

static int compareInts(int first, int second)
{
    if (first < second) return -1;
    if (first > second) return 1;
    return 0;
}

int compareBooks(const Library *library, int indexA, int indexB, SortField field)
{
    const Book *bookA = &library->books[indexA];
    const Book *bookB = &library->books[indexB];

    switch (field) {
    case SORT_BY_ID:    return compareIgnoreCase(bookA->ocaid, bookB->ocaid);
    case SORT_BY_TITLE: return compareIgnoreCase(bookA->title, bookB->title);
    case SORT_BY_YEAR:  return compareInts(bookA->year, bookB->year);
    case SORT_BY_PAGES: return compareInts(bookA->pageCount, bookB->pageCount);
    case SORT_BY_STOCK: return compareInts(bookA->stock, bookB->stock);
    default:            return 0;
    }
}

/* Direction applied first, then index as tie-break so output is deterministic. */
static int compareOrdered(const Library *library, int indexA, int indexB,
                          SortField field, int descending)
{
    int result = compareBooks(library, indexA, indexB, field);

    if (descending) result = -result;
    if (result == 0) result = compareInts(indexA, indexB);
    return result;
}

void bubbleSort(const Library *library, int *order, int count, SortField field, int descending)
{
    int pass;
    int i;
    int swapped;
    int temp;

    for (pass = 0; pass < count - 1; pass++) {
        swapped = 0;
        for (i = 0; i < count - pass - 1; i++) {
            if (compareOrdered(library, order[i], order[i + 1], field, descending) > 0) {
                temp = order[i];
                order[i] = order[i + 1];
                order[i + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void selectionSort(const Library *library, int *order, int count, SortField field, int descending)
{
    int i;
    int j;
    int smallest;
    int temp;

    for (i = 0; i < count - 1; i++) {
        smallest = i;
        for (j = i + 1; j < count; j++) {
            if (compareOrdered(library, order[j], order[smallest], field, descending) < 0) {
                smallest = j;
            }
        }
        if (smallest != i) {
            temp = order[i];
            order[i] = order[smallest];
            order[smallest] = temp;
        }
    }
}

void insertionSort(const Library *library, int *order, int count, SortField field, int descending)
{
    int i;
    int j;
    int current;

    for (i = 1; i < count; i++) {
        current = order[i];
        j = i - 1;
        while (j >= 0 &&
               compareOrdered(library, order[j], current, field, descending) > 0) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = current;
    }
}
