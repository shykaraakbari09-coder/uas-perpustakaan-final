// ===== HEADERS =====
#ifndef TYPES_H
#define TYPES_H

/* Field sizes from the PDF spec and the Books.txt dataset stats:
   longest title 1121, author 460, line 1298.
   Text fields sized generously to avoid truncation on parse. */
#define MAX_OCAID     64
#define MAX_TITLE     2048
#define MAX_AUTHOR    1024
#define MAX_EDITION   128
#define MAX_WORK      128
#define MAX_URL       512
#define MAX_USERNAME  32
#define MAX_PASSWORD  32
#define MAX_ROLE      12
#define MAX_DATE      20
#define MAX_STATUS    20

#define DEFAULT_STOCK   3
#define PAGE_SIZE       20
#define LOGIN_ATTEMPTS  3

#define BOOKS_MASTER_PATH "data/Books.txt"
#define INVENTORY_PATH    "data/inventory.txt"
#define ACCOUNTS_PATH     "data/user.txt"
#define LOANS_PATH        "data/peminjaman.txt"

#define SUCCESS  0
#define FAILURE  (-1)

typedef struct {
    char ocaid[MAX_OCAID];
    char title[MAX_TITLE];
    char author[MAX_AUTHOR];
    int  year;
    int  pageCount;
    char edition[MAX_EDITION];
    char work[MAX_WORK];
    char url[MAX_URL];
    int  stock;
} Book;

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char role[MAX_ROLE];
} Account;

typedef struct {
    unsigned int loanId;
    char username[MAX_USERNAME];
    char ocaid[MAX_OCAID];
    char loanDate[MAX_DATE];
    char status[MAX_STATUS];
} Loan;

typedef struct {
    Book    *books;
    int      bookCount;
    int      bookCapacity;
    Account *accounts;
    int      accountCount;
    int      accountCapacity;
    Loan    *loans;
    int      loanCount;
    int      loanCapacity;
    unsigned int nextLoanId;
} Library;

#endif
#ifndef UTILS_H
#define UTILS_H


int  readLine(char *buffer, int bufferSize);
int  readInt(const char *prompt, int minimum, int maximum, int *result);
int  readString(const char *prompt, char *buffer, int bufferSize);
int  readOptional(const char *prompt, char *buffer, int bufferSize);
int  compareIgnoreCase(const char *first, const char *second);
int  containsIgnoreCase(const char *text, const char *pattern);
void getCurrentDate(char *destination, int destinationSize);
void truncateForDisplay(const char *source, char *destination, int maxLength);

#endif
#ifndef AUTH_H
#define AUTH_H


int authenticate(const Library *library, const char *username, const char *password);
#define LOGIN_CANCELLED (-2)

int login(Library *library);

#endif
#ifndef BOOK_H
#define BOOK_H


int  findBookById(const Library *library, const char *ocaid);
int  addBook(Library *library);
int  editBook(Library *library);
int  deleteBook(Library *library);
void searchBooks(const Library *library);
void showSortedCatalog(const Library *library, int availableOnly);

#endif
#ifndef SORT_H
#define SORT_H


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
#ifndef FILEIO_H
#define FILEIO_H


void initLibrary(Library *library);
int  loadBooks(Library *library);
int  loadAccounts(Library *library);
int  loadLoans(Library *library);
int  saveBooks(const Library *library);
int  saveAccounts(const Library *library);
int  saveLoans(const Library *library);
void freeLibrary(Library *library);

#endif
#ifndef LOAN_H
#define LOAN_H


int  borrowBook(Library *library, const char *username);
int  returnBook(Library *library, const char *username);
void showMyLoans(const Library *library, const char *username);
void showAllLoans(const Library *library);

#endif

// ===== SOURCES =====

/*
 * readLine: read one line from stdin using fgets, strip the trailing
 * newline, and if the line did not fit in the buffer, flush the rest
 * so the next call starts clean. Returns SUCCESS or FAILURE (EOF/error).
 */
int readLine(char *buffer, int bufferSize)
{
    int length;
    int ch;

    if (buffer == NULL || bufferSize < 2) {
        return FAILURE;
    }
    if (fgets(buffer, bufferSize, stdin) == NULL) {
        return FAILURE;
    }
    length = (int)strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        /* line did not fit: drain the rest */
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* discard */
        }
    }
    return SUCCESS;
}

/*
 * readInt: print the prompt, read a line, parse it as an integer with
 * strtol, and loop until the value lies in [minimum, maximum].
 * No raw scanf is used anywhere. Returns SUCCESS on success.
 */
int readInt(const char *prompt, int minimum, int maximum, int *result)
{
    char buffer[64];
    char *end;
    long value;

    if (result == NULL) {
        return FAILURE;
    }
    for (;;) {
        printf("%s", prompt);
        if (readLine(buffer, (int)sizeof(buffer)) != SUCCESS) {
            return FAILURE;
        }
        value = strtol(buffer, &end, 10);
        if (end == buffer || *end != '\0') {
            printf("Input tidak valid. Masukkan angka.\n");
            continue;
        }
        if (value < (long)minimum || value > (long)maximum) {
            printf("Harus antara %d dan %d.\n", minimum, maximum);
            continue;
        }
        *result = (int)value;
        return SUCCESS;
    }
}

/*
 * readString: print the prompt and read a line, rejecting empty input.
 * Loops until the user types something non-empty. The buffer is capped
 * at bufferSize - 1 characters by readLine itself.
 */
int readString(const char *prompt, char *buffer, int bufferSize)
{
    if (buffer == NULL || bufferSize < 2) {
        return FAILURE;
    }
    for (;;) {
        printf("%s", prompt);
        if (readLine(buffer, bufferSize) != SUCCESS) {
            return FAILURE;
        }
        if (buffer[0] != '\0') {
            return SUCCESS;
        }
        printf("Input tidak boleh kosong.\n");
    }
}

/*
 * compareIgnoreCase: same sign convention as strcmp (<0, 0, >0) but
 * ASCII case-insensitive. strcasecmp is POSIX and banned by -ansi,
 * so we lowercase one character at a time with tolower.
 */
int compareIgnoreCase(const char *first, const char *second)
{
    int left;
    int right;

    if (first == NULL || second == NULL) {
        return 0;
    }
    while (*first != '\0' && *second != '\0') {
        left  = tolower((unsigned char)*first);
        right = tolower((unsigned char)*second);
        if (left != right) {
            return left - right;
        }
        first++;
        second++;
    }
    return tolower((unsigned char)*first) - tolower((unsigned char)*second);
}

/*
 * containsIgnoreCase: returns nonzero if pattern appears anywhere in
 * text (case-insensitive). Returns 0 otherwise. Used by searchBooks.
 */
int containsIgnoreCase(const char *text, const char *pattern)
{
    int textLength;
    int patternLength;
    int i;
    int j;
    int match;

    if (text == NULL || pattern == NULL) {
        return 0;
    }
    textLength    = (int)strlen(text);
    patternLength = (int)strlen(pattern);
    if (patternLength == 0) {
        return 1;
    }
    if (patternLength > textLength) {
        return 0;
    }
    for (i = 0; i <= textLength - patternLength; i++) {
        match = 1;
        for (j = 0; j < patternLength; j++) {
            if (tolower((unsigned char)text[i + j]) !=
                tolower((unsigned char)pattern[j])) {
                match = 0;
                break;
            }
        }
        if (match) {
            return 1;
        }
    }
    return 0;
}

/*
 * getCurrentDate: fill destination with today's date in DD-MM-YYYY.
 * strftime is C89; snprintf is not, so we use strftime directly.
 * destinationSize must be at least MAX_DATE (20) bytes.
 */
void getCurrentDate(char *destination, int destinationSize)
{
    time_t now;
    struct tm *local;
    int written;

    if (destination == NULL || destinationSize < 11) {
        if (destination != NULL && destinationSize > 0) {
            destination[0] = '\0';
        }
        return;
    }
    now = time(NULL);
    if (now == (time_t)-1) {
        destination[0] = '\0';
        return;
    }
    local = localtime(&now);
    if (local == NULL) {
        destination[0] = '\0';
        return;
    }
    written = (int)strftime(destination, (size_t)destinationSize, "%d-%m-%Y", local);
    if (written == 0) {
        destination[0] = '\0';
    }
}

/*
 * truncateForDisplay: copy source into destination but keep at most
 * maxLength characters. If the source is longer, the last three
 * characters of destination become "..." so the user knows it was cut.
 * Display only: the full string stays in the Book struct and on disk.
 */
void truncateForDisplay(const char *source, char *destination, int maxLength)
{
    int length;
    int copy;

    if (destination == NULL || maxLength < 1) {
        return;
    }
    if (source == NULL) {
        destination[0] = '\0';
        return;
    }
    length = (int)strlen(source);
    if (length <= maxLength) {
        strcpy(destination, source);
        return;
    }
    if (maxLength <= 3) {
        copy = maxLength;
        memcpy(destination, source, (size_t)copy);
        destination[copy] = '\0';
        return;
    }
    copy = maxLength - 3;
    memcpy(destination, source, (size_t)copy);
    destination[copy]     = '.';
    destination[copy + 1] = '.';
    destination[copy + 2] = '.';
    destination[copy + 3] = '\0';
}

int readOptional(const char *prompt, char *buffer, int bufferSize)
{
    int length;

    printf("%s", prompt);
    fflush(stdout);
    if (readLine(buffer, bufferSize) != SUCCESS) return FAILURE;
    length = (int)strlen(buffer);
    while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
        buffer[--length] = '\0';
    }
    return SUCCESS;
}

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






int findBookById(const Library *library, const char *ocaid)
{
    int i;

    for (i = 0; i < library->bookCount; i++) {
        if (strcmp(library->books[i].ocaid, ocaid) == 0) {
            return i;
        }
    }
    return FAILURE;
}






void searchBooks(const Library *library)
{
    char keyword[128];
    char shortTitle[41];
    char shortAuthor[26];
    char answer[8];
    int i;
    int found = 0;

    if (readString("Kata kunci (ID/judul/penulis): ", keyword, (int)sizeof(keyword)) != SUCCESS) {
        return;
    }
    if (keyword[0] == '\0') {
        printf("Kata kunci tidak boleh kosong.\n");
        return;
    }

    for (i = 0; i < library->bookCount; i++) {
        if (!containsIgnoreCase(library->books[i].ocaid, keyword) &&
            !containsIgnoreCase(library->books[i].title, keyword) &&
            !containsIgnoreCase(library->books[i].author, keyword)) {
            continue;
        }
        if (found % PAGE_SIZE == 0) {
            printf("\n%-28s %-40s %-25s %-5s %s\n", "OCAID", "JUDUL", "PENULIS", "THN", "STOK");
        }
        truncateForDisplay(library->books[i].title, shortTitle, 40);
        truncateForDisplay(library->books[i].author, shortAuthor, 25);
        printf("%-28.28s %-40s %-25s %-5d %d\n",
               library->books[i].ocaid, shortTitle, shortAuthor,
               library->books[i].year, library->books[i].stock);
        found++;
        if (found % PAGE_SIZE == 0) {
            if (readOptional("Enter lanjut, q berhenti: ", answer, (int)sizeof(answer)) != SUCCESS ||
                answer[0] == 'q' || answer[0] == 'Q') {
                break;
            }
        }
    }
    if (found == 0) {
        printf("Tidak ada buku yang cocok.\n");
    } else {
        printf("\n%d buku ditampilkan.\n", found);
    }
}

/* Reads a non-empty text field. Rejects double quotes always, and
 * whitespace when allowSpaces is 0 (bare tokens in the data files). */
static int readField(const char *prompt, char *buffer, int bufferSize, int allowSpaces)
{
    for (;;) {
        if (readString(prompt, buffer, bufferSize) != SUCCESS) {
            return FAILURE;
        }
        if (buffer[0] == '\0') {
            printf("Tidak boleh kosong.\n");
        } else if (strchr(buffer, '"') != NULL) {
            printf("Tanda kutip (\") tidak boleh dipakai.\n");
        } else if (!allowSpaces && (strchr(buffer, ' ') != NULL || strchr(buffer, '\t') != NULL)) {
            printf("Tidak boleh mengandung spasi.\n");
        } else {
            return SUCCESS;
        }
    }
}

int addBook(Library *library)
{
    Book newBook;
    Book *resizedArray;
    int newCapacity;

    memset(&newBook, 0, sizeof(newBook));

    if (readField("OCAID (tanpa spasi): ", newBook.ocaid, MAX_OCAID, 0) != SUCCESS) return FAILURE;
    if (findBookById(library, newBook.ocaid) != FAILURE) {
        printf("OCAID sudah ada. Gunakan Edit buku.\n");
        return FAILURE;
    }
    if (readField("Judul: ", newBook.title, MAX_TITLE, 1) != SUCCESS) return FAILURE;
    if (readField("Penulis: ", newBook.author, MAX_AUTHOR, 1) != SUCCESS) return FAILURE;
    if (readInt("Tahun (1-2100): ", 1, 2100, &newBook.year) != SUCCESS) return FAILURE;
    if (readInt("Jumlah halaman (1-100000): ", 1, 100000, &newBook.pageCount) != SUCCESS) return FAILURE;
    if (readField("Edisi (tanpa spasi): ", newBook.edition, MAX_EDITION, 0) != SUCCESS) return FAILURE;
    if (readField("Work (tanpa spasi): ", newBook.work, MAX_WORK, 0) != SUCCESS) return FAILURE;
    if (readField("URL (tanpa spasi): ", newBook.url, MAX_URL, 0) != SUCCESS) return FAILURE;
    if (readInt("Stok (0-1000): ", 0, 1000, &newBook.stock) != SUCCESS) return FAILURE;

    if (library->bookCount >= library->bookCapacity) {
        newCapacity = (library->bookCapacity == 0) ? 64 : library->bookCapacity * 2;
        resizedArray = (Book *)realloc(library->books, (size_t)newCapacity * sizeof(Book));
        if (resizedArray == NULL) {
            printf("Memori tidak cukup.\n");
            return FAILURE;
        }
        library->books = resizedArray;
        library->bookCapacity = newCapacity;
    }
    library->books[library->bookCount] = newBook;
    library->bookCount++;

    if (saveBooks(library) != SUCCESS) {
        printf("Peringatan: gagal menyimpan ke %s.\n", INVENTORY_PATH);
        return FAILURE;
    }
    printf("Buku '%s' berhasil ditambahkan.\n", newBook.ocaid);
    return SUCCESS;
}

/* Prompts "label [current]: ". Empty input keeps the current text. */
static int editTextField(const char *label, char *target, int bufferSize, int allowSpaces)
{
    char prompt[160];
    char input[MAX_TITLE];

    for (;;) {
        sprintf(prompt, "%s (Enter = tetap): ", label);
        if (readOptional(prompt, input, bufferSize) != SUCCESS) return FAILURE;
        if (input[0] == '\0') return SUCCESS;
        if (strchr(input, '"') != NULL) {
            printf("Tanda kutip (\") tidak boleh dipakai.\n");
        } else if (!allowSpaces && (strchr(input, ' ') != NULL || strchr(input, '\t') != NULL)) {
            printf("Tidak boleh mengandung spasi.\n");
        } else {
            strcpy(target, input);
            return SUCCESS;
        }
    }
}

/* Prompts for a number; empty input keeps the current value. */
static int editIntField(const char *label, int minimum, int maximum, int *target)
{
    char line[32];
    char prompt[96];
    int value;

    for (;;) {
        sprintf(prompt, "%s [%d] (Enter = tetap): ", label, *target);
        if (readOptional(prompt, line, (int)sizeof(line)) != SUCCESS) return FAILURE;
        if (line[0] == '\0') return SUCCESS;
        value = atoi(line);
        if (value < minimum || value > maximum ||
            (value == 0 && strcmp(line, "0") != 0)) {
            printf("Masukkan angka %d-%d.\n", minimum, maximum);
        } else {
            *target = value;
            return SUCCESS;
        }
    }
}

int editBook(Library *library)
{
    char ocaid[MAX_OCAID];
    Book draft;
    int index;

    if (readString("OCAID buku yang diedit: ", ocaid, MAX_OCAID) != SUCCESS) return FAILURE;
    index = findBookById(library, ocaid);
    if (index == FAILURE) {
        printf("Buku tidak ditemukan.\n");
        return FAILURE;
    }

    draft = library->books[index];
    printf("\nJudul   : %s\nPenulis : %s\n", draft.title, draft.author);
    if (editTextField("Judul", draft.title, MAX_TITLE, 1) != SUCCESS) return FAILURE;
    printf("Penulis : %s\n", draft.author);
    if (editTextField("Penulis", draft.author, MAX_AUTHOR, 1) != SUCCESS) return FAILURE;
    if (editIntField("Tahun", 1, 2100, &draft.year) != SUCCESS) return FAILURE;
    if (editIntField("Halaman", 1, 100000, &draft.pageCount) != SUCCESS) return FAILURE;
    if (editIntField("Stok", 0, 1000, &draft.stock) != SUCCESS) return FAILURE;

    library->books[index] = draft;
    if (saveBooks(library) != SUCCESS) {
        printf("Peringatan: gagal menyimpan ke %s.\n", INVENTORY_PATH);
        return FAILURE;
    }
    printf("Buku '%s' diperbarui.\n", draft.ocaid);
    return SUCCESS;
}

int deleteBook(Library *library)
{
    char ocaid[MAX_OCAID];
    char answer[8];
    int index;
    int i;

    if (readString("OCAID buku yang dihapus: ", ocaid, MAX_OCAID) != SUCCESS) return FAILURE;
    index = findBookById(library, ocaid);
    if (index == FAILURE) {
        printf("Buku tidak ditemukan.\n");
        return FAILURE;
    }

    for (i = 0; i < library->loanCount; i++) {
        if (strcmp(library->loans[i].ocaid, ocaid) == 0 &&
            strcmp(library->loans[i].status, "DIPINJAM") == 0) {
            printf("Ditolak: buku masih dipinjam oleh %s.\n", library->loans[i].username);
            return FAILURE;
        }
    }

    printf("Hapus '%s'? (y/n): ", library->books[index].title);
    if (readLine(answer, (int)sizeof(answer)) != SUCCESS ||
        (answer[0] != 'y' && answer[0] != 'Y')) {
        printf("Dibatalkan.\n");
        return FAILURE;
    }

    memmove(&library->books[index], &library->books[index + 1],
            (size_t)(library->bookCount - index - 1) * sizeof(Book));
    library->bookCount--;

    if (saveBooks(library) != SUCCESS) {
        printf("Peringatan: gagal menyimpan ke %s.\n", INVENTORY_PATH);
        return FAILURE;
    }
    printf("Buku '%s' dihapus.\n", ocaid);
    return SUCCESS;
}

void showSortedCatalog(const Library *library, int availableOnly)
{
    int *order;
    int count = 0;
    int i;
    int algorithm;
    int fieldChoice;
    int direction;
    SortField field;
    clock_t startTime;
    double seconds;
    char shortTitle[41];
    char shortAuthor[26];
    char answer[8];

    if (library->bookCount == 0) {
        printf("Katalog kosong.\n");
        return;
    }

    printf("\n=== URUTKAN KATALOG ===\n");
    printf("Algoritma: 1. Bubble  2. Selection  3. Insertion\n");
    if (readInt("Pilih algoritma (1-3): ", 1, 3, &algorithm) != SUCCESS) return;
    printf("Urut berdasarkan: 1. ID  2. Judul  3. Tahun  4. Halaman  5. Stok\n");
    if (readInt("Pilih field (1-5): ", 1, 5, &fieldChoice) != SUCCESS) return;
    printf("Arah: 1. Naik (A-Z, kecil-besar)  2. Turun\n");
    if (readInt("Pilih arah (1-2): ", 1, 2, &direction) != SUCCESS) return;
    field = (SortField)(fieldChoice - 1);

    order = (int *)malloc((size_t)library->bookCount * sizeof(int));
    if (order == NULL) {
        printf("Memori tidak cukup.\n");
        return;
    }
    for (i = 0; i < library->bookCount; i++) {
        if (!availableOnly || library->books[i].stock > 0) {
            order[count++] = i;
        }
    }
    if (count == 0) {
        printf("Tidak ada buku yang tersedia.\n");
        free(order);
        return;
    }

    startTime = clock();
    if (algorithm == 1) {
        bubbleSort(library, order, count, field, direction == 2);
    } else if (algorithm == 2) {
        selectionSort(library, order, count, field, direction == 2);
    } else {
        insertionSort(library, order, count, field, direction == 2);
    }
    seconds = (double)(clock() - startTime) / (double)CLOCKS_PER_SEC;
    printf("\n%d buku diurutkan dalam %.3f detik.\n", count, seconds);

    for (i = 0; i < count; i++) {
        if (i % PAGE_SIZE == 0) {
            printf("\n%-28s %-40s %-25s %-5s %s\n", "OCAID", "JUDUL", "PENULIS", "THN", "STOK");
        }
        truncateForDisplay(library->books[order[i]].title, shortTitle, 40);
        truncateForDisplay(library->books[order[i]].author, shortAuthor, 25);
        printf("%-28.28s %-40s %-25s %-5d %d\n",
               library->books[order[i]].ocaid, shortTitle, shortAuthor,
               library->books[order[i]].year, library->books[order[i]].stock);
        if ((i + 1) % PAGE_SIZE == 0 && i + 1 < count) {
            printf("-- halaman %d dari %d --\n", (i + 1) / PAGE_SIZE,
                   (count + PAGE_SIZE - 1) / PAGE_SIZE);
            if (readOptional("Enter lanjut, q berhenti: ", answer, (int)sizeof(answer)) != SUCCESS ||
                answer[0] == 'q' || answer[0] == 'Q') {
                break;
            }
        }
    }
    free(order);
}





/* Like readString, but empty input is allowed (used for "Enter = keep"). */

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

#define LINE_BUFFER 4096

static int parseQuotedField(const char **cursor, char *destination, int destinationSize);
static int parseBareField(const char **cursor, char *destination, int destinationSize);
static int parseIntField(const char **cursor, int *destination);
static int parseBookLine(const char *line, Book *book, int hasStock);
static void seedInventory(void);

void initLibrary(Library *library)
{
    library->books = NULL;
    library->bookCount = 0;
    library->bookCapacity = 0;
    library->accounts = NULL;
    library->accountCount = 0;
    library->accountCapacity = 0;
    library->loans = NULL;
    library->loanCount = 0;
    library->loanCapacity = 0;
    library->nextLoanId = 1;
}

static int parseQuotedField(const char **cursor, char *destination, int destinationSize)
{
    const char *p = *cursor;
    int n = 0;

    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return FAILURE;
    p++;
    while (*p != '\0' && *p != '"') {
        if (n < destinationSize - 1) destination[n++] = *p;
        p++;
    }
    if (*p != '"') return FAILURE;
    p++;
    destination[n] = '\0';
    *cursor = p;
    return SUCCESS;
}

static int parseBareField(const char **cursor, char *destination, int destinationSize)
{
    const char *p = *cursor;
    int n = 0;

    while (*p == ' ' || *p == '\t') p++;
    while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
        if (n < destinationSize - 1) destination[n++] = *p;
        p++;
    }
    if (n == 0) return FAILURE;
    destination[n] = '\0';
    *cursor = p;
    return SUCCESS;
}

static int parseIntField(const char **cursor, int *destination)
{
    char temp[32];
    if (parseBareField(cursor, temp, (int)sizeof(temp)) != SUCCESS) return FAILURE;
    *destination = atoi(temp);
    return SUCCESS;
}

static int parseTextField(const char **cursor, char *destination, int destinationSize)
{
    const char *p = *cursor;

    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') return parseQuotedField(cursor, destination, destinationSize);
    return parseBareField(cursor, destination, destinationSize);
}

/*
 * Books.txt format (real dataset, CRLF line endings):
 *   ocaid "title" "author" year page_count edition work url
 * title and author are quoted; all other fields are bare tokens.
 * inventory.txt adds a trailing stock integer.
 */
static int parseBookLine(const char *line, Book *book, int hasStock)
{
    const char *cursor = line;

    if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r') return FAILURE;
    if (strncmp(line, "ocaid", 5) == 0) return FAILURE;

    if (parseBareField(&cursor, book->ocaid, MAX_OCAID) != SUCCESS) return FAILURE;
    if (parseTextField(&cursor, book->title, MAX_TITLE) != SUCCESS) return FAILURE;
    if (parseTextField(&cursor, book->author, MAX_AUTHOR) != SUCCESS) return FAILURE;
    if (parseIntField(&cursor, &book->year) != SUCCESS) return FAILURE;
    if (parseIntField(&cursor, &book->pageCount) != SUCCESS) return FAILURE;
    if (parseBareField(&cursor, book->edition, MAX_EDITION) != SUCCESS) return FAILURE;
    if (parseBareField(&cursor, book->work, MAX_WORK) != SUCCESS) return FAILURE;
    if (parseBareField(&cursor, book->url, MAX_URL) != SUCCESS) return FAILURE;
    if (hasStock) {
        if (parseIntField(&cursor, &book->stock) != SUCCESS) return FAILURE;
    } else {
        book->stock = DEFAULT_STOCK;
    }
    return SUCCESS;
}

static int growBooks(Library *library)
{
    Book *temp;
    int newCap;

    if (library->bookCount < library->bookCapacity) return SUCCESS;
    newCap = (library->bookCapacity == 0) ? 64 : library->bookCapacity * 2;
    temp = (Book *)realloc(library->books, (size_t)newCap * sizeof(Book));
    if (temp == NULL) return FAILURE;
    library->books = temp;
    library->bookCapacity = newCap;
    return SUCCESS;
}

static void stripLineEnd(char *line)
{
    int len = (int)strlen(line);
    if (len > 0 && line[len - 1] == '\n') { line[--len] = '\0'; }
    if (len > 0 && line[len - 1] == '\r') { line[--len] = '\0'; }
}

static void seedInventory(void)
{
    FILE *in;
    FILE *destination;
    char line[LINE_BUFFER];
    Book book;
    int seeded;

    in = fopen(BOOKS_MASTER_PATH, "r");
    if (in == NULL) return;
    destination = fopen(INVENTORY_PATH, "w");
    if (destination == NULL) { fclose(in); return; }
    seeded = 0;
    while (fgets(line, LINE_BUFFER, in) != NULL) {
        stripLineEnd(line);
        if (parseBookLine(line, &book, 0) != SUCCESS) continue;
        if (seeded == 0) {
            fprintf(destination, "ocaid \"title\" \"author\" year page_count edition work url stock\n");
            seeded = 1;
        }
        fprintf(destination, "%s \"%s\" \"%s\" %d %d %s %s %s %d\n",
                book.ocaid, book.title, book.author, book.year, book.pageCount,
                book.edition, book.work, book.url, book.stock);
    }
    fclose(in);
    fclose(destination);
}

static int loadBooksFrom(const char *path, Library *library, int hasStock)
{
    FILE *file;
    char line[LINE_BUFFER];
    Book book;

    file = fopen(path, "r");
    if (file == NULL) return FAILURE;
    while (fgets(line, LINE_BUFFER, file) != NULL) {
        stripLineEnd(line);
        if (parseBookLine(line, &book, hasStock) != SUCCESS) continue;
        if (growBooks(library) != SUCCESS) { fclose(file); return FAILURE; }
        library->books[library->bookCount++] = book;
    }
    fclose(file);
    return SUCCESS;
}

int loadBooks(Library *library)
{
    FILE *check;

    check = fopen(INVENTORY_PATH, "r");
    if (check == NULL) {
        seedInventory();
    } else {
        fclose(check);
    }
    return loadBooksFrom(INVENTORY_PATH, library, 1);
}

int saveBooks(const Library *library)
{
    FILE *file;
    int i;

    file = fopen(INVENTORY_PATH, "w");
    if (file == NULL) return FAILURE;
    fprintf(file, "ocaid \"title\" \"author\" year page_count edition work url stock\n");
    for (i = 0; i < library->bookCount; i++) {
        fprintf(file, "%s \"%s\" \"%s\" %d %d %s %s %s %d\n",
                library->books[i].ocaid,
                library->books[i].title,
                library->books[i].author,
                library->books[i].year,
                library->books[i].pageCount,
                library->books[i].edition,
                library->books[i].work,
                library->books[i].url,
                library->books[i].stock);
    }
    fclose(file);
    return SUCCESS;
}

static int growAccounts(Library *library)
{
    Account *temp;
    int newCap;

    if (library->accountCount < library->accountCapacity) return SUCCESS;
    newCap = (library->accountCapacity == 0) ? 8 : library->accountCapacity * 2;
    temp = (Account *)realloc(library->accounts, (size_t)newCap * sizeof(Account));
    if (temp == NULL) return FAILURE;
    library->accounts = temp;
    library->accountCapacity = newCap;
    return SUCCESS;
}

int loadAccounts(Library *library)
{
    FILE *file;
    char line[LINE_BUFFER];
    Account account;

    file = fopen(ACCOUNTS_PATH, "r");
    if (file == NULL) return FAILURE;
    while (fgets(line, LINE_BUFFER, file) != NULL) {
        stripLineEnd(line);
        if (sscanf(line, "%31s %31s %11s",
                   account.username, account.password, account.role) != 3) continue;
        if (growAccounts(library) != SUCCESS) { fclose(file); return FAILURE; }
        library->accounts[library->accountCount++] = account;
    }
    fclose(file);
    return SUCCESS;
}

int saveAccounts(const Library *library)
{
    FILE *file;
    int i;

    file = fopen(ACCOUNTS_PATH, "w");
    if (file == NULL) return FAILURE;
    for (i = 0; i < library->accountCount; i++) {
        fprintf(file, "%s %s %s\n",
                library->accounts[i].username,
                library->accounts[i].password,
                library->accounts[i].role);
    }
    fclose(file);
    return SUCCESS;
}

static int growLoans(Library *library)
{
    Loan *temp;
    int newCap;

    if (library->loanCount < library->loanCapacity) return SUCCESS;
    newCap = (library->loanCapacity == 0) ? 16 : library->loanCapacity * 2;
    temp = (Loan *)realloc(library->loans, (size_t)newCap * sizeof(Loan));
    if (temp == NULL) return FAILURE;
    library->loans = temp;
    library->loanCapacity = newCap;
    return SUCCESS;
}

int loadLoans(Library *library)
{
    FILE *file;
    char line[LINE_BUFFER];
    Loan loan;

    file = fopen(LOANS_PATH, "r");
    if (file == NULL) return FAILURE;
    while (fgets(line, LINE_BUFFER, file) != NULL) {
        stripLineEnd(line);
        if (sscanf(line, "%u %31s %63s %19s %19s",
                   &loan.loanId, loan.username, loan.ocaid,
                   loan.loanDate, loan.status) != 5) continue;
        if (growLoans(library) != SUCCESS) { fclose(file); return FAILURE; }
        library->loans[library->loanCount++] = loan;
        if (loan.loanId >= library->nextLoanId) library->nextLoanId = loan.loanId + 1;
    }
    fclose(file);
    return SUCCESS;
}

int saveLoans(const Library *library)
{
    FILE *file;
    int i;

    file = fopen(LOANS_PATH, "w");
    if (file == NULL) return FAILURE;
    for (i = 0; i < library->loanCount; i++) {
        fprintf(file, "%u %s %s %s %s\n",
                library->loans[i].loanId,
                library->loans[i].username,
                library->loans[i].ocaid,
                library->loans[i].loanDate,
                library->loans[i].status);
    }
    fclose(file);
    return SUCCESS;
}

void freeLibrary(Library *library)
{
    free(library->books);
    free(library->accounts);
    free(library->loans);
    library->books = NULL;
    library->accounts = NULL;
    library->loans = NULL;
    library->bookCount = 0;
    library->accountCount = 0;
    library->loanCount = 0;
    library->bookCapacity = 0;
    library->accountCapacity = 0;
    library->loanCapacity = 0;
}

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
