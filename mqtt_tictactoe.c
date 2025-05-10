#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#define BROKER_IP "34.xxx.xxx.xxx" // Replace here
#define PLAYER1_TOPIC "player1/move"
#define PLAYER2_TOPIC "player2/move"
#define RESULT_TOPIC "game/result"

char board[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

void display_board() {
    printf("\n %c | %c | %c\n", board[0], board[1], board[2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c\n", board[3], board[4], board[5]);
    printf("---|---|---\n");
    printf(" %c | %c | %c\n\n", board[6], board[7], board[8]);
}

void publish_move(const char *topic, int move) {
    char command[256];
    snprintf(command, sizeof(command),
             "mosquitto_pub -h %s -t %s -m \"%d\"", BROKER_IP, topic, move);
    system(command);
}

void publish_result(const char *message) {
    char command[256];
    snprintf(command, sizeof(command),
             "mosquitto_pub -h %s -t %s -m \"%s\"", BROKER_IP, RESULT_TOPIC, message);
    system(command);
}

char check_winner() {
    int wins[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };

    for (int i = 0; i < 8; i++) {
        int a = wins[i][0], b = wins[i][1], c = wins[i][2];
        if (board[a] != ' ' && board[a] == board[b] && board[b] == board[c])
            return board[a];
    }

    return ' ';
}

int is_draw() {
    for (int i = 0; i < 9; i++)
        if (board[i] == ' ')
            return 0;
    return 1;
}

int main() {
    printf("Press 1-9 to make a move\nPress Q to quit\n");

    char current_player = 'X';
    char input[10];

    while (1) {
        display_board();
        printf("[Player %c] > ", current_player);
        fgets(input, sizeof(input), stdin);

        if (input[0] == 'Q' || input[0] == 'q') break;

        int move = atoi(input);
        if (move < 1 || move > 9 || board[move - 1] != ' ') {
            printf("Invalid move. Try again.\n");
            continue;
        }

        board[move - 1] = current_player;

        // Publish move to MQTT
        if (current_player == 'X')
            publish_move(PLAYER1_TOPIC, move);
        else
            publish_move(PLAYER2_TOPIC, move);

        // Check for winner
        char winner = check_winner();
        if (winner != ' ') {
            display_board();
            char result[50];
            snprintf(result, sizeof(result), "Player %c wins!", winner);
            publish_result(result);
            printf("%s\n", result);
            break;
        }

        // Check for draw
        if (is_draw()) {
            display_board();
            publish_result("Draw!");
            printf("It's a draw!\n");
            break;
        }

        // Switch player
        current_player = (current_player == 'X') ? 'O' : 'X';
    }

    return 0;
}
