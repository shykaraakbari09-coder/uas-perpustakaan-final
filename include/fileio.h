#ifndef FILEIO_H
#define FILEIO_H

#include "types.h"

void initLibrary(Library *library);
int  loadBooks(Library *library);
int  loadAccounts(Library *library);
int  loadLoans(Library *library);
int  saveBooks(const Library *library);
int  saveAccounts(const Library *library);
int  saveLoans(const Library *library);
void freeLibrary(Library *library);

#endif
