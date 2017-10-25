#include <MD5.h>

char input = NULL;
const int tamMsg = 32;
const int tamPack = 68;
const int tamMd5 = 33;
const char key[] = "tfg123";


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(A0, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}
int i;
unsigned long tempo;

int sp = 400;
int ad, res;
//intermediate variables
float f_e0 = 0, f_e1 = 0, f_e2 = 0;
float f_y0 = 0, f_y1 = 0;
float f_kp = 1, f_ki = 100, f_kd = 0;
float f_T = 0.001;

#define SHIFT (256)
int16_t i_e0 = 0, i_e1 = 0, i_e2 = 0;
int16_t i_y0 = 0, i_y1 = 0;

#define MAX_SAT 1023
#define MIN_SAT 0
int pid_sw_fixed(int ad);

void loop() {
  tempo = micros() + 1000;
  if (Serial.available() > 0) {
    input = (char) Serial.read();
    waitingMsg(input);
  }
  i++;
  if (i >= 1000) {
    i = 0;
    if (sp == (600)) {
      sp = (400);
      digitalWrite(2, LOW);
    } else {
      digitalWrite(2, HIGH);
      sp = (600);
    }
  }
  ad = analogRead(0);
  res = pid_sw_fixed(ad) >> 2;

  analogWrite(9, res);
  while (tempo > micros());

}

void waitingMsg(char input) {
  if (input == '$') {
    char pack[tamPack] = {NULL};
    int pos = 0;
    pack[pos] = input;
    pos++;
    receiving(pack, pos);
  }
}

void subchar(char string[], int first, int last, char ret[]) {
  int pos = 0;
  for (int i = first; i < last; i++) {
    ret[pos] = string[i];
    pos++;
  }
  ret[pos] = '\0';
  pos++;
}

void addchar(char string1[], char string2[], char ret[]) {
  int pos = 0;
  for (int i = 0; i < siz(string1); i++) {
    ret[pos] = string1[i];
    pos++;
  }

  for (int j = 0; j < siz(string2); j++) {
    ret[pos] = string2[j];
    pos++;
  }

  ret[pos] = '\0';
  pos++;

}

int siz(char ptr[]) {
  int count = 0;
  while (ptr[count] != '\0')
  {
    ++count;
  }
  return count;
}

void receiving(char pack[tamPack], int pos) {
  char next = NULL;
  while (true) {
    if (pos > (tamPack - 1)) {
      break;
    } else {
      if (next == NULL) {
        while (true) {
          if (Serial.available() > 0) {
            input = (char) Serial.read();

            break;
          }
        }
      } else {
        input = next;
        next = NULL;
      }
      if (input == '/') {
        while (true) {
          if (Serial.available() > 0) {
            next = (char) Serial.read();
            break;
          }
        }
        if (next == 'n') {
          pack[pos] = input;
          pos++;
          pack[pos] = next;
          next = NULL;
          unpack(pack);
          break;
        } else {
          pack[pos] = input;
          pos++;
        }
      } else if (input == '$') {
        waitingMsg(input);
        break;
      } else {
        pack[pos] = input;
        pos++;
      }
    }
  }
}

void unpack(char pack[tamPack]) {
  char msgReceived[tamMsg] = {NULL};
  char md5Received[tamMd5] = {NULL};
  int pos = 0;
  for (int i = 1; i < siz(pack); i++) {
    char c = pack[i];
    if (c == ',') {
      subchar(pack, 1, i, msgReceived);
      pos = i + 1;
    }
    if (c == '/') {
      char d = pack[i + 1];
      if ( d == 'n') {
        subchar(pack, pos, i, md5Received);
      }
    }
  }
  validate(msgReceived, md5Received);
}

void validate(char msgReceived[tamMsg], char md5Received[tamMd5]) {
  char key_msg[siz(key) + siz(msgReceived)] = {NULL};
  addchar(key, msgReceived, key_msg);
  unsigned char* hash = MD5::make_hash(key_msg);
  char *md5str = MD5::make_digest(hash, 16);
  int count = 0;
  if ((siz(md5str)) == (siz(md5Received))) {
    for (int i = 0; i < siz(md5Received); i++) {
      if (md5str[i] == md5Received[i]) {
        count++;
      }
    }
  }
  if (count == 32) {
    count = 0;
    processingMsg(msgReceived);
  }
}

void processingMsg(char msgReceived[tamMsg]) {
  //aplicacao
  char command[tamMsg] = {NULL};
  float number = 0;
  int ini = 0;
  for (int i = 0; i < siz(msgReceived); i++) {
    if (msgReceived[i] == '#') {
      subchar(msgReceived, (ini + 1), i, command);
      if (msgReceived[ini] == 'P') {
        f_kp = atof(command);
        digitalWrite(3, HIGH);
        delay(5000);
        digitalWrite(3, LOW);
      }
      if (msgReceived[ini] == 'I') {
        f_ki = atof(command);
        digitalWrite(4, HIGH);
        delay(5000);
        digitalWrite(4, LOW);
      }
      if (msgReceived[ini] == 'D') {
        f_kd = atof(command);
        digitalWrite(5, HIGH);
        delay(5000);
        digitalWrite(5, LOW);
      }
      if (msgReceived[ini] == 'T') {
        f_T = atof(command);
        digitalWrite(6, HIGH);
        delay(5000);
        digitalWrite(6, LOW);
      }
      ini = i + 1;
    }
  }
  building(msgReceived);
}

//-----------------------------Sending---------------------------------

void building(char msg[tamMsg]) {
  char key_msg[siz(key) + siz(msg)] = {NULL};
  addchar(key, msg, key_msg);
  unsigned char* hash = MD5::make_hash(key_msg);
  char *md5str = MD5::make_digest(hash, 16);

  int pos = 0;
  char pack[tamPack] = {NULL};
  pack[pos] = '$';
  pos++;
  for (int i = 0; i < siz(msg) ; i++) {
    pack[pos] = msg[i];
    pos++;
  }
  pack[pos] = ',';
  pos++;

  for (int i = 0; i < siz(md5str) ; i++) {
    pack[pos] = md5str[i];
    pos++;
  }

  pack[pos] = '/';
  pos++;
  pack[pos] = 'n';
  pos++;
  sending(pack);

}

void sending(char msg[]) {
  for (int i = 0; i < siz(msg); i++) {
    Serial.println(msg[i]);
  }
}

//-----------------------------PID---------------------------------

int pid_sw_fixed(int ad) {
  int16_t i_k1 = (f_kp + f_ki * f_T + f_kd / f_T) * SHIFT;
  int16_t i_k2 = -((f_kp + 2 * f_kd / f_T) * SHIFT);
  int16_t i_k3 = (f_kd / f_T) * SHIFT;

  // Update variables
  i_y1 = i_y0;
  i_e2 = i_e1;
  i_e1 = i_e0;

  //ad = READ_AD())
  i_e0 = (sp - ad);

  // Processing
  //the multiplication is for a number range in 4.2(6 bits) (gains from zero up to +15.75)
  // times 0.3ff (10 bits)
  //    y0 = y1 + (i_kp * (e0 - e2)) +
  //            (i_ki * (e0) * i_T) +
  //            (i_kd * (e0 - (2 * e1) + e2) / i_T);
  //stated in terms of errors instead of coefficients
  i_y0 = (((int32_t) i_k1 * i_e0) + ((int32_t) i_k2 * i_e1) + ((int32_t) i_k3 * i_e2));
  i_y0 = i_y0 >> 8;
  i_y0 += i_y1;

  // Saturation
  if (i_y0 > MAX_SAT) {
    i_y0 = MAX_SAT;
  } else if (i_y0 < MIN_SAT) {
    i_y0 = MIN_SAT;
  }

  return i_y0;
}


