#define GLFW_INCLUDE_GLU
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
 //#include <GL/glew.h>

#include <iostream>
#include <iomanip>
#include <opencv2/core.hpp>    // include OpenCV core headers
#include <opencv2/imgproc.hpp> // include image processing headers
#include <opencv2/highgui.hpp> // include GUI-related headers

#include "MarkerTracker.h"
#include "Ball.hpp"
#include "Player.hpp"
#include "DrawPrimitives.h"
#include "Object.hpp"

bool debugmode = true;
bool balldebug = false;

float resultTransposedMatrix_player1[16];
float resultTransposedMatrix_player2[16];
float resultTransposedMatrix_world[16];
float snowmanLookVector[4];
int towards = 0x005A;
int towardsList[2] = {0x005a, 0x0272};
int towardscounter = 0;

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

cv::Mat arrayToMat(float* matrix){
    return cv::Mat(4, 4, CV_32FC1, matrix);
}

void MatToArray(cv::Mat1f matrix, float* mat_array){
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            mat_array[i*4 + j] = matrix(i,j);
        }
    }
}

cv::Point3f getPosInWorld(float* resultTransposedMatrix_object, float* resultTransposedMatrix_world){
    cv::Mat Mat_object = arrayToMat(resultTransposedMatrix_object).t();
    cv::Mat Mat_world = arrayToMat(resultTransposedMatrix_world).t();
    
    cv::Mat Mat_world_inv = Mat_world.inv();
    cv::Mat object_pos_mat = Mat_world.inv() * Mat_object;
    
    cv::Point3f object_pos(object_pos_mat.at<float>(0,3), object_pos_mat.at<float>(1,3), object_pos_mat.at<float>(2,3));
    
    return object_pos;
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

void update(std::vector<Marker> &markers, std::vector<Object*>& objects, Player& player1, Player& player2){
    
    // update position
    for(int i=0; i<markers.size(); i++){
        const int code =markers[i].code;
        
        // fix scale(translate x, y)
        float scale = 0.4;
        markers[i].resultMatrix[3] *= scale;
        markers[i].resultMatrix[7] *= scale;
        
        if(code == code_player1) {
            
            // transpose
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_player1[x*4+y] = markers[i].resultMatrix[y*4+x];
            
            // change the coordinate to the world coordinate
            cv::Point3f pos = getPosInWorld(resultTransposedMatrix_player1, resultTransposedMatrix_world);
            
            player1.setPos(pos);
            std::cout << "main player1 pos:" << player1.pos << std::endl;
            
        }else if(code == code_player2){
            
            // transpose
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_player2[x*4+y] = markers[i].resultMatrix[y*4+x];
            
            // change the coordinate to the world coordinate
            cv::Point3f pos = getPosInWorld(resultTransposedMatrix_player2, resultTransposedMatrix_world);
            
            player2.setPos(pos);
            
        }else if(code == code_world){
            
            for (int x=0; x<4; ++x)
                for (int y=0; y<4; ++y)
                    resultTransposedMatrix_world[x*4+y] = markers[i].resultMatrix[y*4+x];
        }
    }
    
    // move obj according to the velocity
    for(Object* obj : objects){
        obj->move();
    }
}

void display(const cv::Mat &img_bgr, std::vector<Object*>& objects, Player player1, Player player2){

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
    
    // move to marker-position
    glMatrixMode( GL_MODELVIEW );
    
    // draw debug objects
    if(debugmode){
        // draw world axis
        glLoadIdentity();
        glLoadMatrixf( resultTransposedMatrix_world );
        // x axis
        glColor4f(1, 0, 0, 1);
        drawLine(-10, 0, 0, 10, 0, 0);
        // y axis
        glColor4f(0, 1, 0, 1);
        drawLine(0, -10, 0, 0, 10, 0);
        // z axis
        glColor4f( 0, 0, 1, 1);
        drawLine(0, 0, -10, 0, 0, 10);
    }
    
    // draw player
    player1.draw(resultTransposedMatrix_world);
    player2.draw(resultTransposedMatrix_world);


    //shootBall(1);
    
    // draw ball
    for(Object* obj : objects){
        obj->draw(resultTransposedMatrix_world);
    }

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
    
    // set object
    std::vector<Object*> objects;
    Player player1(cv::Point3f(10,10,10), cv::Vec3f(0,0,0));
    Player player2(cv::Point3f(10,10,10), cv::Vec3f(0,0,0));
    
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
        
        /* Update objects position */
        update(markers, objects, player1, player2);
        
        /* Render here */
        display(img_bgr, objects, player1, player2);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    
    return 0;
    
}


