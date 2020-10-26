#include <stdlib.h>
#include <stdio.h>
#include "data.h"
#include <math.h>

struct datadef data;
char string[80];
float min_z,max_z;

void alloc_data()
{
  int i,j;

  data.comment.size = 0;
  data.comment.ptr = (char *) malloc(COMMENTMAXSIZE*sizeof(char));
  data.size=1;
  data.step=1;
  data.x=0;
  data.y=0;
  data.z=0;
  data.x_gain=10;
  data.y_gain=10;
  data.z_gain=10;
  data.z_freq=10;
  data.ptr = (float *) malloc(512*512*sizeof(float));
}

void free_data()
{
  free(data.ptr);
  free(data.comment.ptr);
}

main(int argc,char **argv)
{
        float x;
        float z;
        unsigned int i,j;
        
    alloc_data();
    if (argc<3)
    {
        printf("Usage: minmax <oldfile.stm> <newfile.stm>\n");
        exit(1);
    }
   load_image(argv[1]);
   find_min_max();
printf("minz %f maxz %f\n",min_z,max_z);       
    for(j=0;j<data.size;j++)
        for(i=0;i<data.size;i++)
        {

/*
            printf("%f\n",(*(data.ptr+j*data.size+i)-min_z)/(max_z-min_z)*9+1);
*/
            z=(*(data.ptr+j*data.size+i)-min_z)/(max_z-min_z)*9+1;
/*
            printf("%f\n",z);
*/
            *(data.ptr+j*data.size+i)=log10((double) z)*255;

        }
    

    save_image(argv[2]);
/*
  printf("data.comment.size = %u;\n",(data.comment.size));
  *(data.comment.ptr+data.comment.size) = '\0';
  printf("*data.comment.ptr = %s;\n",data.comment.ptr);
  printf("data.size = %u;\n",(data.size));
  printf("data.step = %u;\n",(data.step));
  printf("data.x = %u;\n",(data.x));
  printf("data.y = %u;\n",(data.y));
  printf("data.z = %u;\n",(data.z));
  printf("data.x_gain = %f;\n",(data.x_gain));
  printf("data.y_gain = %f;\n",(data.y_gain));
  printf("data.z_gain = %f;\n",(data.z_gain));
  printf("data.z_freq = %u;\n",(data.z_freq));
  for(j=0;j<data.size;j++)
  {
    for (i=0;i<data.size;i++) printf("%f ",*(data.ptr+j*data.size+i));
    printf("\n");
  }
*/
   
}

void save_image(char *filename)
{
  FILE *fp;
 
  fp = fopen(filename,"wb");
  fwrite(&data.comment.size,sizeof(data.comment.size),1,fp);
  fwrite(data.comment.ptr,sizeof(char),(size_t)data.comment.size,fp);
  fwrite(&data.size,sizeof(data.size),1,fp);
  fwrite(&data.step,sizeof(data.step),1,fp);
  fwrite(&data.x,sizeof(data.x),1,fp);
  fwrite(&data.y,sizeof(data.y),1,fp);
  fwrite(&data.z,sizeof(data.z),1,fp);
  fwrite(&data.x_gain,sizeof(data.x_gain),1,fp);
  fwrite(&data.y_gain,sizeof(data.y_gain),1,fp);
  fwrite(&data.z_gain,sizeof(data.z_gain),1,fp);
  fwrite(&data.z_freq,sizeof(data.z_freq),1,fp);
  fwrite(data.ptr,sizeof(float),(size_t)(data.size*data.size),fp);
  fclose(fp);

  strset(string,'\0');
  strncpy(string,filename,strlen(filename)-strlen(strrchr(filename,'.')));
  strcat(string,".dx");
  fp = fopen(string,"w");
  fprintf(fp,"file=%s\n",strlwr(strrchr(filename,'\\')+1));
  fprintf(fp,"grid=%dx%d\n",data.size,data.size);
  fprintf(fp,"format=lsb binary\n");
  fprintf(fp,"type=float\n");
  fprintf(fp,"majority=row\n");
  fprintf(fp,"header=bytes %d\n",sizeof(data.comment.size)+
				 sizeof(char)*data.comment.size+
				 sizeof(data.size)+
				 sizeof(data.step)+
				 sizeof(data.x)+
				 sizeof(data.y)+
				 sizeof(data.z)+
				 sizeof(data.x_gain)+
				 sizeof(data.y_gain)+
				 sizeof(data.z_gain)+
				 sizeof(data.z_freq));
  fclose(fp);
}

void find_min_max()
{
    unsigned int i,j;

    min_z=70000;
    max_z=-1;
    for(j=0;j<data.size;j++)
    {
        for(i=0;i<data.size;i++)
        {
            if (*(data.ptr+j*data.size+i)<min_z) min_z=*(data.ptr+j*data.size+i);
            else if (*(data.ptr+j*data.size+i)>max_z) max_z=*(data.ptr+j*data.size+i);
        }
    }
}

void load_image(char *filename)
{
  FILE *fp;

  fp = fopen(filename,"rb");
  fread(&data.comment.size,sizeof(data.comment.size),1,fp);
  fread(data.comment.ptr,sizeof(char),(size_t)data.comment.size,fp);
  fread(&data.size,sizeof(data.size),1,fp);
  fread(&data.step,sizeof(data.step),1,fp);
  fread(&data.x,sizeof(data.x),1,fp);
  fread(&data.y,sizeof(data.y),1,fp);
  fread(&data.z,sizeof(data.z),1,fp);
  fread(&data.x_gain,sizeof(data.x_gain),1,fp);
  fread(&data.y_gain,sizeof(data.y_gain),1,fp);
  fread(&data.z_gain,sizeof(data.z_gain),1,fp);
  fread(&data.z_freq,sizeof(data.z_freq),1,fp);
  fread(data.ptr,sizeof(float),(size_t)(data.size*data.size),fp);
  fclose(fp);
  *(data.comment.ptr+data.comment.size) = '\0';
}
        

