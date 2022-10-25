#define SW_VERSION 8
void Menu();
void clearScreen();

typedef enum types{
    UCHAR,
    UINT,
    CHAR,
    INT
} varType;
varType typef;

struct inputFreq{
  unsigned long ontime;
  unsigned long offtime;
  unsigned long period;
  float freq;
};
typedef struct inputFreq inputFreq;
