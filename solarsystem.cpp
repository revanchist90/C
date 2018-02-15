#include <GL\glut.h>
#include <stdlib.h>
#include <stdio.h>

 
GLfloat   radius[]={0, 5, 8 ,11,16,52,95,192,300 };
GLfloat  drotation[]={0,20, 8,5,2.6,0.45,0.2, 0.07,0.03 } ;
GLfloat   dspin[]={5,0,0,20,20,25,25, 20,30} ;

 
GLfloat  rotation[9];
Glfloat  spin[9];

GLfloat    r[]={4,0.04,0.1,0.1,0.06,0.7,0.68,0.4,0.38};

GLsizei width=600;
GLsizei height=600;

//float position[9];

GLfloat  ex,ey,ez;
GLfloat  tx,ty,tz;
GLfloat  vx,vy,vz; 


GLUquadric *sun;
GLUquadric *mercury;
GLUquadric *venus;
GLUquadric *earth;
GLUquadric *mars;
GLUquadric *jupiter;
GLUquadric *saturn;
GLUquadric *uranus;
GLUquadric *neptune;

/************************************/


 void  keyboard(unsigned int key,int x,int y)
 {   if (key=='w'){
           ey++; ty++;     }
       if (key=='a'){
           ex--; tx-- ;        }
        if (key=='s'){
           ey--; ty--;          }
         if (key=='d'){
           ex++; tx++;           }
           if (key=='q'){
             ez--; tz--;           }
           if (key=='e'){
             ez++; tz++;           }
       glutPostRedisplay();
       }






 void reshape(int newidth,int newheight )
  { if (newwidth<newheight){
       width=newwidth;
       height=width;                 }
     else 
     {  height=newheight;
        width =height;
          }  
    glViewport(0,0,(GLsizei)width,(GLsizei)height);    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
     gluPerspective (45.0, 1, 1.0,100.0);  
      }

//////////////////////////////////////////////
void init()
{ sun= gluNewQuadric();
  mercury= gluNewQuadric(); 
   venus=gluNewQuadric() ;
  earth= gluNewQuadric();
  mars= gluNewQuadric();
  jupiter= gluNewQuadric() ;
   saturn=gluNewQuadric() ;
   uranus=gluNewQuadric();
  neptune= gluNewQuadric();
   
  int i;
  for(i=0;i<9;i++)
      {  rotation[i]=0;
         spin[i]=0;
                  }
  
     } 
void special(int key,int x, int y)
{ //printf("Special key event detected.\n");
 
if (key==GLUT_KEY_UP) 
{ty++;//  if (ty==ey){ty++;}
  } 
  if (key==GLUT_KEY_DOWN)
{  ty--;// if (ty==ey){ty--;}
   } 
   if (key==GLUT_KEY_LEFT)
   {tx--; //if (tx==ex){tx--;}
  }
if (key==GLUT_KEY_RIGHT) 
{   tx++; //if (tx==ey){tx++;}
  }
if (key==GLUT_KEY_PAGE_UP){
     tz++;                  }
if (key==GLUT_KEY_PAGE_DOWN){
     tz--;                 }
 if (key==GLUT_KEY_F1){tx=(-1)*tx;ty=(-1)*ty;tz=(-1)*tz;}   
  if ((tz==ez)&&(ty==ey)&&(tx==ex)){ty++;} 
    
glutPostRedisplay(); }



void display()
{
glClearColor(1,1,1,0);
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
       //sun 
       glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[0],0,0,1);  
       glTranslatef(radius[0],0,0);
       glRotatef(spin[0],0,0,1); 
       gluSphere(sun,r[0],10,10); 
      //mercury
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[1],0,0,1);  
       glTranslatef(radius[1],0,0);
       glRotatef(spin[1],0,0,1); 
      gluSphere(mercury,r[1],10,10);
      //venus 
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[2],0,0,-1);  
       glTranslatef(radius[2],0,0);
       glRotatef(spin[2],0,0,-1); 
      gluSphere(venus,r[2],10,10);
     
       //earth
       glLoadIdentity();
        gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[3],0,0,1);  
       glTranslatef(radius[3],0,0);
       glRotatef(spin[3],0,0,1); 
      gluSphere(earth,r[3],10,10);
      //mars
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[4],0,0,1);  
       glTranslatef(radius[4],0,0);
       glRotatef(spin[4],0,0,1); 
      gluSphere(mars,r[4],10,10) ;
       //jupiter
       glLoadIdentity();
        gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[5],0,0,1);  
       glTranslatef(radius[5],0,0);
       glRotatef(spin[5],0,0,1); 
      gluSphere(jupiter,r[5],10,10);
      //saturn
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
     glRotatef(rotation[6],0,0,1);  
       glTranslatef(radius[6],0,0);
       glRotatef(spin[6],0,0,1); 
      gluSphere(saturn,r[6],10,10);
      //uranus
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[7],0,0,-1);  
       glTranslatef(radius[7],0,0);
       glRotatef(spin[7],0,0,-1); 
      gluSphere(uranus,r[7],10,10);
      //neptune
      glLoadIdentity();
       gluLookAt(ex,ey,ez,tx,ty,tz,vx,vy,vz);
      glRotatef(rotation[8],0,0,1);  
       glTranslatef(radius[8],0,0);
       glRotatef(spin[8],0,0,1); 
      gluSphere(neptune,r[8],10,10);
      ///////////////////////
       glFlush();
}

void animate()
{   
    int i;
   for(i=0;i<9;i++)
   { rotation[i] += drotation;
      spin[i] += dspin;
      if  (rotation[i]>360){rotation[i]-=360;}
       if  (spin[i]>360){spin[i]-=360;}
                        }      
   glutPostRedisplay();                       
     }

int main(int argc, char** argv){
glutInit(&argc,argv);
glutInitWindowPosition(50,50);
glutInitWindowSize(width,height);
glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
glutCreateWindow("solar system simulation");
init();
glEnable(GL_DEPTH_TEST);
glEnable(GL_TEXTURE_2D);
glMatrixMode(GL_PROJECTION);
//gluOrtho2D(0,50,0,50);
glutDisplayFunc(display);
glutKeyboardFunc(keyboard);
glutSpecialFunc(special);

glutReshapeFunc(reshape);

glutIdleFunc(animate);

glutMainLoop();
return 0;}
