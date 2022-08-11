#pragma once

#include <glad/glad.h>

// we use GLM to create the view matrix and to manage camera transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bullet/btBulletDynamicsCommon.h>
#include <utils/model_v1.h>
#include <utils/shader_v1.h>

#define SHININESS 25.0
#define ALPHA 0.2
#define F0 0.9

class GameObject
{
public: 
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    btRigidBody* rb;
    Model* model;
    GLfloat color[3] = {1.0f,0.0f,0.0f};
    // shininess coefficients (passed from the application)
    float shininess;

    // uniforms for GGX model
    float alpha; // rugosity - 0 : smooth, 1: rough
    float f0; // fresnel reflectance at normal incidence

private:
    /* data */
public:
    GameObject(glm::vec3 pos, glm::vec3 s, glm::vec3 r, Model* m): position(pos),scale(s),rotation(r),model(m), shininess(SHININESS),alpha(alpha),f0(F0){
        rb=nullptr;
    }
    ~GameObject();
    void Draw(glm::mat4 view,Shader shader){
        GLfloat matrix[16];
        btTransform transform;
        GLint objDiffuseLocation = glGetUniformLocation(shader.Program, "diffuseColor");
         // we determine the position in the Shader Program of the uniform variables
        GLint shineLocation = glGetUniformLocation(shader.Program, "shininess");
        GLint alphaLocation = glGetUniformLocation(shader.Program, "alpha");
        GLint f0Location = glGetUniformLocation(shader.Program, "F0");
        glUniform1f(shineLocation, shininess);
        glUniform1f(alphaLocation, alpha);
        glUniform1f(f0Location, F0);
        glUniform3fv(objDiffuseLocation, 1, color);
        // we take the transformation matrix of the rigid boby, as calculated by the physics engine
        
        // we reset to identity at each frame
        glm::mat4 objModelMatrix = glm::mat4(1.0f);
        glm::mat3 objNormalMatrix = glm::mat3(1.0f);
        if(rb!=nullptr){
            rb->getMotionState()->getWorldTransform(transform);
            // we convert the Bullet matrix (transform) to an array of floats
            transform.getOpenGLMatrix(matrix);
            // we create the GLM transformation matrix
            // 1) we convert the array of floats to a GLM mat4 (using make_mat4 method)
            // 2) Bullet matrix provides rotations and translations: it does not consider scale (usually the Collision Shape is generated using directly the scaled dimensions). If, like in our case, we have applied a scale to the original model, we need to multiply the scale to the rototranslation matrix created in 1). If we are working on an imported and not scaled model, we do not need to do this
            objModelMatrix = glm::make_mat4(matrix) * glm::scale(objModelMatrix, scale);
            // we create the normal matrix
            objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
        }
        else{
            objModelMatrix = glm::translate(objModelMatrix, position);
            objModelMatrix = glm::rotate(objModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            objModelMatrix = glm::scale(objModelMatrix, scale);
            // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
            objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
        }
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));

        // we render the model
        // N.B.) if the number of models is relatively low, this approach (we render the same mesh several time from the same buffers) can work. If we must render hundreds or more of copies of the same mesh,
        // there are more advanced techniques to manage Instanced Rendering (see https://learnopengl.com/#!Advanced-OpenGL/Instancing for examples).
        model->Draw();
    }

    void addRigidbody(Physics bulletSimulation, int type,float mass, float friction, float restitution){
        this->rb = bulletSimulation.createRigidBody(type,position,scale,rotation,mass,friction,restitution, model);
        this->rb->setUserPointer(this);
        this->rb->setUserIndex(0);
    }


    void setColor3(GLfloat c[]){
        color[0]=c[0];
        color[1]=c[1];
        color[2]=c[2];
    } 

};

GameObject::~GameObject()
{
}
