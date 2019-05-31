
#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
 //#include <GL/glew.h>

#include <iostream>
#include <iomanip>
#include <opencv2/core.hpp>    // include OpenCV core headers
#include <opencv2/imgproc.hpp> // include image processing headers
#include <opencv2/highgui.hpp> // include GUI-related headers


#include "PoseEstimation.h"
#include "MarkerTracker.h"
#include "DrawPrimitives.h"
#include "Ball.hpp"


struct Position { double x,y,z; };

bool debugmode = false;
bool balldebug = false;

float resultTransposedMatrix_player1[16];
float resultTransposedMatrix_player2[16];
float resultTransposedMatrix_World[16];
float snowmanLookVector[4];
int towards = 0x005A;
int towardsList[2] = {0x005a, 0x0272};
int towardscounter = 0;

Ball ball(cv::Point3d(0,0,0));

int camera_width;
int camera_height;

const int code_player1 = 0x0b44;
const int code_player2 = 0x1228;
const int code_world = 0x1c44;

//const int virtual_camera_angle = 30;



void InitializeVideoStream( cv::VideoCapture &camera ) {
    if( camera.isOpened() )
        camera.release();
    
    camera.open(0); // open the default camera
    if ( camera.isOpened()==false ) {
        std::cout << "No webcam found, using a video file" << std::endl;
        camera.open("MarkerMovie.mpg");
        if ( camera.isOpened()==false ) {
            std::cout << "No video file found. Exiting."      << std::endl;
            exit(0);
        }
    }
}

void multMatrix(float mat[16], float vec[4])
{
    for(int i=0; i<4; i++)
    {
        snowmanLookVector[i] = 0;
        for(int j=0; j<4; j++)
            snowmanLookVector[i] += mat[4*i + j] * vec[j];
    }
}

/* program & OpenGL initialization */
void initGL(int argc, char *argv[])
{
    // initialize the GL library
    
    // pixel storage/packing stuff
    glPixelStorei(GL_PACK_ALIGNMENT, 1);// for glReadPixels
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for glTexImage2D
    glPixelZoom(1.0, -1.0);
    
    // enable and set colors
    glEnable( GL_COLOR_MATERIAL );
    glClearColor( 0, 0, 0, 1.0 );
    
    // enable and set depth parameters
    glEnable( GL_DEPTH_TEST );
    glClearDepth( 1.0 );
    
    // light parameters
    GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat light_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_dif[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    
    // enable lighting
    glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
    glLightfv( GL_LIGHT0, GL_AMBIENT,  light_amb );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_dif );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
}

void display(GLFWwindow* window, const cv::Mat &img_bgr, std::vector<Marker> &markers)
{
    //const auto camera_image_size = sizeof(unsigned char) *img_bgr.rows*img_bgr.cols * 3;
    
    int width0, height0;
    glfwGetFramebufferSize(window, &width0, &height0);
    
    //reshape(window, width, height);
    
    // clear buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // draw background image
    glDisable( GL_DEPTH_TEST );
    
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, camera_width, 0.0, camera_height );
    
    glRasterPos2i( 0, camera_height-1 );
    
    glDrawPixels( camera_width, camera_height, GL_RGB, GL_UNSIGNED_BYTE, img_bgr.data );
    
    glPopMatrix();
    
    glEnable(GL_DEPTH_TEST);
    
    //    return;
    
    // move to marker-position
    glMatrixMode( GL_MODELVIEW );
    
    // draw something at marker position
    float resultMatrix_0d22[16]; // Player1
    float resultMatrix_1068[16]; // Player2
    float resultMatrix_10e2[16]; // Stage(World Coordinate)
    for(int i=0; i<markers.size(); i++){
        const int code =markers[i].code;
        
        // fix scale(translate x, y)
        float scale = 0.4;
        markers[i].resultMatrix[3] *= scale;
        markers[i].resultMatrix[7] *= scale;
        
        if(code == code_player1) {
            for(int j=0; j<16; j++)
                resultMatrix_0d22[j] = markers[i].resultMatrix[j];
            
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_player1[x*4+y] = resultMatrix_0d22[y*4+x];
        
        }else if(code == code_player2){
            for(int j=0; j<16; j++)
                resultMatrix_1068[j] = markers[i].resultMatrix[j];
            
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_player2[x*4+y] = resultMatrix_1068[y*4+x];
            
        }else if(code == code_world){
            for(int j=0; j<16; j++)
                resultMatrix_10e2[j] = markers[i].resultMatrix[j];
            
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_World[x*4+y] = resultMatrix_10e2[y*4+x];
        }
        
    }
    
    // draw player1
    // Fixed tranlate scale
    glLoadMatrixf( resultTransposedMatrix_player1 );
    drawCube(0.01, 0.05, 0.01);
    
    // draw player2
    glLoadMatrixf( resultTransposedMatrix_player2 );
    drawCube(0.01, 0.01, 0.01);
    
    // World Coordinate
    // draw ball
//    Position player1_camera(resultTransposedMatrix_player1[
    
    ball.move();
    ball.debug();
    glLoadIdentity();
    glLoadMatrixf( resultTransposedMatrix_World );
    glTranslatef((float) ball.pos.x, (float) ball.pos.y + 0.024f, (float) ball.pos.z);
    glColor4f(1,0,0,1);
    drawSphere(0.005, 10, 10);
    
    int key = cv::waitKey (10);
    if (key == 27) exit(0);
    else if (key == 100) debugmode = !debugmode;
    else if (key == 98) balldebug = !balldebug;
    
}

void reshape( GLFWwindow* window, int width, int height ) {
    
    // set a whole-window viewport
    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
    
    // create a perspective projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 30, ((GLfloat)width/(GLfloat)height), 0.01, 100 );
    
    // invalidate display
    //glutPostRedisplay();
}

bool checkMarker(std::vector<Marker> &markers, int check_code){
    for(int i=0; i<markers.size(); i++){
        const int code =markers[i].code;
        if(code == check_code) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    
    cv::VideoCapture cap;
    GLFWwindow* window;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Initialize Camera Size */
    cv::Mat img_bgr;
    InitializeVideoStream(cap);
    while(true){
        cap >> img_bgr;
        if(img_bgr.empty()){
            std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
            InitializeVideoStream(cap);
            cv::waitKey(1000); /// Wait for one sec.
            continue;
        }else{
            break;
        }
    }
    camera_width = img_bgr.cols;
    camera_height = img_bgr.rows;
    
    
    // initialize the window system
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(camera_width/2, camera_height/2, "Game Window", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    // Set callback functions for GLFW
    glfwSetFramebufferSizeCallback(window, reshape);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval( 1 );
    
    int window_width, window_height;
    
    glfwGetFramebufferSize(window, &window_width, &window_height);  //window_width:1696 window_height:960
    
    reshape(window, window_width, window_height);
    
    glViewport(0, 0, window_width, window_height);
    
    // initialize the GL library
    initGL(argc, argv);
    
    // setup OpenCV
    const double kMarkerSize = 0.1;// 0.03; // [m]
    MarkerTracker markerTracker(kMarkerSize);
    std::vector<Marker> markers;
    
    // set ball
    ball = Ball(cv::Point3d(0,0,0));
    
    //    float resultMatrix[16];
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        markers.resize(0);
        /* Capture here */
        cap >> img_bgr;
        
        if(img_bgr.empty()){
            std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
            InitializeVideoStream(cap);
            cv::waitKey(1000); /// Wait for one sec.
            continue;
        }
        
        /* Track a marker */
        markerTracker.findMarker( img_bgr, markers);///resultMatrix);
        
        /* debug marker code */
        for (int i=0; i<markers.size(); i++) {
            std::cout << std::hex << markers[i].code << std::endl;
        }
        std::cout << std::endl;
        
//        std::cout << img_bgr.size() << std::endl;
//        cv::imshow("img_bgr", img_bgr);
//        cv::waitKey(10); /// Wait for one sec.
        
        
        /* Render here */
        display(window, img_bgr, markers);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    
    return 0;
    
}


