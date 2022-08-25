#pragma once

#define MIN_MOVEMENT_TIME 2.5
#define MAX_MOVEMENT_TIME 10.5

#define MOVEMENT_POWER 8.0f

#define MIN_SHOOT_TIME 2.5
#define MAX_SHOOT_TIME 15.0

#define NUMBER_OF_STAGES 5


#include <random>
#include "utils/gameObject.h"
#include "utils/camera.h"

class enemiesAI : public GameObject
{

private:
    float next_shoot, next_movement;
    float movement_reaction_time, shooting_reaction_time;
    bool shoot;
    int stage;
    Camera *camera;
    void Move(){
            ///////
    rb->setActivationState(DISABLE_DEACTIVATION);
    btVector3 linearVelocity;
    glm::vec3 direction;
    // initial velocity of the bullet
    // matrix for the inverse matrix of view and projection
    glm::mat4 unproject;
    // we create a Rigid Body with mass = 1
    linearVelocity=this->rb->getCenterOfMassPosition();
    direction=camera->Position()-glm::vec3(linearVelocity.getX(),linearVelocity.getY(),linearVelocity.getZ());
    direction.y=0;
    direction = glm::normalize(direction) * MOVEMENT_POWER;

    // we apply the impulse and shoot the bullet in the scene
    // N.B.) the graphical aspect of the bullet is treated in the rendering loop
    linearVelocity = btVector3(direction.x, direction.y, direction.z);
    this->rb->setLinearVelocity(linearVelocity);
    cout<<"moving"<<endl;
    }
public:
    const float level_modifiers[NUMBER_OF_STAGES]={1,0.8,0.6,0.4,0.2};
    enemiesAI(glm::vec3 pos, glm::vec3 s, glm::vec3 r, Model* m, Camera *player):GameObject(pos,s,r,m),stage(4),next_movement(0),next_shoot(0), camera(player){
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> shoot_reaction_generator(MIN_SHOOT_TIME, MAX_SHOOT_TIME);
        std::uniform_real_distribution<float> movement_reaction_generator(MIN_MOVEMENT_TIME, MAX_MOVEMENT_TIME);
        movement_reaction_time= movement_reaction_generator(mt);
        shooting_reaction_time= shoot_reaction_generator(mt);
        cout<<"generated enemy, movement time: "<<movement_reaction_time<<" shooting time: "<<shooting_reaction_time<<endl;
    }
    bool getShootAndReset(){
        bool s= shoot;
        shoot=false;
        return s;
    }
    
    void Update(float current_time){
        if(current_time>next_shoot){
            shoot=true;
            next_shoot=current_time+(shooting_reaction_time*level_modifiers[stage]);
        }
        if(current_time>next_movement){
            Move();
            next_movement= current_time+(movement_reaction_time*level_modifiers[stage]);
        }
    }

    void SetStage(int s){
        if(s>=0&& s<NUMBER_OF_STAGES){
            stage=s;
        }
    }
    
};


