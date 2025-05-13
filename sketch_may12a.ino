#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SpectrumSetup-E5_EXT";
const char* password = "brightroad484";
const char* mqtt_server = "34.53.123.240"; // change here

WiFiClient espClient;
PubSubClient client(espClient);

char board[9] = {'1','2','3','4','5','6','7','8','9'};
bool gameOver = false;

void resetBoard() {
  for (int i = 0; i < 9; i++) {
    board[i] = '1' + i;
  }
  gameOver = false;
}

char checkWin() {
  int wins[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8},
    {0,3,6}, {1,4,7}, {2,5,8},
    {0,4,8}, {2,4,6}
  };
  for (int i = 0; i < 8; i++) {
    if (board[wins[i][0]] == board[wins[i][1]] &&
        board[wins[i][1]] == board[wins[i][2]])
      return board[wins[i][0]];
  }
  for (int i = 0; i < 9; i++) {
    if (board[i] != 'X' && board[i] != 'O') return ' ';
  }
  return 'T';
}

char determinePlayerFromTopic(char* topic) {
  if (strstr(topic, "player1") != NULL) return 'X';
  if (strstr(topic, "player2") != NULL) return 'O';
  return '?';
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (gameOver) {
    Serial.println("Game is over, ignoring move.");
    return;
  }

  char msg[16];
  strncpy(msg, (char*)payload, length);
  msg[length] = '\0';

  Serial.print("Received MQTT message on topic [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  int pos = atoi(msg) - 1;
  char player = determinePlayerFromTopic(topic);

  Serial.print("Parsed move position: ");
  Serial.println(pos);
  Serial.print("Player detected: ");
  Serial.println(player);

  if (pos < 0 || pos > 8 || board[pos] == 'X' || board[pos] == 'O') {
    Serial.println("Invalid move or position already taken.");
    return;
  }

  board[pos] = player;
  char result = checkWin();

  char out[16];
  snprintf(out, sizeof(out), "%.9s,%c", board, result);
  boolean ok = client.publish("game/board", out, true);

  if (ok) {
    Serial.print("Published board: ");
    Serial.println(out);
  } else {
    Serial.println("Failed to publish board update!");
  }

  delay(500);

  if (result == 'X' || result == 'O' || result == 'T') {
    Serial.print("Game over (");
    Serial.print(result);
    Serial.println("), resetting board");

    gameOver = true;

    delay(2000); 

    resetBoard();

    char resetMsg[16];
    snprintf(resetMsg, sizeof(resetMsg), "%.9s,%c", board, ' ');
    boolean resetOk = client.publish("game/board", resetMsg, true);

    if (resetOk) {
      Serial.print("Published reset board: ");
      Serial.println(resetMsg);
    } else {
      Serial.println("Failed to publish reset board!");
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Tic-Tac-Toe Ready");

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED. Stopping.");
    while (true);
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.print("Connecting to MQTT...");
  while (!client.connected()) {
    if (client.connect("ESP32TicTac")) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  client.subscribe("game/move/#");
  Serial.println("Subscribed to topic: game/move/#");

  resetBoard();

  char startupMsg[16];
  snprintf(startupMsg, sizeof(startupMsg), "%.9s,%c", board, ' ');
  boolean published = client.publish("game/board", startupMsg, true);

  if (published) {
    Serial.print("Published initial board on startup: ");
    Serial.println(startupMsg);
  } else {
    Serial.println("Failed to publish initial board!");
  }
}

void loop() {
  client.loop();
}
