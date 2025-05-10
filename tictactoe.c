#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BROKER_IP "34.53.29.173"  // change here

char board[9] = {'1','2','3','4','5','6','7','8','9'};

void drawBoard() {
    system("cls || clear");
    printf("ESP32 Tic-Tac-Toe\n");
    printf("------------------------\n\n");
    printf("Your Symbol: X   Opponent: O\n\n");

    for (int i = 0; i < 9; i++) {
        printf(" %c ", board[i]);
        if (i % 3 != 2) printf("|");
        if (i % 3 == 2 && i != 8) printf("\n---+---+---\n");
    }
    printf("\n\n");
}

int isMoveValid(int move) {
    return board[move] != 'X' && board[move] != 'O';
}

void makeMove(int move, char symbol) {
    board[move] = symbol;
}

void resetBoard() {
    for (int i = 0; i < 9; i++) board[i] = '1' + i;
}
char checkWin() {
    int wins[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };

    for (int i = 0; i < 8; i++) {
        int a = wins[i][0], b = wins[i][1], c = wins[i][2];
        if (board[a] == board[b] && board[b] == board[c])
            return board[a];
    }

    int filled = 1;
    for (int i = 0; i < 9; i++) {
        if (board[i] != 'X' && board[i] != 'O') {
            filled = 0;
            break;
        }
    }

    if (filled) return 'T';

    return ' ';
}

void runMode1() {
    int userMove, pos;
    char symbol, result;

    resetBoard();

    while (1) {
        drawBoard();

        printf("Your Move (1-9): ");
        scanf("%d", &userMove);

        if (userMove < 1 || userMove > 9 || !isMoveValid(userMove - 1)) {
            printf("Invalid move. Try again.\n");
            getchar();  // clear leftover newline
            continue;
        }

        makeMove(userMove - 1, 'X');
        result = checkWin();
        if (result == 'X') {
            drawBoard();
            printf("You win!\n");
            break;
        } else if (result == 'T') {
            drawBoard();
            printf("It's a draw!\n");
            break;
        }

        // 🔁 Step 1: Send local board to GCP
        char boardString[10];
        for (int i = 0; i < 9; i++) boardString[i] = board[i];
        boardString[9] = '\0';

        char updateCmd[512];
        sprintf(updateCmd,
            "ssh -i C:/Users/Andre/.ssh/id_ed25519 andrewpalacios2004@%s \"echo '%s' > /home/andrewpalacios2004/Project3/board.txt\"",
            BROKER_IP, boardString);
        system(updateCmd);

        // 🔁 Step 2: Run playervsbash.sh remotely
        char sshCmd[512];
        sprintf(sshCmd,
            "ssh -i C:/Users/Andre/.ssh/id_ed25519 andrewpalacios2004@%s \"cd /home/andrewpalacios2004/Project3 && ./playervsbash.sh\"",
            BROKER_IP);

        FILE *fp = popen(sshCmd, "r");
        if (!fp) {
            perror("Failed to SSH and run playervsbash.sh");
            return;
        }

        char line[32];
        if (fgets(line, sizeof(line), fp) == NULL) {
            perror("No move received from GCP");
            pclose(fp);
            return;
        }
        pclose(fp);

        // Expecting format: "O 5"
        if (sscanf(line, "%c %d", &symbol, &pos) != 2 || pos < 1 || pos > 9) {
            printf("Failed to parse opponent move: %s\n", line);
            break;
        }

        if (!isMoveValid(pos - 1)) {
            printf("Opponent made invalid move: %d\n", pos);
            break;
        }

        makeMove(pos - 1, symbol);
        drawBoard();

        result = checkWin();
        if (result == 'O') {
            printf("You lost.\n");
            break;
        } else if (result == 'T') {
            printf("It's a draw.\n");
            break;
        }
    }

    printf("Game over. Press Enter to return to menu...");
    getchar(); getchar();
}


void runMode2() {
    int currentPlayer = 0;
    int move;
    char symbols[2] = {'X', 'O'};

    resetBoard();

    while (1) {
        drawBoard();
        printf("Player %c, enter your move (1-9): ", symbols[currentPlayer]);
        scanf("%d", &move);

        if (move < 1 || move > 9 || !isMoveValid(move - 1)) {
            printf("Invalid move. Try again.\n");
            getchar(); getchar();
            continue;
        }

        makeMove(move - 1, symbols[currentPlayer]);

        char result = checkWin();
        if (result == 'X' || result == 'O') {
            drawBoard();
            printf("Player %c wins!\n", result);
            break;
        } else if (result == 'T') {
            drawBoard();
            printf("It's a draw!\n");
            break;
        }

        currentPlayer = 1 - currentPlayer;
    }

    printf("Press Enter to return to menu...");
    getchar();
}
void runMode3() {
    resetBoard();
    drawBoard();

    char sshCmd[256];
    sprintf(sshCmd,
        "ssh -i C:/Users/Andre/.ssh/id_ed25519 andrewpalacios2004@%s \"cd /home/andrewpalacios2004/Project3 && ./game.sh\"",
        BROKER_IP);

    FILE *fp = popen(sshCmd, "r");
    if (!fp) {
        perror("SSH failed");
        return;
    }

    char line[128];
    char symbol;
    int pos;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%c %d", &symbol, &pos) == 2) {
            if (pos >= 1 && pos <= 9) {
                makeMove(pos - 1, symbol);
                drawBoard();
                Sleep(1000);
            }
        } else {
            printf("%s", line);
        }
    }

    pclose(fp);
    printf("Game complete. Press Enter to return to menu...");
    getchar(); getchar();
}


void showMenu() {
    int choice;
    while (1) {
        system("clear");
        printf("Welcome to ESP32 Tic-Tac-Toe\n");
        printf("-----------------------------\n");
        printf("1. Laptop vs Bash\n");
        printf("2. Laptop vs Laptop\n");
        printf("3. Auto Play\n");
        printf("4. Exit\n");
        printf("\nSelect a mode: ");
        scanf("%d", &choice);

        if (choice == 1) {
            runMode1();
        } else if (choice == 2) {
            runMode2();
            getchar();
        } else if (choice == 3) {
            runMode3();
        } else if (choice == 4) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice.\n");
            getchar(); getchar();
        }
    }
}

int main() {
    showMenu();
    return 0;
}
