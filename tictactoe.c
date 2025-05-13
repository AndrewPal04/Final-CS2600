#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define BROKER_IP "34.53.123.240"  // change here

char board[10];

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
    int userMove;
    char result = ' ';
    char boardResponse[64];
    char boardCopy[10];

    const char* broker_ip = "34.53.123.240";
    FILE *fp;
    {
        char subInit[512];
        sprintf(subInit, "mosquitto_sub -h %s -t game/board -C 1", broker_ip);
        fp = popen(subInit, "r");
        if (fp && fgets(boardResponse, sizeof(boardResponse), fp)) {
            boardResponse[strcspn(boardResponse, "\r\n")] = 0;
            // parse
            char* t = strtok(boardResponse, ",");
            if (t) {
                strncpy(boardCopy, t, 9);
                boardCopy[9] = '\0';
                t = strtok(NULL, ",");
                result = (t && strlen(t) > 0) ? toupper(t[0]) : ' ';
            }
            memcpy(board, boardCopy, 9);
        }
        if (fp) pclose(fp);
    }

    drawBoard();

    while (1) {
        if (result == 'X') { printf("You win!\n"); break; }
        if (result == 'O') { printf("You lost.\n"); break; }
        if (result == 'T') { printf("It's a draw!\n"); break; }

        printf("Your Move (1-9): ");
        scanf("%d", &userMove);
        getchar();

        {
            char pubCmd[256];
            sprintf(pubCmd, "mqtt_send.bat %d", userMove);
            system(pubCmd);
        }

        {
            char sub1[512];
            sprintf(sub1, "mosquitto_sub -h %s -t game/board -C 1", broker_ip);
            fp = popen(sub1, "r");
        }
        if (!fp || fgets(boardResponse, sizeof(boardResponse), fp) == NULL) {
            perror("read failed");
            if (fp) pclose(fp);
            return;
        }
        pclose(fp);

        boardResponse[strcspn(boardResponse, "\r\n")] = 0;

        {
            char* t = strtok(boardResponse, ",");
            if (t) {
                strncpy(boardCopy, t, 9);
                boardCopy[9] = '\0';
                t = strtok(NULL, ",");
                result = (t && strlen(t) > 0) ? toupper(t[0]) : ' ';
            }
        }
        memcpy(board, boardCopy, 9);
        drawBoard();

        if (result == 'X') { printf("You win!\n"); break; }
        if (result == 'O') { printf("You lost.\n"); break; }
        if (result == 'T') { printf("It's a draw!\n"); break; }

        printf("Opponent's turn...\n");
        {
            char repub[512];
            sprintf(repub, "mosquitto_pub -h %s -t game/board -m \"%.9s,%c\"", broker_ip, board, result);
            system(repub);
            char sshCmd[512];
            sprintf(sshCmd,
                "ssh -i C:/Users/Andre/.ssh/id_ed25519 andrewpalacios2004@%s '/home/andrewpalacios2004/Project3/player2.sh'",
                broker_ip);
            system(sshCmd);
        }

        {
            char sub2[512];
            sprintf(sub2, "mosquitto_sub -h %s -t game/board -C 1", broker_ip);
            fp = popen(sub2, "r");
        }
        if (!fp || fgets(boardResponse, sizeof(boardResponse), fp) == NULL) {
            perror("read bot failed");
            if (fp) pclose(fp);
            return;
        }
        pclose(fp);

        boardResponse[strcspn(boardResponse, "\r\n")] = 0;

        {
            char* t = strtok(boardResponse, ",");
            if (t) {
                strncpy(boardCopy, t, 9);
                boardCopy[9] = '\0';
                t = strtok(NULL, ",");
                result = (t && strlen(t) > 0) ? toupper(t[0]) : ' ';
            }
        }
        memcpy(board, boardCopy, 9);
        drawBoard();

        if (result == 'X') { printf("You win!\n"); break; }
        if (result == 'O') { printf("You lost.\n"); break; }
        if (result == 'T') { printf("It's a draw!\n"); break; }
    }

    Sleep(2000);
    printf("Game over. Press Enter to return to menu...");
    while (getchar()!='\n');
}

void runMode2() {
    int move;
    char result = ' ';
    char boardResponse[64];
    char boardCopy[10];
    const char* broker_ip = "34.53.123.240";
    const char* mqtt_path = "C:\\mosq\\mosquitto_pub.exe";

    int currentPlayer = 0;
    const char* topics[2] = {"player1", "player2"};
    const char symbols[2] = {'X', 'O'};

    char subInit[512];
    sprintf(subInit, "mosquitto_sub -h %s -t game/board -C 1", broker_ip);
    FILE* fp = popen(subInit, "r");
    if (fp && fgets(boardResponse, sizeof(boardResponse), fp)) {
        char* token = strtok(boardResponse, ",");
        if (token) {
            strncpy(boardCopy, token, 9);
            boardCopy[9] = '\0';
            token = strtok(NULL, ",");
            result = (token && strlen(token) > 0) ? toupper(token[0]) : ' ';
        }
        memcpy(board, boardCopy, 9);
    }
    if (fp) pclose(fp);

    drawBoard();

    while (1) {
        if (result == 'X') {
            printf("Player X wins!\n");
            break;
        } else if (result == 'O') {
            printf("Player O wins!\n");
            break;
        } else if (result == 'T') {
            printf("It's a draw!\n");
            break;
        }

        printf("Player %c, enter your move (1-9): ", symbols[currentPlayer]);
        scanf("%d", &move);
        getchar();

        if (move < 1 || move > 9) {
            printf("Invalid move. Try again.\n");
            continue;
        }

        char pubCmd[256];
        sprintf(pubCmd, "mosquitto_pub -h %s -t game/move/%s -m \"%d\"", broker_ip, topics[currentPlayer], move);
        system(pubCmd);

        char subCmd[512];
        sprintf(subCmd, "mosquitto_sub -h %s -t game/board -C 1", broker_ip);
        fp = popen(subCmd, "r");
        if (!fp || fgets(boardResponse, sizeof(boardResponse), fp) == NULL) {
            perror("Failed to receive board from ESP32");
            if (fp) pclose(fp);
            return;
        }
        pclose(fp);

        boardResponse[strcspn(boardResponse, "\r\n")] = 0;
        char* token = strtok(boardResponse, ",");
        if (token) {
            strncpy(boardCopy, token, 9);
            boardCopy[9] = '\0';
            token = strtok(NULL, ",");
            result = (token && strlen(token) > 0) ? toupper(token[0]) : ' ';
        }

        memcpy(board, boardCopy, 9);
        drawBoard();

        if (result == 'X') {
            printf("Player X wins!\n");
            break;
        } else if (result == 'O') {
            printf("Player O wins!\n");
            break;
        } else if (result == 'T') {
            printf("It's a draw!\n");
            break;
        }

        currentPlayer = 1 - currentPlayer;
    }

    Sleep(2000);
    printf("Game over. Press Enter to return to menu...");
    while (getchar() != '\n');
}


void runMode3() {
    char boardResponse[64];
    char boardCopy[10];
    char result = ' ';
    const char* broker_ip = "34.53.123.240";
    const char* mqtt_path = "C:\\mosq\\mosquitto_pub.exe";

    char resetCmd[256];
    sprintf(resetCmd, "%s -r -h %s -t game/board -m \"123456789, \"", mqtt_path, broker_ip);
    system(resetCmd);
    Sleep(1000);

    system("player1.exe");
    Sleep(1000);

    while (1) {

        int attempts = 0;
        do {
            FILE *fp = popen("mosquitto_sub -h 34.53.123.240 -t game/board -C 1", "r");
            if (!fp || fgets(boardResponse, sizeof(boardResponse), fp) == NULL) {
                perror("Failed to read board");
                if (fp) pclose(fp);
                return;
            }
            pclose(fp);

            boardResponse[strcspn(boardResponse, "\r\n")] = 0;

            char* token = strtok(boardResponse, ",");
            if (token) {
                strncpy(boardCopy, token, 9);
                boardCopy[9] = '\0';
                token = strtok(NULL, ",");
                result = (token && strlen(token) > 0) ? toupper(token[0]) : ' ';
                if (result != 'X' && result != 'O' && result != 'T') result = ' ';
            } else {
                result = ' ';
            }

            attempts++;
            if (attempts > 10) {
                printf("Timeout: no valid board received after 10 attempts.\n");
                return;
            }

            Sleep(500);
        } while (strcmp(boardCopy, "123456789") == 0);

        memcpy(board, boardCopy, 9);
        drawBoard();

        if (result == 'X') {
            printf("Player 1 (C) wins!\n");
            break;
        } else if (result == 'O') {
            printf("Player 2 (Bash) wins!\n");
            break;
        } else if (result == 'T') {
            printf("It's a draw!\n");
            break;
        }

        int countX = 0, countO = 0;
        for (int i = 0; i < 9; i++) {
            if (board[i] == 'X') countX++;
            else if (board[i] == 'O') countO++;
        }

        if (countX <= countO) {
            Sleep(1000);
            system("player1.exe");
        } else {
            Sleep(1000);
            char sshCmd[512];
            sprintf(sshCmd,
                "ssh -i C:/Users/Andre/.ssh/id_ed25519 andrewpalacios2004@%s '/home/andrewpalacios2004/Project3/player2.sh'",
                broker_ip);
            system(sshCmd);
        }

        int moveAttempts = 0;
        do {
            FILE *fp = popen("mosquitto_sub -h 34.53.123.240 -t game/board -C 1", "r");
            if (!fp || fgets(boardResponse, sizeof(boardResponse), fp) == NULL) {
                perror("Failed to read post-move board");
                if (fp) pclose(fp);
                return;
            }
            pclose(fp);

            boardResponse[strcspn(boardResponse, "\r\n")] = 0;
            char* token = strtok(boardResponse, ",");
            if (token) {
                strncpy(boardCopy, token, 9);
                boardCopy[9] = '\0';
                token = strtok(NULL, ",");
                result = (token && strlen(token) > 0) ? toupper(token[0]) : ' ';
                if (result != 'X' && result != 'O' && result != 'T') result = ' ';
            } else {
                result = ' ';
            }

            moveAttempts++;
            if (moveAttempts > 10) {
                printf("Timeout waiting for board after move.\n");
                return;
            }

            Sleep(500);
        } while (strcmp(boardCopy, "123456789") == 0);

        memcpy(board, boardCopy, 9);
        drawBoard();

        if (result == 'X') {
            printf("Player 1 (C) wins!\n");
            break;
        } else if (result == 'O') {
            printf("Player 2 (Bash) wins!\n");
            break;
        } else if (result == 'T') {
            printf("It's a draw!\n");
            break;
        }

        Sleep(1000);
    }

    Sleep(3000);
    int ch;
    printf("Game complete. Press Enter to return to menu...");
    while ((ch = getchar()) != '\n' && ch != EOF);
    getchar();

}



void showMenu() {
    int choice;
    while (1) {
        system("clear");
        printf("Welcome to ESP32 Tic-Tac-Toe\n");
        printf("-----------------------------\n");
        printf("1. Laptop vs Bash (1 player)\n");
        printf("2. Laptop vs Laptop (2 player)\n");
        printf("3. Auto Play (C vs Bash)\n");
        printf("4. Exit\n");
        printf("\nSelect a mode: ");
        scanf("%d", &choice);

        if (choice == 1) {
            runMode1();
        } else if (choice == 2) {
            runMode2();
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
