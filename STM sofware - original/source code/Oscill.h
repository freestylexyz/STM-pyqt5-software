typedef struct oscill_data_
{
        unsigned int time;
        unsigned short data;
} oscill_data;
#define O_DATA_MAX 100000L
#define O_LOWER_MIN 0L
#define O_HIGHER_MAX 65535L
#define O_OFFSET_MIN (-32768L)
#define O_OFFSET_MAX 32767L
#define O_VOLT_DELT 328L
#define O_VOLT_DDELT 3277L
#define O_TIME_MIN 20L
#define O_TIME_MAX 100000L
#define O_TIME_DELT 100L
#define O_TIME_DDELT 1000L
#define O_X_MIN 15
#define O_X_MAX 310
#define O_Y_MIN 16
#define O_Y_MAX 333
#define MAGIC_VAL 32820
#define O_TRIGGER_MIN 1
#define O_TRIGGER_MAX 65534
