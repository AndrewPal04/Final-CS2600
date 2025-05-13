#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define BROKER_IP "34.53.123.240"
#define MQTT_PATH "C:\\mosq\\mosquitto_pub.exe"

int main() {
    char boardResponse[64], board[10];
    char result = ' ';
    srand(time(NULL));

    FILE *fp = popen("mosquitto_sub -h " BROKER_IP " -t game/board -C 1", "r");
    if (fp && fgets(boardResponse, sizeof(boardResponse), fp)) {
        sscanf(boardResponse, "%9[^,],%c", board, &result);
    }
    if (fp) pclose(fp);

    int move = -1;
    while (1) {
        int try = rand() % 9;
        if (board[try] != 'X' && board[try] != 'O') {
            move = try + 1;
            break;
        }
    }

    char cmd[256];
    sprintf(cmd, "%s -h %s -t game/move/player1 -m \"%d\"", MQTT_PATH, BROKER_IP, move);
    system(cmd);
    return 0;
}
