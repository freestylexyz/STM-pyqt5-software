#include <stdlib.h>
#include <malloc.h>
#include "common.h"
#include "data.h"
#include "stm.h"
#include "dio.h"

datadef *data,*all_data[1000];
struct commentdef *gcomment;
char string[100];

void alloc_data(datadef **dataptr,int type,int size,int x_type,int y_type,
    int seq_size)
{
// malloc, initialize fields for datadef

  (*dataptr) = (datadef *)malloc(sizeof(datadef));
  (*dataptr)->comment.size = 0;
  (*dataptr)->comment.ptr = (char *)malloc(COMMENTMAXSIZE*sizeof(char));
  *((*dataptr)->comment.ptr) = 0;
  (*dataptr)->sample_type.size = 0;
  (*dataptr)->sample_type.ptr = (char *)malloc(SAMPLETYPESIZE*sizeof(char));
  *((*dataptr)->sample_type.ptr) = 0;
  (*dataptr)->dosed_type.size = 0;
  (*dataptr)->dosed_type.ptr = (char *)malloc(SAMPLETYPESIZE*sizeof(char));
  *((*dataptr)->dosed_type.ptr) = 0;
  (*dataptr)->size = 1;
  (*dataptr)->step = 1;
  (*dataptr)->x = 0;
  (*dataptr)->y = 0;
  (*dataptr)->z = 0;
  (*dataptr)->x_gain = 10;
  (*dataptr)->y_gain = 10;
  (*dataptr)->z_gain = 10;
  (*dataptr)->z_freq = 10;
  (*dataptr)->ptr = NULL;
  (*dataptr)->time2d = NULL;
  (*dataptr)->data2d = NULL; 
  (*dataptr)->xf=(*dataptr)->yf = NULL;
  (*dataptr)->xd=(*dataptr)->yd = NULL;
  (*dataptr)->pal.dacbox = NULL;
  (*dataptr)->pal.logpal = NULL;
  (*dataptr)->saved = 0;
  (*dataptr)->min_z = 0;
  (*dataptr)->max_z = 1;
  (*dataptr)->valid = 0;
  (*dataptr)->x_range = 2; /*default +-10 V */
  (*dataptr)->y_range = 2;
  (*dataptr)->x_offset_range = 2; /*default +-10 V */
  (*dataptr)->y_offset_range = 2;
  (*dataptr)->bias_range = 2; /*default +-10 V */
  (*dataptr)->i_set_range = 2;
  (*dataptr)->amp_gain = 8;
  (*dataptr)->dosed_langmuir = 0;
  (*dataptr)->i_setpoint = 2048;
  (*dataptr)->version = STM_DATA_VERSION;
  (*dataptr)->step_delay = 0;
  (*dataptr)->inter_step_delay = 0;
  (*dataptr)->digital_feedback = 0;
  (*dataptr)->tip_spacing = 0.5;
  (*dataptr)->inter_step_delay = 0;
  (*dataptr)->scan_num = 1;
  (*dataptr)->scan_feedback = 1;
  (*dataptr)->read_ch = zi_ch;
  (*dataptr)->crash_protection = 0;
  (*dataptr)->overshoot = 0;
  (*dataptr)->overshoot_percent = 0;
  (*dataptr)->overshoot_wait1 = 0;
  (*dataptr)->overshoot_wait2 = 0;
  (*dataptr)->filename[0] = '\0';
  (*dataptr)->dep_filename[0] = '\0';
  (*dataptr)->full_filename[0] = '\0';
  (*dataptr)->measure_i = 0;
  (*dataptr)->measure_z = 0;
  (*dataptr)->measure_2 = 0;
  (*dataptr)->measure_3 = 0;
  (*dataptr)->dither0 = 0;
  (*dataptr)->dither1 = 0;
  (*dataptr)->track_offset_x = 0;
  (*dataptr)->track_offset_y = 0;
  (*dataptr)->track_max_bits = 0;
  (*dataptr)->track_sample_every = 0;
  (*dataptr)->tracking_mode = 0;
  (*dataptr)->track_auto_auto = 0;
  (*dataptr)->track_iterations = 0;
  (*dataptr)->track_every = 0;
  (*dataptr)->z_offset_bias = 0;
  (*dataptr)->num_passes = 0;
  (*dataptr)->bias_speed_step = 0;
  (*dataptr)->temperature = 0;
  (*dataptr)->read_seq = NULL;
  (*dataptr)->read_seq_num = seq_size;
  (*dataptr)->current_read_seq = 0;
  (*dataptr)->track_at_min = 0;
  (*dataptr)->offset_hold = 0;
  (*dataptr)->feedback_at_min = 0;
  (*dataptr)->track_plane_fit_a = 0.0;
  (*dataptr)->track_plane_fit_b = 0.0;
  (*dataptr)->lockin_sen1 = 0.0;
  (*dataptr)->lockin_sen2 = 0.0;
  (*dataptr)->lockin_osc = 0.0;
  (*dataptr)->lockin_w = 0.0;
  (*dataptr)->lockin_tc = 0.0;
  (*dataptr)->lockin_n1 = 0.0;
  (*dataptr)->lockin_pha1 = 0.0;
  (*dataptr)->lockin_pha2 = 0.0;
  (*dataptr)->lockin_in_use = LOCKIN_NOT_USED;
  (*dataptr)->total_steps = 0;
  (*dataptr)->bit16 = 0;

  (*dataptr)->total_steps = 1;
  (*dataptr)->sequence[0].type = READ_TYPE;
  (*dataptr)->sequence[0].state = 1;
  (*dataptr)->sequence[0].wait = 1;
  (*dataptr)->sequence[0].out_ch = 0;
  (*dataptr)->sequence[0].step_bias = 0;
  (*dataptr)->sequence[0].in_ch  = zi_ch;
  (*dataptr)->sequence[0].num_reads = 5;
  strcpy((*dataptr)->sequence[0].step_type_string,"Read");

  alloc_data_ptrs(dataptr,type,size,x_type,y_type,seq_size);
  unequalize(*dataptr);
}
  
void alloc_data_ptrs(datadef **dataptr,int type,int size,int x_type,int y_type,int seq_size)
{
// malloc datadef's data pointers depending on type and size of data
	
  (*dataptr)->x_type = x_type;
  (*dataptr)->y_type = y_type;
  (*dataptr)->type = type;

  switch(type)
  {
    case DATATYPE_3D_PARAMETERS:
        (*dataptr)->size = size;
        alloc_data_seq(dataptr,seq_size);
        break;
    case DATATYPE_3D:
        (*dataptr)->ptr = (float *)malloc(size * size * sizeof(float));
        alloc_data_seq(dataptr,seq_size);
        (*dataptr)->pal.type = PALTYPE_MAPPED;
        (*dataptr)->pal.dacbox = (unsigned char *)malloc(SIZEOFMPAL);
        break;
    case DATATYPE_2D:
        (*dataptr)->size = size;
        (*dataptr)->time2d = (float *)malloc(size * sizeof(float));
        (*dataptr)->data2d = (unsigned short *)malloc(size * sizeof(unsigned short));
        break;
    case DATATYPE_SPEC_I:
    case DATATYPE_SPEC_Z:
    case DATATYPE_SPEC_DIDV:
    case DATATYPE_MASS:
        (*dataptr)->size = size;
        (*dataptr)->ptr = (float *)malloc(size * sizeof(float));
        break;
    case DATATYPE_GEN2D:
        (*dataptr)->size = size;
        switch(x_type)
        {
            case GEN2D_FLOAT:
                (*dataptr)->xf = (float *)malloc(size * sizeof(float));
                break;
            case GEN2D_USHORT:
                (*dataptr)->xd = (unsigned short *)malloc(size * sizeof(unsigned short));
                break;
        }
        switch(y_type)
        {
            case GEN2D_FLOAT:
                (*dataptr)->yf = (float *)malloc(size * sizeof(float));
                break;
            case GEN2D_USHORT:
                (*dataptr)->yd = (unsigned short *)malloc(size * sizeof(unsigned short));
                break;
        }
		break;
    case DATATYPE_HOP:
        (*dataptr)->size = size;
        (*dataptr)->xd = (unsigned short *)malloc(size * sizeof(unsigned short));
        (*dataptr)->yd = (unsigned short *)malloc(size * sizeof(unsigned short));
        //(*dataptr)->time2d = (float *) malloc(size*sizeof(float));
		break;	
  }
}

void unequalize(datadef *dataptr)
{

    dataptr->equalized = 0;

#ifdef OLD
    int i,j;
    
  j=1;
  for(i=0;i<DATA_HIST_MAX;i++)
  {
    if (i<j*DATA_HIST_MAX/LOGPALUSED) dataptr->hist[i]=(j-1);
    else
    {
        dataptr->hist[i]=j++;
    }
  }
#endif

}

void free_data(datadef *(*dataptr))
{
// free up all ptrs that were malloc'ed by alloc_data

  if((*dataptr) != NULL)
  {
      free_data_ptrs(dataptr);
  
      free((*dataptr)->comment.ptr);
      free((*dataptr)->sample_type.ptr);
      free((*dataptr)->dosed_type.ptr);
      free((*dataptr));
      (*dataptr) = NULL;
  }
}

void free_data_ptrs(datadef *(*dataptr))
{
// free up all ptrs that were malloc'ed by alloc_data_ptrs

  if((*dataptr)->ptr != NULL) free((*dataptr)->ptr);
  if((*dataptr)->time2d != NULL) free((*dataptr)->time2d);
  if((*dataptr)->data2d != NULL) free((*dataptr)->data2d);
  if((*dataptr)->xf != NULL) free((*dataptr)->xf);
  if((*dataptr)->yf != NULL) free((*dataptr)->yf);
  if((*dataptr)->xd != NULL) free((*dataptr)->xd);
  if((*dataptr)->yd != NULL) free((*dataptr)->yd);
  if((*dataptr)->pal.dacbox != NULL) free((*dataptr)->pal.dacbox);
  free_data_seq(dataptr);
  destroy_logpal(&((*dataptr)->pal.logpal));

  (*dataptr)->ptr = NULL;
  (*dataptr)->time2d = NULL;
  (*dataptr)->data2d = NULL; 
  (*dataptr)->xf = (*dataptr)->yf = NULL;
  (*dataptr)->xd = (*dataptr)->yd = NULL;
  (*dataptr)->pal.dacbox = NULL;
  (*dataptr)->pal.logpal = NULL;

}

void alloc_data_seq(datadef ** dataptr,int seq_size)
{
// malloc enough space for entire READ_SEQ array

  (*dataptr)->read_seq_num = seq_size;
  if((*dataptr)->current_read_seq >= seq_size) 
            (*dataptr)->current_read_seq = seq_size - 1;
  (*dataptr)->read_seq = (READ_SEQ *)malloc(sizeof(READ_SEQ) * seq_size);
}

void free_data_seq(datadef **dataptr)
{
// free up all ptrs that were malloc'ed by alloc_data

  if((*dataptr)->read_seq != NULL) free((*dataptr)->read_seq);
  (*dataptr)->read_seq = NULL;
  (*dataptr)->read_seq_num = 0;
//  (*dataptr)->current_read_seq=0;
}    
