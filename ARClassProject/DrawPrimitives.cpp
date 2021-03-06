//
//  DrawPrimitives.cpp
//  ARClassProject
//
//  Created by 森敦史 on 2019/06/02.
//  Copyright © 2019 森敦史. All rights reserved.
//

#include "DrawPrimitives.h"

#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
#include <math.h>


void drawSphere(double r, int lats, int longs) {
    int i, j;
    for(i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = r * sin(lat0);
        double zr0 = r *  cos(lat0);
        
        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1  = r * sin(lat1);
        double zr1 = r * cos(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);
            
            glNormal3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
            glVertex3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
            glNormal3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
            glVertex3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
        }
        glEnd();
    }
}

void drawCube(double width, double height, double length) {
    // キューブの頂点情報。
    GLdouble aCubeVertex[][3] = {
        { -width/2, -height/2, -length/2 },
        { width/2, -height/2, -length/2 },
        { width/2, height/2, -length/2 },
        { -width/2, height/2, -length/2 },
        { -width/2, -height/2, length/2 },
        { width/2, -height/2, length/2 },
        { width/2, height/2, length/2 },
        { -width/2, height/2, length/2 }
    };
    // キューブの面。
    int aCubeFace[][4] = {
        { 0, 1, 2, 3 },
        { 1, 5, 6, 2 },
        { 5, 4, 7, 6 },
        { 4, 0, 3, 7 },
        { 4, 5, 1, 0 },
        { 3, 2, 6, 7 }
    };
    // キューブに対する法線ベクトル。
    GLdouble aCubeNormal[][3] = {
        { 0.0, 0.0,-1.0 },
        { 1.0, 0.0, 0.0 },
        { 0.0, 0.0, 1.0 },
        {-1.0, 0.0, 0.0 },
        { 0.0,-1.0, 0.0 },
        { 0.0, 1.0, 0.0 }
    };
    

    glBegin( GL_QUADS );
    for (size_t i = 0; i < 6; ++i)
    {
        glNormal3dv( aCubeNormal[i] );// 法線ベクトルをキューブに当てる。
        for (size_t j = 0; j < 4; ++j)
        {
            glVertex3dv( aCubeVertex[ aCubeFace[i][j] ] );
        }
    }
    glEnd();
}


void drawLine(double x1, double y1, double z1, double x2, double y2, double z2) {
    glBegin(GL_LINES);
    glVertex3d(x1, y1, z1);
    glVertex3d(x2, y2, z2);
    glEnd();
}


//void solidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
//{
//    glBegin(GL_LINE_LOOP);
//    GLUquadricObj* quadric = gluNewQuadric();
//    gluQuadricDrawStyle(quadric, GLU_FILL);
//
//    gluCylinder(quadric, base, 0, height, slices, stacks);
//
//    gluDeleteQuadric(quadric);
//    glEnd();
//}

void drawSnowman( bool female = 0)
{
    glRotatef( -90, 1, 0, 0 );
    glScalef(0.03f, 0.03f, 0.03f);
    
    // draw 3 white spheres
    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    drawSphere( 0.8, 10, 10 );
    glTranslatef( 0.0f, 0.8f, 0.0f );
    drawSphere( 0.6, 10, 10 );
    if(female)
    {
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        glTranslatef(-0.2f, 0.05f, 0.3f);
        drawSphere( 0.32, 10, 10 );
        glTranslatef(0.4f, 0, 0);
        drawSphere( 0.32, 10, 10 );
        glPopMatrix();
    }
    glTranslatef( 0.0f, 0.6f, 0.0f );
    drawSphere( 0.4, 10, 10 );
    
    // draw the eyes
    glPushMatrix();
    glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
    glTranslatef( 0.2f, 0.2f, 0.2f );
    drawSphere( 0.066, 10, 10 );
    glTranslatef( 0, 0, -0.4f );
    drawSphere( 0.066, 10, 10 );
    glPopMatrix();
    
    // draw a nose
    glColor4f( 1.0f, 0.5f, 0.0f, 1.0f );
    glTranslatef( 0.3f, 0.0f, 0.0f );
    glRotatef( 90, 0, 1, 0 );
    //solidCone( 0.1, 0.3, 10, 10 );
}

void drawRectangle(double w, double h) {
    glBegin(GL_POLYGON);
    glVertex3f(0, 0, 0.0);
    glVertex3f(w, 0.0, 0.0);
    glVertex3f(w, 0.0 - h, 0.0);
    glVertex3f(0.0, 0.0 - h, 0.0);
    glEnd();
    
}
