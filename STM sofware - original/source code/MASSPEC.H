#define M_IN_CH      3
#define M_OUT_CH      11
#define M_X_MIN 16
#define M_X_MAX 309
#define M_Y_MIN 17
#define M_Y_MAX 332
#define M_V_PER_AMU     0.05
#define M_MIN_MIN       2048
#define M_MIN_MAX       4095
#define M_MAX_MIN       2048
#define M_MAX_MAX       4095
#define M_SPEED_MIN     0.1
#define M_SPEED_MAX     5
#define M_SPEED_DELT    0.1
#define M_SPEED_DDELT   0.5
#define M_PTR_MIN       0
#define M_PTR_MAX       2047
#define M_SCAN_DELT     1
#define M_SCAN_DDELT    5
#define M_VERT_MAX      0xFFFF/2
#define M_VERT_MIN      1
#define M_VERT_DELT     0xFFFF/200
#define M_VERT_DDELT    0xFFFF/20

float m_v_to_amu(float);
float m_amu_to_v(float);

void load_mas_old(char *);

