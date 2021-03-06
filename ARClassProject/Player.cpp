//
//  Player.cpp
//  ARClassProject
//
//  Created by 森敦史 on 2019/06/01.
//  Copyright © 2019 森敦史. All rights reserved.
//

#include "Player.hpp"
#include "DrawPrimitives.h"

Player::Player(){
    life = 3;
    length = 0.01;
    height = 0.05;
    width = 0.01;
    
    color = cv::Vec3f(1.0,1.0,1.0);
    state = 0;
}

void Player::draw(float* resultTransposedMatrix_world){
    glLoadIdentity();
    glLoadMatrixf( resultTransposedMatrix_world );
    
    glColor4f(color[0], color[1], color[2], 1.0);
    drawCube(0.01, 0.05, 0.01);
    
    pos.x=resultTransposedMatrix_world[12];
    pos.y=resultTransposedMatrix_world[13];
    pos.z=resultTransposedMatrix_world[14];
    
    std::cout << color << std:: endl;
}
