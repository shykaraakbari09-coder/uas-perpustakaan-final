#ifndef AUTH_H
#define AUTH_H

#include "types.h"

int authenticate(const Library *library, const char *username, const char *password);
#define LOGIN_CANCELLED (-2)

int login(Library *library);

#endif
