#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loan.h"
#include "book.h"
#include "utils.h"
#include "fileio.h"

#define STATUS_BORROWED "DIPINJAM"
#define STATUS_RETURNED "KEMBALI"

static const char *titleOf(const Library *library, const char *ocaid)
{
    int index = findBookById(library, ocaid);

    if (index == FAILURE) return "(buku sudah dihapus)";
    return library->books[index].title;
}

/* Matches OCAID or exact title, ignoring case. */
#define KEY_AMBIGUOUS  (-2)
#define MAX_CANDIDATES 10

static int findBookByKey(const Library *library, const char *key)
{
    char shortTitle[41];
    int matchCount = 0;
    int firstMatch = FAILURE;
    int shown = 0;
    int i;

    for (i = 0; i < library->bookCount; i++) {
        if (compareIgnoreCase(library->books[i].ocaid, key) == 0 ||
            compareIgnoreCase(library->books[i].title, key) == 0) {
            return i;
        }
    }
    for (i = 0; i < library->bookCount; i++) {
        if (containsIgnoreCase(library->books[i].title, key) ||
            containsIgnoreCase(library->books[i].ocaid, key)) {
            if (matchCount == 0) firstMatch = i;
            matchCount++;
        }
    }
    if (matchCount == 0) return FAILURE;
    if (matchCount == 1) return firstMatch;

    printf("Ada %d buku cocok. Pilih Pinjam lagi, lalu ketik OCAID lengkapnya:\n", matchCount);
    for (i = 0; i < library->bookCount && shown < MAX_CANDIDATES; i++) {
        if (containsIgnoreCase(library->books[i].title, key) ||
            containsIgnoreCase(library->books[i].ocaid, key)) {
            truncateForDisplay(library->books[i].title, shortTitle, 40);
            printf("  %-28.28s %s\n", library->books[i].ocaid, shortTitle);
            shown++;
        }
    }
    if (matchCount > MAX_CANDIDATES) {
        printf("  ... dan %d lainnya. Persempit kata kuncinya.\n", matchCount - MAX_CANDIDATES);
    }
    return KEY_AMBIGUOUS;
}

static unsigned int nextLoanId(const Library *library)
{
    unsigned int highest = 0;
    int i;

    for (i = 0; i < library->loanCount; i++) {
        if (library->loans[i].loanId > highest) highest = library->loans[i].loanId;
    }
    return highest + 1;
}

static int isActiveLoanOf(const Loan *loan, const char *username)
{
    return strcmp(loan->username, username) == 0 &&
           strcmp(loan->status, STATUS_BORROWED) == 0;
}

static int countActiveLoans(const Library *library, const char *username)
{
    int i;
    int total = 0;

    for (i = 0; i < library->loanCount; i++) {
        if (isActiveLoanOf(&library->loans[i], username)) total++;
    }
    return total;
}

void showMyLoans(const Library *library, const char *username)
{
    char shortTitle[41];
    int i;
    int found = 0;

    printf("\n=== PEMINJAMAN AKTIF: %s ===\n", username);
    for (i = 0; i < library->loanCount; i++) {
        if (!isActiveLoanOf(&library->loans[i], username)) continue;
        if (found == 0) {
            printf("%-6s %-12s %-28s %s\n", "ID", "TANGGAL", "OCAID", "JUDUL");
        }
        truncateForDisplay(titleOf(library, library->loans[i].ocaid), shortTitle, 40);
        printf("%-6u %-12s %-28.28s %s\n", library->loans[i].loanId,
               library->loans[i].loanDate, library->loans[i].ocaid, shortTitle);
        found++;
    }
    if (found == 0) printf("(tidak ada peminjaman aktif)\n");
}

void showAllLoans(const Library *library)
{
    char shortTitle[41];
    char answer[8];
    int i;

    if (library->loanCount == 0) {
        printf("Belum ada riwayat peminjaman.\n");
        return;
    }
    for (i = 0; i < library->loanCount; i++) {
        if (i % PAGE_SIZE == 0) {
            printf("\n%-6s %-14s %-12s %-10s %-24s %s\n",
                   "ID", "PEMINJAM", "TANGGAL", "STATUS", "OCAID", "JUDUL");
        }
        truncateForDisplay(titleOf(library, library->loans[i].ocaid), shortTitle, 40);
        printf("%-6u %-14.14s %-12s %-10s %-24.24s %s\n",
               library->loans[i].loanId, library->loans[i].username,
               library->loans[i].loanDate, library->loans[i].status,
               library->loans[i].ocaid, shortTitle);
        if ((i + 1) % PAGE_SIZE == 0 && i + 1 < library->loanCount) {
            if (readOptional("Enter lanjut, q berhenti: ", answer, (int)sizeof(answer)) != SUCCESS ||
                answer[0] == 'q' || answer[0] == 'Q') {
                return;
            }
        }
    }
    printf("\n%d transaksi.\n", library->loanCount);
}

int borrowBook(Library *library, const char *username)
{
    char key[MAX_TITLE];
    Loan *resizedArray;
    Loan *loan;
    int bookIndex;
    int newCapacity;
    int i;

    if (readString("Ketik OCAID lengkap atau judul lengkap buku: ", key, (int)sizeof(key)) != SUCCESS) return FAILURE;
    if (key[0] == '\0') {
        printf("Tidak boleh kosong.\n");
        return FAILURE;
    }
    bookIndex = findBookByKey(library, key);
    if (bookIndex == KEY_AMBIGUOUS) return FAILURE;
    if (bookIndex == FAILURE) {
        printf("Buku tidak ditemukan.\n");
        return FAILURE;
    }
    if (library->books[bookIndex].stock <= 0) {
        printf("Stok '%s' habis.\n", library->books[bookIndex].title);
        return FAILURE;
    }
    for (i = 0; i < library->loanCount; i++) {
        if (isActiveLoanOf(&library->loans[i], username) &&
            strcmp(library->loans[i].ocaid, library->books[bookIndex].ocaid) == 0) {
            printf("Anda masih meminjam buku ini.\n");
            return FAILURE;
        }
    }

    if (library->loanCount >= library->loanCapacity) {
        newCapacity = (library->loanCapacity == 0) ? 16 : library->loanCapacity * 2;
        resizedArray = (Loan *)realloc(library->loans, (size_t)newCapacity * sizeof(Loan));
        if (resizedArray == NULL) {
            printf("Memori tidak cukup.\n");
            return FAILURE;
        }
        library->loans = resizedArray;
        library->loanCapacity = newCapacity;
    }

    loan = &library->loans[library->loanCount];
    memset(loan, 0, sizeof(Loan));
    loan->loanId = nextLoanId(library);
    strcpy(loan->username, username);
    strcpy(loan->ocaid, library->books[bookIndex].ocaid);
    getCurrentDate(loan->loanDate, MAX_DATE);
    strcpy(loan->status, STATUS_BORROWED);
    library->loanCount++;
    library->books[bookIndex].stock--;
    library->nextLoanId = loan->loanId + 1;

    if (saveBooks(library) != SUCCESS || saveLoans(library) != SUCCESS) {
        library->loanCount--;
        library->books[bookIndex].stock++;
        saveBooks(library);
        printf("Gagal menyimpan transaksi. Dibatalkan.\n");
        return FAILURE;
    }
    printf("Berhasil meminjam '%s' (ID peminjaman %u, tanggal %s). Sisa stok: %d.\n",
           library->books[bookIndex].title, loan->loanId, loan->loanDate,
           library->books[bookIndex].stock);
    return SUCCESS;
}

int returnBook(Library *library, const char *username)
{
    int loanId;
    int target = -1;
    int bookIndex;
    int i;

    if (countActiveLoans(library, username) == 0) {
        printf("Anda tidak punya peminjaman aktif.\n");
        return FAILURE;
    }
    showMyLoans(library, username);
    if (readInt("ID peminjaman yang dikembalikan: ", 1, 1000000000, &loanId) != SUCCESS) {
        return FAILURE;
    }

    for (i = 0; i < library->loanCount; i++) {
        if (library->loans[i].loanId == (unsigned int)loanId) {
            target = i;
            break;
        }
    }
    if (target < 0) {
        printf("ID peminjaman %d tidak ditemukan.\n", loanId);
        return FAILURE;
    }
    if (strcmp(library->loans[target].username, username) != 0) {
        printf("Peminjaman itu bukan milik Anda.\n");
        return FAILURE;
    }
    if (strcmp(library->loans[target].status, STATUS_BORROWED) != 0) {
        printf("Peminjaman itu sudah dikembalikan.\n");
        return FAILURE;
    }

    bookIndex = findBookById(library, library->loans[target].ocaid);
    strcpy(library->loans[target].status, STATUS_RETURNED);
    if (bookIndex != FAILURE) library->books[bookIndex].stock++;

    if (saveLoans(library) != SUCCESS || saveBooks(library) != SUCCESS) {
        strcpy(library->loans[target].status, STATUS_BORROWED);
        if (bookIndex != FAILURE) library->books[bookIndex].stock--;
        saveLoans(library);
        printf("Gagal menyimpan transaksi. Dibatalkan.\n");
        return FAILURE;
    }
    printf("Peminjaman ID %d dikembalikan.\n", loanId);
    return SUCCESS;
}
