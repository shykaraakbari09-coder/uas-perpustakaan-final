#ifndef LOAN_H
#define LOAN_H

#include "types.h"

int  borrowBook(Library *library, const char *username);
int  returnBook(Library *library, const char *username);
void showMyLoans(const Library *library, const char *username);
void showAllLoans(const Library *library);

#endif
