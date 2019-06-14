#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <string.h>

#define DIM_MAX 2400 //massima risuluzione video

//Descrittore del file video /dev/video
int fd;
int type;
struct v4l2_requestbuffers bufrequest;
struct v4l2_buffer bufferinfo;
void* buffer_start;
void* mq;
void* mqs;

/** azzera tutti i pixel della matrice che non hanno il valore massimo */

void soglia_matrice(void* mat,size_t dim,size_t pixel_bytes,double sog)
{
  unsigned short min=65535;
  size_t i=0;
  //cerco il massimo
  for(i=0;i<dim*dim;i++)
    {
      if((*(unsigned short*)(mat+i*pixel_bytes))<=min)
        min=(*(unsigned short*)(mat+i*pixel_bytes));
    }
  //soglio la matrice
  for(i=0;i<dim*dim;i++)
    {
      if((*(unsigned short*)(mat+i*pixel_bytes))>(unsigned short)(min))
        (*(unsigned short*)(mat+i*pixel_bytes))=1;
      else
        (*(unsigned short*)(mat+i*pixel_bytes))=0;
    }
}

void punto_matrice(void* mat,size_t dim,size_t pixel_bytes,size_t* r,size_t* c)
{
  unsigned short min=65535;
  size_t i,j;
  //cerco il massimo
  for(i=0;i<dim;i++)
    {
      for(j=0;j<dim;j++)
        {
          
          size_t pos=i*dim+j;
          if((*(unsigned short*)(mat+pos*pixel_bytes))<=min)
            {
              min=(unsigned short)(*(unsigned short*)(mat+pos*pixel_bytes));
              *r=i;
              *c=j;
             
             
            }
        }
    }
  
  
}




/** riceve una matrice mxn e la ritaglia a mxm */
void quadra_matrice(void * mat_quad, void* mat, size_t m, size_t n,size_t pixel_bytes)
{
  size_t i;
  for(i=0;i<m;i++)
    memcpy(mat_quad+(i*m*pixel_bytes),mat+(i*pixel_bytes)*n,(pixel_bytes)*m);
}


/** riceve i puntatori a due buffer, la dimensione della matrice e il numero
 * di bytes per pixel
 */
void scala_matrice(void * scalata,void * matrice, size_t pixel_bytes,size_t dim)
{
 
  /**
   * Crea una matrice quadrata delle dimesione dim/2 x dim/2 
   * dimezzando prima le colonne poi le righe
   */
  
  size_t r,c;
  unsigned short a;
  //Crea matrice dim x dim/2
  for (r=0;r<dim/2;r++)
    {
      for (c=0;c<dim/2;c++)
        {
          (*(unsigned short*)(scalata+r*dim/2*pixel_bytes+c*pixel_bytes)=

           (*(unsigned short*)(matrice+2*r*dim*pixel_bytes+2*c*pixel_bytes)));
        }
    }
}


void LE_2_BE(void * mat,size_t dim)
{
 
  /**
   * Crea una matrice quadrata delle dimesione dim/2 x dim/2 
   * dimezzando prima le colonne poi le righe
   */
  
  size_t r,c;
  
  for (r=0;r<dim;r++)
    {
      for (c=0;c<dim*2;c+=2)
        {
          size_t pos=r*(dim*2)+c;

          char b1,b2;

          b1=*((char *)mat+pos+1);

          b2=*((char *)mat+pos);

          *((char *)mat+pos+1)=b2;

          *((char *)mat+pos)=b1;
        }
    }
}




void attiva_camera(char * video_dev_name)
{
  if((fd = open(video_dev_name, O_RDWR)) < 0){
    perror("open");
    exit(1);
  }
  
  struct v4l2_capability cap;
  if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
    perror("VIDIOC_QUERYCAP");
    exit(1);
  }
  printf("Driver: %s, card: %s", cap.driver, cap.card);
  
  struct v4l2_format format;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  format.fmt.pix.width = 640;
  format.fmt.pix.height = 480;
  
  if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
    perror("VIDIOC_S_FMT");
    exit(1);
  }

   bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = 1;
  
  if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
    perror("VIDIOC_REQBUFS");
    exit(1);
  }
  
 
  memset(&bufferinfo, 0, sizeof(bufferinfo));
  
  bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo.memory = V4L2_MEMORY_MMAP;
  bufferinfo.index = 0;
  
  if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
    perror("VIDIOC_QUERYBUF");
    exit(1);
  }
  
  buffer_start = mmap(
                            NULL,
                            bufferinfo.length,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            fd,
                            bufferinfo.m.offset
                            );
  
  if(buffer_start == MAP_FAILED){
    perror("mmap");
    exit(1);
  }
   /** buffer per la matrice quadrata */
   mq=malloc(480*480*2);
   mqs=malloc(240*240*2);
}

void chiudi_camera()
{
  /** Disattiva streaming */
  if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
    perror("VIDIOC_STREAMOFF");
    exit(1);
  }
  free(mq);
  free(mqs);
  close(fd);
}

int leggi_camera(double inp[])
{
    
  
 

 
  if(mq==0||mqs==0)
    {
      perror("Matrice quadrata");
      exit(1);
    }
  
  memset(buffer_start, 0, bufferinfo.length);
 
  memset(&bufferinfo, 0, sizeof(bufferinfo));
  
  bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo.memory = V4L2_MEMORY_MMAP;
  bufferinfo.index = 0; /* Queueing buffer index 0. */
  
  // Attiva lo streaming
  type = bufferinfo.type;
  if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
    perror("VIDIOC_STREAMON");
    exit(1);
  }
  
  
  size_t i=0;
  
  
  // Put the buffer in the incoming queue.
  if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
    perror("VIDIOC_QBUF");
    exit(1);
  }
  
  // The buffer's waiting in the outgoing queue.
  if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
    perror("VIDIOC_QBUF");
    exit(1);
  }
    
  /** immagine quadrata originale */      
  quadra_matrice(mq, buffer_start, 480, 640,2);
  
   /** immagine scalata */      
  scala_matrice(mqs,mq, 2,480);
  
  /** immagine scalata */      
  scala_matrice(mq,mqs, 2,240);
  
  /** immagine scalata */      
  scala_matrice(mqs,mq, 2,120);
  
  /** immagine scalata */      
  scala_matrice(mq,mqs, 2,60);

  //soglia_matrice(mq,30,2,1);

  LE_2_BE(mq,30);

  print_object((unsigned short*)mq,30, 30,5,90,"");
      
  /* coordiante del massimo */
  int r,c;
  punto_matrice(mq,30,2,&r,&c);

  int jpgfile;
  
  if((jpgfile = open("/tmp/tmp.yuv", O_WRONLY | O_CREAT, 0660)) < 0){
    perror("open");
    exit(1);
  }
  
  write(jpgfile, mq, 30*30*2);
  close(jpgfile);
  
  /* ritorno all main */
  
  *(inp+1)=(double)(c-15)/15.;
  *(inp+2)=(double)(30-r-15)/15.;

  //free(mq);
  // free(mqs);
  
  return 0;
}


void print_object(unsigned short x[],int r, int c,int R,int C,char *str)
{
  unsigned short min,max;
  min=65535;
  max=0;
  int im=-1,jm=-1;
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	if(x[i*c+j]>max) max=x[i*c+j];
	if(x[i*c+j]<min)
          {
            min=x[i*c+j];
            im=i;jm=j;
            
          }
      }
  
  double i_range=max-min;
  
  if(i_range==0)
    {
      i_range=1;
    }
  double max_c=255;
  double min_c=232;//232;
  double c_range=max_c-min_c;
  double conv=c_range/i_range;

  printf("\x1b[%d;%dH%s  ",R-2,C,str);
  for(int i=0;i<r;i++)
    for(int j=0;j<c;j++)
      {
	double gl;
	gl=x[i*c+j];
	int col=(int)(min_c+(double)(gl-min)*conv);
	printf("\x1b[%d;%dH\x1b[48;5;%dm  \x1b[0m",i+R,2*j+C,col);
      }
  fflush(stdout);
}
