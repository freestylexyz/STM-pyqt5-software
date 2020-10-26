#define TRACE_UNDEFINED 0
#define TRACE_IN_STATE 1
#define TRACE_OUT_STATE 2

typedef struct trace_eldef
{
        float width;
        struct trace_eldef *next;
        struct trace_eldef *prev;
} TRACE_EL;

typedef struct trace_bindef
{
        int *array;
        int size;
        float width;
} TRACE_BIN;

typedef  struct time_binsdef 
{ float time; int num; } TIME_BINS;


