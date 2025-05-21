#include "motor.h"
#include "main.h"
#include <p18f4620.h>
#include <xc.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

float xPos = 0;
float yPos = 0;
float zPos = 1;

#define delayUp 200         //100 ms
#define delayDown 150
#define cycle 1500
#define CW 1
#define CCW 0
#define arcRes 4

int sameQuadrantAndInPath(float x, float y, float z, float cx, float cy, int direction);
int moveHalfCircle(float x, float y, float cx, float cy, int dir, int top, float r);

void moveToXYZ(float x, float y, float z)
{
    float xDelta = x - xPos;
    float yDelta = y - yPos;
    float unitToPulse = 300/25.4;       //300 is for inches to pulses
    
    
    //1700 Pulses max
    //  M1 M2
    //-Y 0 0
    //+Y 1 1
    //-X 0 1
    //+X 1 0
  //-X-Y 0
  //+X+Y 1
  //+X-Y   0
  //-X+Y   1
    if (z <= 0) {
        dropServo();
    } else if (z > 0)
    {
        liftServo();
    }
    zPos = z;
    
    
    printf("xDel %f yDel %f",xDelta,yDelta);
    float M1Distance = 0;
    float M2Distance = 0;
    
    if (fabs(xDelta)>fabs(yDelta))
    {
        printf("><\r\n");
        if (xDelta>=0)
        {
            M1Dir = 1;
            M2Dir = 0;
            M1Distance = (xDelta+yDelta)/2;
            M2Distance = xDelta-M1Distance;
        } else if (xDelta<0)
        {
            M1Dir = 0;
            M2Dir = 1;
            M1Distance = (xDelta+yDelta)/(-2);
            M2Distance = yDelta+M1Distance;
        }
    } else if (fabs(yDelta)>=fabs(xDelta))
    {
        printf("Hourglass\r\n");
        if (yDelta>=0)
        {
            M1Dir = 1;
            M2Dir = 1;
            M1Distance = (xDelta+yDelta)/2;
            M2Distance = yDelta-M1Distance;
        } else if (yDelta<0)
        {
            M1Dir = 0;
            M2Dir = 0;
            M1Distance = (xDelta+yDelta)/(-2);
            M2Distance = xDelta+M1Distance;
        }
    }
    
    M1Distance = fabs(M1Distance);
    M2Distance = fabs(M2Distance);
    float M1Pulse = unitToPulse*M1Distance;
    float M2Pulse = unitToPulse*M2Distance;
    float M1Left = M1Pulse;
    float M2Left = M2Pulse;
    
//    printf("M1Pulse: %f",M1Pulse);
//    printf("M2Pulse: %f\r\n",M2Pulse);
    if (M1Pulse>=M2Pulse)
    {
        while (M1Left > 0 || M2Left > 0)
        {
            for (int i = 0; i <= 500; i++) {}
            if (M2Left/M2Pulse >= M1Left/M1Pulse) {
//                printf("m2: %f",M2Left/M2Pulse);
//                printf("m1: %f\r\n",M1Left/M1Pulse);
                --M2Left;
                M2Step = 1;
            }
//            printf("reg\r\n");
            M1Step = 1;
            for (int i = 0; i <= 500; i++) {}
            M2Step = 0;
            M1Step = 0;
            --M1Left;
            
        }
    } else if (M2Pulse >= M1Pulse)
    {
        while (M1Left>0 || M2Left > 0)
        {
            for (int i = 0; i <= 200; i++) {}
            if (M1Left/M1Pulse >= M2Left/M2Pulse) {
//                printf("m2: %f",M2Left/M2Pulse);
//                printf("m1: %f\r\n",M1Left/M1Pulse);
                --M1Left;
                M1Step = 1;
            }
//            printf("reg\r\n");
            M2Step = 1;
            for (int i = 0; i <= 200; i++) {}
            M2Step = 0;
            M1Step = 0;
            --M2Left;
        }
    }
    xPos = x;
    yPos = y;
}


void moveArcCW(float x, float y, float z, float i, float j)
{
    float centerX = xPos + i;
    float centerY = yPos + j;
    float radius = sqrt(i*i+j*j);
    
    if (j>0 || (j==0&&i>0))
    {
        if (moveHalfCircle(x,y,centerX,centerY,CW,0,radius)) return;
        if (moveHalfCircle(x,y,centerX,centerY,CW,1,radius)) return;
        moveHalfCircle(x,y,centerX,centerY,CW,0,radius); 
        return;
    } else if (j < 0 || (j==0 && i < 0)) {
        if (moveHalfCircle(x,y,centerX,centerY,CW,1,radius)) return;
        if (moveHalfCircle(x,y,centerX,centerY,CW,0,radius)) return;
        moveHalfCircle(x,y,centerX,centerY,CW,1,radius);
        return;
    }
}

void moveArcCCW(float x, float y, float z, float i, float j)
{
    float centerX = xPos + i;
    float centerY = yPos + j;
    float radius = sqrt(i*i+j*j);
    
    if (j>0 || (j==0&&i>0))
    {
        if (moveHalfCircle(x,y,centerX,centerY,CCW,0,radius)) return;
        if (moveHalfCircle(x,y,centerX,centerY,CCW,1,radius)) return;
        moveHalfCircle(x,y,centerX,centerY,CCW,0,radius); 
        return;
    } else if (j < 0 || (j==0 && i < 0)) {
        if (moveHalfCircle(x,y,centerX,centerY,CCW,1,radius)) return;
        if (moveHalfCircle(x,y,centerX,centerY,CCW,0,radius)) return;
        moveHalfCircle(x,y,centerX,centerY,CCW,1,radius);
        return;
    }
}

int moveHalfCircle(float x, float y, float cx, float cy, int dir, int top, float r)
{
    int resnum = 4;
    if (r>= 20) resnum = 4;
    else if (r>=10) resnum = 3;
    else if (r>= 5) resnum = 2;
    else if (r>= 3) resnum = 2;
    else resnum = 1;
    float resolution = r/resnum;
    if (top == 1)
    {
        if (dir == CW)
        {
            while (1)
            {
                if (x-xPos<= resolution && y>= cy)
                {
                    
                    moveToXYZ(x,y,zPos);
                    return 1;
                }
                if (cx+r <= xPos + resolution)
                {
                    moveToXYZ(cx+r,cy,zPos);
                    return 0;
                }
                moveToXYZ(xPos+resolution,sqrt(r*r-((xPos+resolution-cx)*(xPos+resolution-cx)))+cy,zPos);
                
            }
        } else if (dir==CCW)
        {
            while (1)
            {
                if (xPos-x<= resolution && y>=cy)
                {
                    moveToXYZ(x,y,zPos);
                    return 1;
                }
                if (cx-r >= xPos - resolution)
                {
                    moveToXYZ(cx-r,cy,zPos);
                    return 0;
                }
                moveToXYZ(xPos-resolution,sqrt(r*r-((xPos-resolution-cx)*(xPos-resolution-cx)))+cy,zPos);
                
            }
            
        }
    } else if (top == 0)
    {
        if (dir == CW)
        {
            while (1)
            {
                if (xPos-x<= resolution && y <= cy)
                {
                    moveToXYZ(x,y,zPos);
                    return 1;
                }
                if (cx-r >= xPos - resolution)
                {
                    moveToXYZ(cx-r,cy,zPos);
                    return 0;
                }
                moveToXYZ(xPos-resolution,-1*sqrt(r*r-((xPos-resolution-cx)*(xPos-resolution-cx)))+cy,zPos);
                
            }
            
        } else if (dir==CCW)
        {
            while (1)
            {
                if (x-xPos<= resolution && (y <= cy))
                {
                    moveToXYZ(x,y,zPos);
                    return 1;
                }
                if (cx+r <= xPos + resolution)
                {
                    moveToXYZ(cx+r,cy,zPos);
                    return 0;
                }
                moveToXYZ(xPos+resolution,-1*sqrt(r*r-((xPos+resolution-cx)*(xPos+resolution-cx)))+cy,zPos);
                
            }
            
        }
    }
}

//void moveArcCW(float x, float y, float z, float i, float j)
//{
//    float centerX = xPos + i;
//    float centerY = yPos + j;
//    
//    float radius = sqrt(i*i+j*j);
//    
//    float resolution = arcRes;
//    int counter = 0;
//    while (sameQuadrantAndInPath(x,y,z,centerX,centerY,CW)==0)
//    {
//        counter++;
//        printf("arc CW\r\n");
//        int cxside = (xPos>centerX?1:0);
//        int cyside = (yPos>centerY?1:0);
//        
//        if (yPos == centerY) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//            cyside = (cxside==1?0:1);
//        }
//        if (xPos == centerX) {
//            cxside = cyside;
//        }
//        
//        float xDest;
//        float yDest;
//        
//        if (cxside == 1 && cyside == 1){
//            xDest = centerX + radius;
//            yDest = centerY;
//        } else if (cxside == 1 && cyside ==0)
//        {
//            xDest = centerX;
//            yDest = centerY-radius;
//        } else if (cxside == 0 && cyside == 1)
//        {
//            xDest = centerX;
//            yDest = centerY+radius;
//        } else {
//            xDest = centerX-radius;
//            yDest = centerY;
//        }
//        
////        printf("Before check\r\n");
//        if (cyside == 1)
//        {
//            float increment = (xDest-xPos)/arcRes;
//            for (float m = 0; m<arcRes;m++)
//            {
//                printf("inc: %f \r\n",increment);
//                moveToXYZ(xPos+increment,sqrt(radius*radius-((xPos+increment-centerX)*(xPos+increment-centerX)))+centerY,z);
//            }
//        } else {
//            float increment = (xPos-xDest)/arcRes;
//            for (float m = arcRes; m>0;m--)
//            {
//                printf("inc: %f \r\n",increment);
//                moveToXYZ(xPos-increment,(-1*sqrt(radius*radius-((xPos-increment-centerX)*(xPos-increment-centerX))))+centerY,z);
//            }
//        }
//        if (counter==5) break;
//    }
//    int cyside = (yPos>centerY?1:0);
//    int cxside = (xPos>centerX?1:0);
//    if (yPos == centerY) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//        cyside = (cxside==1?0:1);
//    }
//    if (cyside == 1)
//    {
//        float increment = (x-xPos)/arcRes;
//        for (float m = 0; m<arcRes;m++)
//        {
//            moveToXYZ(xPos+increment,sqrt(radius*radius-((xPos+increment-centerX)*(xPos+increment-centerX)))+centerY,z);
//        }
//    } else 
//        {
//        float increment = (xPos-x)/arcRes;
//        for (float m = arcRes; m>0;m--)
//        {
//            moveToXYZ(xPos-increment,(-1*sqrt(radius*radius-((xPos-increment-centerX)*(xPos-increment-centerX))))+centerY,z);
//        }
//        
//    }
//    printf("Last CW\r\n");
//    xPos = x;
//    yPos = y;
//    zPos = z;
//    
//}



//int sameQuadrantAndInPath(float x, float y, float z, float cx, float cy, int direction)
//{
////    printf("Calculating if same quad\r\n");
//    int dxside = (x>cx?1:0);        //which side of the center is the destination and current x position on, left or right
//    int cxside = (xPos>cx?1:0);
//    
//    int dyside = (y>cy?1:0);        //which side of the center is the dest vs current y position on, above or below
//    int cyside = (yPos>cy?1:0); 
//    
//    if (fabs(xPos-cx)<=0.01)xPos = cx;
//    if (fabs(yPos-cy)<=0.01)yPos = cy;
//    
//    if (direction == CW)
//    {
//        if (yPos == cy) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//            cyside = (cxside==1?0:1);
//        }
//        if (xPos == cx) {
//            cxside = cyside;
//        }
//    } else if (direction == CCW)
//    {
//        if (yPos == cy) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//            cyside = cxside;
//        }
//        if (xPos == cx) {
//            cxside = (cyside==1?0:1);
//        }
//    }
//    
//    if (dxside == cxside && dyside == cyside)
//    {
//        printf("almost\r\n");
//        if (direction == CW) {
//            printf("x: %f xp: %f",x,xPos);
//            if (cyside == 1 && xPos<= x) {return 1;}
//            else if (cyside == 0 && xPos>=x) {return 1;}
//        } else if (direction == CCW){
//            printf("x: %f xp: %f",x,xPos);
//            if (cyside == 0 && x<= xPos) {return 1;}
//            else if (cyside == 1 && xPos>=x) {return 1;}
//        }
//    } else {
//        return 0;
//    }
//    return 0;
//}

//void moveArcCCW(float x, float y, float z, float i, float j)
//{
//    
//    float centerX = xPos + i;
//    float centerY = yPos + j;
//    
//    float radius = sqrt(i*i+j*j);
//    
//    float resolution = arcRes;
//    
//    int counter = 0;
//    while (sameQuadrantAndInPath(x,y,z,centerX,centerY,CCW)==0)
//    {
//        counter++;
//        printf("arc CCW\r\n");
//        int cxside = (xPos>centerX?1:0);
//        int cyside = (yPos>centerY?1:0);
//        
//        if (yPos == centerY) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//            cyside = cxside;
//        }
//        if (xPos == centerX) {
//            cxside = (cyside==1?0:1);
//        }
//        
//        float xDest;
//        float yDest;
//        
//        if (cxside == 1 && cyside == 1){
//            xDest = centerX;
//            yDest = centerY+radius;
//        } else if (cxside == 1 && cyside ==0)
//        {
//            xDest = centerX+radius;
//            yDest = centerY;
//        } else if (cxside == 0 && cyside == 1)
//        {
//            xDest = centerX-radius;
//            yDest = centerY;
//        } else {
//            xDest = centerX;
//            yDest = centerY-radius;
//        }
//        
////        printf("Before check\r\n");
//        if (cyside == 1)
//        {
//            float increment = (xPos-xDest)/arcRes;
//            for (float m = arcRes; m>0;m--)
//            {
////                printf("Trying to move upper y\r\n");
//                moveToXYZ(xPos-increment,sqrt(radius*radius-((xPos-increment-centerX)*(xPos-increment-centerX)))+centerY,z);
//            }
//        } else {
//            float increment = (xDest-xPos)/arcRes;
//            for (float m = 0; m<arcRes;m++)
//            {
////                printf("Trying to move lower y\r\n");
//                moveToXYZ(xPos+increment,(-1*sqrt(radius*radius-((xPos+increment-centerX)*(xPos+increment-centerX))))+centerY,z);
//            }
//        }
//        if (counter==5) break;
//    }
//    int cyside = (yPos>centerY?1:0);
//    int cxside = (xPos>centerX?1:0);
//    if (yPos == centerY) {              //for the edge cases, we want the counter clockwise quadrant to be selected
//            cyside = cxside;
//    }
//    if (cyside == 1)
//        {
//            float increment = (xPos-x)/arcRes;
//            for (float m = arcRes; m>0;m--)
//            {
//                moveToXYZ(xPos-increment,sqrt(radius*radius-((xPos-increment-centerX)*(xPos-increment-centerX)))+centerY,z);
//            }
//        } else {
//            float increment = (x-xPos)/arcRes;
//            for (float m = 0; m<arcRes;m++)
//            {
//                moveToXYZ(xPos+increment,(-1*sqrt(radius*radius-((xPos+increment-centerX)*(xPos+increment-centerX))))+centerY,z);
//            }
//    }
//    printf("Last CCW\r\n");
//    xPos = x;
//    yPos = y;
//    zPos = z;
//    
//}

void liftServo()
{
    for (int j = 0; j < 20; j++){
    Servo = 1;
    for (int i = 0; i <= cycle; i++)
    {
        if (i == delayUp) Servo = 0;
        
    }
    }
    zPos = 2;
}

void dropServo()
{
    for (int j = 0; j < 20; j++) {
    Servo = 1;
    for (int i = 0; i <= cycle; i++)
    {
        if (i == delayDown) Servo = 0;
        
    }
    }
    zPos = -1;
}