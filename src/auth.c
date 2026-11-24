#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auth.h"
#include "utils.h"
#include "fileio.h"

#define MIN_PASSWORD_LENGTH 4

static const char *selectRole(void);
static int  hasUsername(const Library *library, const char *username);
static int  readCredential(const char *prompt, char *buffer, int bufferSize, int minimumLength);
static void registerAccount(Library *library);

int authenticate(const Library *library, const char *username, const char *password)
{
    int i;

    for (i = 0; i < library->accountCount; i++) {
        if (strcmp(library->accounts[i].username, username) == 0 &&
            strcmp(library->accounts[i].password, password) == 0) {
            return i;
        }
    }
    return FAILURE;
}

static int hasUsername(const Library *library, const char *username)
{
    int i;

    for (i = 0; i < library->accountCount; i++) {
        if (strcmp(library->accounts[i].username, username) == 0) return 1;
    }
    return 0;
}

static int readCredential(const char *prompt, char *buffer, int bufferSize, int minimumLength)
{
    for (;;) {
        if (readString(prompt, buffer, bufferSize) != SUCCESS) return FAILURE;
        if ((int)strlen(buffer) < minimumLength) {
            printf("Minimal %d karakter.\n", minimumLength);
        } else if (strchr(buffer, ' ') != NULL || strchr(buffer, '\t') != NULL ||
                   strchr(buffer, '"') != NULL) {
            printf("Tidak boleh mengandung spasi atau tanda kutip.\n");
        } else {
            return SUCCESS;
        }
    }
}

static void registerAccount(Library *library)
{
    Account account;
    Account *resizedArray;
    char confirm[MAX_PASSWORD];
    int newCapacity;

    memset(&account, 0, sizeof(account));
    printf("\n=== DAFTAR AKUN BARU ===\n");
    if (readCredential("Username: ", account.username, MAX_USERNAME, 1) != SUCCESS) return;
    if (hasUsername(library, account.username)) {
        printf("Username sudah dipakai.\n");
        return;
    }
    if (readCredential("Password (min 4): ", account.password, MAX_PASSWORD, MIN_PASSWORD_LENGTH) != SUCCESS) return;
    if (readString("Ulangi password: ", confirm, MAX_PASSWORD) != SUCCESS) return;
    if (strcmp(account.password, confirm) != 0) {
        printf("Konfirmasi password tidak sama.\n");
        return;
    }
    strcpy(account.role, "user");

    if (library->accountCount >= library->accountCapacity) {
        newCapacity = (library->accountCapacity == 0) ? 8 : library->accountCapacity * 2;
        resizedArray = (Account *)realloc(library->accounts, (size_t)newCapacity * sizeof(Account));
        if (resizedArray == NULL) {
            printf("Memori tidak cukup.\n");
            return;
        }
        library->accounts = resizedArray;
        library->accountCapacity = newCapacity;
    }
    library->accounts[library->accountCount] = account;
    library->accountCount++;

    if (saveAccounts(library) != SUCCESS) {
        printf("Peringatan: gagal menyimpan akun.\n");
        return;
    }
    printf("Akun '%s' berhasil dibuat. Silakan login sebagai User.\n", account.username);
}

/* Returns "admin", "user", or NULL to quit. Registration loops back to the menu. */
static const char *selectRole(void)
{
    char choice[8];

    for (;;) {
        printf("\n=== MASUK SEBAGAI ===\n");
        printf("1. Admin\n");
        printf("2. User\n");
        printf("3. Daftar akun baru\n");
        printf("4. Keluar\n");
        if (readString("Pilihan: ", choice, (int)sizeof(choice)) != SUCCESS) return NULL;
        if (strcmp(choice, "1") == 0) return "admin";
        if (strcmp(choice, "2") == 0) return "user";
        if (strcmp(choice, "4") == 0) return NULL;
        if (strcmp(choice, "3") == 0) {
            /* handled in login(), which owns the mutable Library */
            return "daftar";
        }
        printf("Pilihan tidak valid.\n");
    }
}

int login(Library *library)
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    const char *role;
    int attempt;
    int accountIndex;

    for (;;) {
        role = selectRole();
        if (role == NULL) return LOGIN_CANCELLED;
        if (strcmp(role, "daftar") == 0) {
            registerAccount(library);
            continue;
        }
        break;
    }

    for (attempt = 1; attempt <= LOGIN_ATTEMPTS; attempt++) {
        printf("\n=== LOGIN %s (percobaan %d/%d) ===\n", role, attempt, LOGIN_ATTEMPTS);
        if (readString("Username: ", username, MAX_USERNAME) != SUCCESS) return LOGIN_CANCELLED;
        if (readString("Password: ", password, MAX_PASSWORD) != SUCCESS) return LOGIN_CANCELLED;
        accountIndex = authenticate(library, username, password);
        if (accountIndex != FAILURE && strcmp(library->accounts[accountIndex].role, role) == 0) {
            printf("Selamat datang, %s (%s).\n",
                   library->accounts[accountIndex].username,
                   library->accounts[accountIndex].role);
            return accountIndex;
        }
        printf("Username atau password salah.\n");
    }
    return FAILURE;
}
