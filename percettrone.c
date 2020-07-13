#include <stdlib.h>
#include <stdio.h>
#include <time.h>


/*Numero di neuroni di input*/
#define VROWS 2

/*Ampiezza degli assi x e y (da -X_Y_MAX/2 a + X_Y_MAX/2)
Si modifichi la dimensione della finestra del terminale
in conseguenza del valore assegnato a quuesta macro.
*/
#define X_Y_MAX 40

/* Calcola il valore di attivazione del percettrone in base all'input e ai pesi*/
double calculate_activation(double weight[],double input[]);

/* determina l'uscita alta o bassa (0 o 1) del percettrone*/
int classify_activation(double activation_value); 

/*Stampa a video il grafico dei punti della retta separante le classi*/
void print_graph(double x,double y, int cls, double w[],char label[]);

void attiva_camera(char * video_dev_name);
void vector_init(double res[VROWS],double val);
int leggi_camera(double inp[]);
void chiudi_camera();


void delay(int milliseconds){
  long pause;
  clock_t now,then;
  
  pause = milliseconds*(CLOCKS_PER_SEC/1000);
  now = then = clock();
  while( (now-then) < pause )
    now = clock();
}

int main(int argc,char *argv[])
{
  char *camera;
  
  if(argc>1)
    camera=argv[1];
  else
    camera="/dev/video0";
  
  double x[VROWS+1];/*input cognitrone*/
  int out;/*outpput cognitrone*/
  double w[VROWS+1];/*pesi cognitrone*/
  double r;/*velocità apprendimento*/
  double a;
  int cls;
  
  /*Inizializza il vettore pesi del percettrone*/
  vector_init(w,1);
  w[0]=0.2;
  w[1]=.5;
  w[2]=-2.5;
  vector_init(x,1);
  r=0.05;

   /*Cancella il riquadro*/
  
  
  attiva_camera(camera);
  leggi_camera(x);
  printf("\x1b[2J");  
  /*Addestra il percettrone*/
  //printf("Addestramento Percettrone\n**********************\n");
  for(int in=0;in<20;in++)
    {
      getchar();
      printf("\x1b[2J");  
      
      
      leggi_camera(x);

      //printf("\x1b[40;1HIndividuato massimo  x=%lf, y=%lf",x[1],x[2]);
      printf("\nInserisci la Classe (0,1) e premi invio");
      scanf("%d",&out);

      /** scala x */
      x[1]*=X_Y_MAX/2;
      x[2]*=X_Y_MAX/2;
       
      a=calculate_activation(w,x);

      cls=classify_activation(a);
      
      //printf("x=(%lf,%lf), out=%d, cls=%d\n",x[1],x[2],out,cls);
      
      /*Calcolo il nuovo vettore pesi w*/
      
      for(int i=0; i<VROWS+1; i++)
        {
          w[i]=w[i]+r*(out-cls)*x[i];
          printf("w_%d=%lf\t",i,w[i]);
        }
      print_graph( x[1],x[2], out,  w,"Apprendimento");
    }

  while(1)
    {
      
      printf("\x1b[2J");  
      leggi_camera(x);
      
      /** scala x */
      x[1]*=X_Y_MAX/2;
      x[2]*=X_Y_MAX/2;
      
      a=calculate_activation(w,x);
      
      cls=classify_activation(a);
      print_graph( x[1],x[2], cls,  w,"Test");
      if(cls==0)
        printf("\x1b[40;90HClassificazione: \x1b[38;5;9mCLASSE 0\x1b[0m");
      else
        printf("\x1b[40;90HClassificazione: \x1b[38;5;10mCLASSE 1 (1)\x1b[0m");
      fflush(stdout);
      getchar();
    }
  
  chiudi_camera();
  
}


double calculate_activation(double w[],double x[])
{
  int c;
  double a=0;
  c=VROWS;
  for(int j=0;j<c+1;j++)
    {
      a=a+w[j]*x[j];
    }
  return a;
}


int classify_activation(double a)
{
  if(a>0) return 1; else return 0;
}
  
void vector_init(double res[VROWS],double val){
  int c;
  c=VROWS;
  
    for(int j=0;j<c+1;j++)
      {
	res[j]=val;
      }
}

  

int read_test(FILE *fp,double inp[]){
  int r;
  r=fscanf(fp,"%lf,%lf",inp+1,inp+2);
  return r;
}

void print_graph(double x,double y, int cls, double w[],char label[])
{
  /*Mappa logica del grafico*/
  static char plot_r_c[X_Y_MAX+1][X_Y_MAX+1];
  int r,c;
 
  /*Stampa la retta e gli assi*/
  double m,q;
  if(w[2]==0) return;
  m=-w[1]/w[2];
  q=-w[0]/w[2];

  for(int i=-X_Y_MAX/2+1; i<X_Y_MAX/2;i+=1)
    {
      r=X_Y_MAX/2;
      c=X_Y_MAX/2+i;  
      printf("\x1b[%d;%dH%c",r,c,'-');
      c=X_Y_MAX/2;
      r=X_Y_MAX/2+i;
      printf("\x1b[%d;%dH%c",r,c,'|');

      c=X_Y_MAX/2+i;
      r=X_Y_MAX/2-((m*i)+q);
      if(c<=X_Y_MAX&&r<=X_Y_MAX&&r>0){
        if(plot_r_c[r][c]!='1'&&plot_r_c[r][c]!='0') plot_r_c[r][c]='/';
        printf("\x1b[%d;%dH%c",r,c,'/');
      }
    }
  /*Aggunge le frecce degli assi*/
        r=X_Y_MAX/2;
      c=X_Y_MAX;  
      printf("\x1b[%d;%dH%s",r,c,"->x");
      c=X_Y_MAX/2;
      r=1;
      printf("\x1b[%d;%dH%s",r,c-1,"y^");
      
  /*Ripristina i punti*/
  for(int i=1;i<X_Y_MAX+1;i++)
    for(int j=1;j<X_Y_MAX+1;j++)
      {
        if(plot_r_c[i][j]=='1')
          {
            printf("\x1b[%d;%dH\x1b[38;5;10m%c\x1b[0m",i,j,plot_r_c[i][j]);
          }
         if(plot_r_c[i][j]=='0')
          {
            printf("\x1b[%d;%dH\x1b[38;5;9m%c\x1b[0m",i,j,plot_r_c[i][j]);
          }
      }
  r=-y+X_Y_MAX/2;
  c=X_Y_MAX/2+x;
  if(cls==0)
  //Stampa cls=0
    {
      printf("\x1b[%d;%dH\x1b[38;5;9m%c\x1b[0m",r,c,'0');
      plot_r_c[r][c]='0';
    }
  else
  //Stampa cls=0
    {
      printf("\x1b[%d;%dH\x1b[38;5;10m%c\x1b[0m",r,c,'1');
      plot_r_c[r][c]='1';
    }
    
  char retta[30];
  sprintf(retta,"m=%0.2lf, q=%0.2lf",m,q);
  printf("\x1b[3;1H%s",retta);
  printf("\x1b[2;1H%s",label);
  getchar();
}

