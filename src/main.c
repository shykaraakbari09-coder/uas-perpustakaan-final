#include <stdio.h>
#include <string.h>
#include "types.h"
#include "utils.h"
#include "fileio.h"
#include "auth.h"
#include "book.h"
#include "loan.h"

static void adminMenu(Library *library);
static void userMenu(Library *library, const char *username);
static void pauseScreen(void);

int main(void)
{
    Library library;
    int accountIndex;

    initLibrary(&library);
    if (loadBooks(&library) != SUCCESS) {
        printf("Gagal memuat data buku.\n");
        freeLibrary(&library);
        return 1;
    }
    if (loadAccounts(&library) != SUCCESS) {
        printf("Gagal memuat data akun.\n");
        freeLibrary(&library);
        return 1;
    }
    if (loadLoans(&library) != SUCCESS) {
        printf("Gagal memuat data peminjaman.\n");
        freeLibrary(&library);
        return 1;
    }

    accountIndex = login(&library);
    if (accountIndex == LOGIN_CANCELLED) {
        printf("Keluar.\n");
        freeLibrary(&library);
        return 0;
    }
    if (accountIndex < 0) {
        printf("Login gagal. Keluar.\n");
        freeLibrary(&library);
        return 1;
    }

    if (strcmp(library.accounts[accountIndex].role, "admin") == 0) {
        adminMenu(&library);
    } else {
        userMenu(&library, library.accounts[accountIndex].username);
    }

    saveBooks(&library);
    saveLoans(&library);
    freeLibrary(&library);

    printf("Terima kasih. Sampai jumpa.\n");
    return 0;
}

static void pauseScreen(void)
{
    char buffer[8];
    printf("\nTekan Enter untuk melanjutkan...");
    readLine(buffer, (int)sizeof(buffer));
}

static void adminMenu(Library *library)
{
    int choice;

    for (;;) {
        printf("\n===== MENU ADMIN =====\n");
        printf("1. Tambah buku\n");
        printf("2. Edit buku\n");
        printf("3. Hapus buku\n");
        printf("4. Cari buku\n");
        printf("5. Lihat katalog\n");
        printf("6. Laporan peminjaman\n");
        printf("7. Logout\n");

        if (readInt("Pilihan: ", 1, 7, &choice) != SUCCESS) {
            return;
        }

        switch (choice) {
        case 1: addBook(library); break;
        case 2: editBook(library); break;
        case 3: deleteBook(library); break;
        case 4: searchBooks(library); break;
        case 5: showSortedCatalog(library, 0); break;
        case 6: showAllLoans(library); break;
        case 7: return;
        default: break;
        }
        pauseScreen();
    }
}

static void userMenu(Library *library, const char *username)
{
    int choice;

    for (;;) {
        printf("\n===== MENU USER (%s) =====\n", username);
        printf("1. Lihat katalog\n");
        printf("2. Cari buku\n");
        printf("3. Pinjam buku\n");
        printf("4. Kembalikan buku\n");
        printf("5. Peminjaman saya\n");
        printf("6. Logout\n");

        if (readInt("Pilihan: ", 1, 6, &choice) != SUCCESS) {
            return;
        }

        switch (choice) {
        case 1: showSortedCatalog(library, 1); break;
        case 2: searchBooks(library); break;
        case 3: borrowBook(library, username); break;
        case 4: returnBook(library, username); break;
        case 5: showMyLoans(library, username); break;
        case 6: return;
        default: break;
        }
        pauseScreen();
    }
}
