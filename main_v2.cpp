#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include "sphere.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI 3.141592   //Holy Pi!
#define h 0.025       //step length for Runge-Kutta
#define k 0.67        //driving force frequency (radian)
#define THETA_0 0     //initial angle(radian)
#define omega_0 0     //initial angular velocity

#define A 1.4         //driving force amplitude
#define b 0.4         //damping constant
#define m 1           //mass of pendulum
#define R 1           //length of rod
#define grav 1        //gravitational constant

//functions of the differential equation
float f(float time, float tht,float omega){
        return omega;
}
float g(float time, float tht, float omega){
        return -(grav/R)*sin(tht)-((b/(m*R*R))*omega)+((A/(m*R*R))*cos(k*time));
}


using namespace std;
GLFWwindow* window;

//window size
const unsigned int SCR_WIDTH = 1366; // screen width
const unsigned int SCR_HEIGHT = 768; // screen height

//timing
    float deltaTime = 0.0f; //time between current frame and last frame
    float lastFrame = 0.0f;

//for mouse moves
    bool firstMouse = true;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 400.0f;
    float lastY = 300.0f;

//for mouse scroll    
    float fov = 45.0f;

//camera
    glm::vec3 cameraPos = glm::vec3(4.0f, 0.0f, 16.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    void processInput(GLFWwindow *window);
    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

//light position (necessary for lighting) 
    glm::vec3 lightPos = glm::vec3(3.0f, 2.0f, 0.0f);


/******************main function***************************/

int main( void )
{
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Sphere", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
   
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glEnable(GL_DEPTH_TEST);

    Shader sphereShader("vertex.vs", "fragment.fs");
    Shader lightShader("lightsource.vs", "lightsource.fs");
    Shader gridShader("grid.vs", "grid.fs");
    Shader planeShader("plane.vs", "plane.fs");
    Shader myLightShader("myvertex.vs", "fragment.fs");
    unsigned int sphereProgram = sphereShader.programID();
    unsigned int lightProgram = lightShader.programID();
    unsigned int gridProgram = gridShader.programID();
    unsigned int planeProgram = planeShader.programID();
    unsigned int myLightProgram = myLightShader.programID();

    int subdivision = 6; //number of subdivision for sphere
    int pointsPerRow = (int)pow(2, subdivision) + 1; //keeps number of points in a row(latitude or longtitude)
    int pointsPerFace = pointsPerRow * pointsPerRow;
    int trianglePerFace = (pointsPerRow -1) * (pointsPerRow - 1) * 2; 
    float vertexcoord[6][pointsPerFace][3]; //it stores all vertices: [6 faces][points per face(in each face)][3 coordinates(for each point)] 
    unsigned int arrayElement = trianglePerFace * 3 * 3 * 6;  //triangle per face * 3 vertices per triangle * 3 coordinates per vertex * 6 faces 
    float drawingvertices[arrayElement * 2]; //multiplied by 2, because it also keeps normal coordinates for each vertex coordinate.

    drawSphere(subdivision, drawingvertices);

    float cylinder[360];
    drawCylinder(cylinder);

    float gridArray[612];
    drawGrid(gridArray);
    
    //sphere
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drawingvertices), drawingvertices, GL_STATIC_DRAW);
    
    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
   

    //light source   
    unsigned int lightSourceVAO;
    glGenVertexArrays(1, &lightSourceVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(lightSourceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

     
    //cylinder
    unsigned int cylinderVAO;
    glGenVertexArrays(1, &cylinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cylinder), cylinder, GL_STATIC_DRAW);
    glBindVertexArray(cylinderVAO);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    //grid-------grid has separate VBO. it also has separate vertex and fragment shaders.
    unsigned int gridVBO,gridVAO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridArray), gridArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //coordinate plane
    float coordinatePlane[] = {
                8.0f,  5.0f, 0.0f,   0.0f, 1.0f,
                14.0f, 5.0f, 0.0f,   1.0f, 1.0f,
                8.0f,  -1.0f, 0.0f,   0.0f, 0.0f,
                8.0f,  -1.0f, 0.0f,   0.0f, 0.0f,
                14.0f, -1.0f, 0.0f,   1.0f, 0.0f,
                14.0f, 5.0f, 0.0f,   1.0f, 1.0f
                };


    unsigned int coordinateVBO,coordinateVAO;
    glGenVertexArrays(1, &coordinateVAO);
    glGenBuffers(1, &coordinateVBO);
    glBindVertexArray(coordinateVAO);
    glBindBuffer(GL_ARRAY_BUFFER, coordinateVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinatePlane), coordinatePlane, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);  
    unsigned char *data = stbi_load("coordinateplane.png", &width, &height, &nrChannels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else std::cout<<"NO IMAGE"<<std::endl;
    stbi_image_free(data);
    
    float theta= THETA_0;
    float omg= omega_0;
    float time = 0;

    float k1,k2,k3,k4,l1,l2,l3,l4;
    float driving_force;
    
    //Initialize f and g functions.
    f(time,theta,omg);
    g(time,theta,omg);
    float current_angle;

    do{
        

        driving_force = A * cos(k * time);
        current_angle = theta * 180 / PI; //converts theta(radian) to degree


        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float times =  1 * glfwGetTime();
        glm::vec3 orbit(5 * sin(times), 1.0f,  5 * cos(times));

        processInput(window);
        glClearColor(0.6f, 0.6f, 0.6f, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(sphereProgram);       

        
        glUniform3f(glGetUniformLocation(sphereProgram, "objectColor"), 0.0f, 1.0f, 0.0f);
        glUniform3f(glGetUniformLocation(sphereProgram, "lightColor"), 1.0f, 1.0f, 1.0f);           
        glUniform3fv(glGetUniformLocation(myLightProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glm::mat4 projection2 = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model2 = glm::mat4(1.0f);
        glm::mat4 view2 = glm::mat4(1.0f); 
        view2 = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        //view2 = glm::rotate(view2, glm::radians(current_angle), glm::vec3(0.0f, 0.0f, 1.0f));
        view2 = glm::translate(view2, lightPos);
        glUniformMatrix4fv(glGetUniformLocation(myLightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection2));
        glUniformMatrix4fv(glGetUniformLocation(myLightProgram, "view"),  1, GL_FALSE, glm::value_ptr(view2));
        glUniformMatrix4fv(glGetUniformLocation(myLightProgram, "model"),  1, GL_FALSE, glm::value_ptr(model2));
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        
        glm::mat4 model = glm::mat4(1.0f);
            
        glm::mat4 view = glm::mat4(1.0f);        
          

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        view = glm::rotate(view, glm::radians(current_angle), glm::vec3(0.0f, 0.0f, 1.0f));
        view = glm::translate(view, glm::vec3(0.0f, -6.0f, 0.0f));
      
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "model"),  1, GL_FALSE, glm::value_ptr(model));

         

        k1= h*f(time,theta,omg);
        l1= h*g(time,theta,omg);
        k2= h*f(time+(0.5*h),theta+(0.5*k1),omg+(0.5*l1));
        l2= h*g(time+(0.5*h),theta+(0.5*k1),omg+(0.5*l1));
        k3= h*f(time+(0.5*h),theta+(0.5*k2),omg+(0.5*l2));
        l3= h*g(time+(0.5*h),theta+(0.5*k2),omg+(0.5*l2));
        k4= h*f(time+h,theta+k3,omg+l3);
        l4= h*g(time+h,theta+k3,omg+l3);

        theta = theta+(k1 + (2*k2) + (2*k3) + k4)/6;
        omg = omg+(l1 + (2*l2) + (2*l3) + l4)/6;
        //Below two lines keep the theta in range of -2PI to 2PI.
        if(theta>2*PI) theta= theta-(2*PI);
        if(theta<-2*PI) theta= theta+(2*PI);
        
        time =time+h;

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, arrayElement);

        glBindVertexArray(cylinderVAO);
        glDrawArrays(GL_TRIANGLES, 0, 360);

  /*  
        glUseProgram(lightProgram);
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f));
        
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"),  1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(lightSourceVAO);
        glDrawArrays(GL_TRIANGLES, 0, arrayElement);                
*/
        //drawing grid
        glUseProgram(gridProgram);

        glm::mat4 projectionGrid = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 viewGrid = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 modelGrid = glm::mat4(1.0f);
        //glm::mat4 projection2 = glm::mat4(1.0f);
        //glm::mat4 view2 = glm::mat4(1.0f);
        viewGrid = glm::translate(viewGrid, glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(gridProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionGrid));
        glUniformMatrix4fv(glGetUniformLocation(gridProgram, "view"),  1, GL_FALSE, glm::value_ptr(viewGrid));
        glUniformMatrix4fv(glGetUniformLocation(gridProgram, "model"),  1, GL_FALSE, glm::value_ptr(modelGrid));

        glBindVertexArray(gridVAO);
        glDrawArrays(GL_LINES, 0, 612);

        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(planeProgram);

        glm::mat4 projectionPlane = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 viewPlane = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 modelPlane = glm::mat4(1.0f);
        //glm::mat4 projection2 = glm::mat4(1.0f);
        //glm::mat4 view2 = glm::mat4(1.0f);
        viewPlane = glm::translate(viewPlane, glm::vec3(0.0f, -6.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(planeProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionPlane));
        glUniformMatrix4fv(glGetUniformLocation(planeProgram, "view"),  1, GL_FALSE, glm::value_ptr(viewPlane));
        glUniformMatrix4fv(glGetUniformLocation(planeProgram, "model"),  1, GL_FALSE, glm::value_ptr(modelPlane));

        glBindVertexArray(coordinateVAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 8.5 * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if(glfwGetKey(window , GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos += cameraUp * cameraSpeed;

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos -= cameraUp * cameraSpeed;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front); 
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    fov -= (float)yoffset;
    if(fov < 1.0f) fov = 1.0f;
    if(fov > 45.0f) fov = 45.0f;
}

