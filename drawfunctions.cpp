#include "drawfunctions.h"
#include <cmath>
#include <vector>

//bu kodun iceriginden bahset
using namespace std;

#define PI 3.141592

void drawSphere(int subdiv, float drawingArray[]){
    float theta, phi; //angles
    float n1[3], n2[3]; //normalvector1 and normalvector2
    float p[3]; //p will be =  radius*(normalvector1 X normalvector2)
    int pointsPerRow = (int)pow(2, subdiv) + 1; //keeps number of points in a row(latitude or longtitude)
    vector<float> vertices;

    for(unsigned int face = 0; face <6; face++){
        for(unsigned int latitude = 0; latitude < pointsPerRow; latitude++){
            theta = (-45.0f + 90.0f * latitude / (pointsPerRow - 1)) * (PI / 180.0f);
            
            /*
            normal_*[][] arrays above keeps the coordinates of normal vectors of latitudinal and longitudinal planes.
            normal_1 array keeps for latitudinal plane and normal_2 keeps for longitudinal plane.

            The row of the arrays keeps the number that represents which face of the cube map.
             Number    Face
              0         +X
              1         -X
              2         +Y
              3         -Y
              4         +Z
              5         -Z

            The column of the arrays keeps the coordinates.
    */

            float normal1_x[6][2] = {0, (float)-sin(theta),
                                     1, (float)-sin(theta),
                                     2, (float)-cos(theta),
                                     3,  0,
                                     4,  0,
                                     5,  0          
                            };

            float normal1_y[6][2] = {0, (float) cos(theta),
                                     1, (float)-cos(theta),
                                     2, (float)-sin(theta),
                                     3, (float) sin(theta),
                                     4, (float) cos(theta),
                                     5, (float) cos(theta)          
                            };
            float normal1_z[6][2] = {0,  0,
                                     1,  0,
                                     2,  0,
                                     3, (float)-cos(theta),
                                     4, (float)-sin(theta),
                                     5, (float) sin(theta)          
                            };

            n1[0] = normal1_x[face][1];
            n1[1] = normal1_y[face][1];
            n1[2] = normal1_z[face][1];
            
            for(unsigned int longtitude = 0; longtitude < pointsPerRow; longtitude++){
                phi = (-45.0f + 90.0f * longtitude / (pointsPerRow - 1)) * (PI / 180.0f);
                
            float normal2_x[6][2] = {0, (float) sin(phi),
                                     1, (float) sin(phi),
                                     2, 0,
                                     3, (float) cos(phi),
                                     4, (float)-cos(phi),
                                     5, (float) cos(phi)          
                            };

            float normal2_y[6][2] = {0,  0,
                                     1,  0,
                                     2, (float) sin(phi),
                                     3, (float)-sin(phi),
                                     4,  0,
                                     5,  0          
                            };

            float normal2_z[6][2] = {0, (float) cos(phi),
                                     1, (float) cos(phi),
                                     2, (float)cos(phi),
                                     3, 0,
                                     4, (float) sin(phi),
                                     5, (float)-sin(phi)          
                            };
                
                n2[0] = normal2_x[face][1];
                n2[1] = normal2_y[face][1];
                n2[2] = normal2_z[face][1];

                //vector product of normals: p = n1 X n2
                p[0] = n1[1] * n2[2] - n1[2] * n2[1];
                p[1] = n1[2] * n2[0] - n1[0] * n2[2];
                p[2] = n1[0] * n2[1] - n1[1] * n2[0];

                // normalize p vector
                float scale = 1 / sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
                p[0] *= scale;
                p[1] *= scale;
                p[2] *= scale;

                // add a vertex into array
                vertices.push_back(p[0]);
                vertices.push_back(p[1]);
                vertices.push_back(p[2]); 
            }
        }
    }

    //we need to order vertices properly to draw the sphere with glDrawArray
    int pointsPerFace = pointsPerRow * pointsPerRow;
    int trianglePerFace = (pointsPerRow -1) * (pointsPerRow - 1) * 2;
    float vertexcoord[6][pointsPerFace][3]; //it stores all vertices: [6 faces][points per face(in each face)][3 coordinates(for each point)] 
    unsigned int arrayElement = trianglePerFace * 3 * 3 * 6; //triangle per face * 3 vertices per triangle * 3 coordinates per vertex * 6 faces 

    
    //  arrayElement = (number of triangles in a face) * (3 vertices in a triangle) * (3 coordinates for a vertex) * (6 faces)
    //cout<<"subdiv: "<<subdiv<<endl;
    //cout<<"pointsPerRow: "<<pointsPerRow<<endl;
    //cout<<"pointsPerFace: "<<pointsPerFace<<endl;
    //cout<<"arrayElement:"<<arrayElement<<endl;
    //cout<<"vertices:"<<vertices.size()<<endl;
    //float drawingArray[arrayElement * 2]; // multiplied by 2 because normal vector coordinates are added
    int f = 0; //face
    int v = 0; //vertex
    int c = 0; //coordinate
    
    for(int i = 0; i < vertices.size(); i++){
	    vertexcoord[f][v][c] = vertices[i];
   	    if(c == 2){ c = 0;
                if(v == (pointsPerFace - 1)){ v = 0;
                             if(f != 5) f++;
                           } else v++;   
                } else c++;

	}

    int drawindex = 0; //index for drawingArray array
    int index;     //index for counting vertices in a face 
    int number = trianglePerFace / 2;//(int)pow(2, pointsPerRow) / 2; //burasi hatali olabilir.

    float v1[3], v2[3], normalVector[3];
    //assign vertices data to drawingArray properly for drawing with GL_TRIANGLES
    for(unsigned int face = 0; face < 6; face++){
        index = 0;
        //left-lower triangles
        for(unsigned int i = 1; i <= number; i++){

            //normal vector calculation
            for(unsigned int j = 0; j < 3; j++){
                v1[j] = vertexcoord[face][index + 1][j] - vertexcoord[face][index][j] ;
                v2[j] = vertexcoord[face][index + pointsPerRow][j] - vertexcoord[face][index][j];          
            }

            normalVector[0] = v1[1] * v2[2] - v1[2] * v2[1];
            normalVector[1] = v1[2] * v2[0] - v1[0] * v2[2];
            normalVector[2] = v1[0] * v2[1] - v1[1] * v2[0];

            float scale = 1 / sqrt((normalVector[0] * normalVector[0]) + (normalVector[1] * normalVector[1]) + (normalVector[2] * normalVector[2]));

            normalVector[0]*= scale;
            normalVector[1]*= scale;
            normalVector[2]*= scale;
            
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index][j];
                drawindex++;
            }
            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }

            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index + 1][j];
                drawindex++;
            }

            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }

            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index + pointsPerRow][j];
                drawindex++;
            }

            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }
                   
            if((index + 2) % pointsPerRow == 0) index+= 2;
                else index++;
        }

        //right-upper triangles
        index = 0;
        for(unsigned int i = 1; i <= number; i++){
            //normal vector calculation
            for(unsigned int j = 0; j < 3; j++){
                v1[j] = vertexcoord[face][index + pointsPerRow][j] - vertexcoord[face][index + pointsPerRow+1][j];
                v2[j] = vertexcoord[face][index +1][j] - vertexcoord[face][index + pointsPerRow+ 1][j];
            }

            normalVector[0] = v1[1] * v2[2] - v1[2] * v2[1];
            normalVector[1] = v1[2] * v2[0] - v1[0] * v2[2];
            normalVector[2] = v1[0] * v2[1] - v1[1] * v2[0];

            float scale = 1 / sqrt((normalVector[0] * normalVector[0]) + (normalVector[1] * normalVector[1]) + (normalVector[2] * normalVector[2]));
            
            normalVector[0]*= scale;
            normalVector[1]*= scale;
            normalVector[2]*= scale;

            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index + pointsPerRow+ 1][j];
                drawindex++;
            }
            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }

            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index+ pointsPerRow][j];
                drawindex++;
            }
            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }

            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = vertexcoord[face][index + 1][j];
                drawindex++;
            }
            //assign normal vectors coordinates
            for(unsigned int j = 0; j < 3; j++){
                drawingArray[drawindex] = normalVector[j];
                drawindex++;
            }

            if((index + 2) % pointsPerRow == 0) index+= 2;
                else index++;
        }
    }
}


void drawCylinder(float drawCyl[]){

	float pointsUpper[10][3], pointsLower[10][3];
    int cylIndex = 0;
    float radius = 0.1f;
    float v_1[3], v_2[3], normal[3];

    for(int i = 0; i < 10; i++){
        pointsUpper[i][0] = (float)radius * sin(i * (2 * 3.1416) / 10);
        pointsUpper[i][1] = 6.0f;
        pointsUpper[i][2] = (float)radius * cos(i * (2 * 3.1416) / 10);

        pointsLower[i][0] = (float)radius * sin(i * (2 * 3.1416) / 10);
        pointsLower[i][1] = 1.0f;
        pointsLower[i][2] = (float)radius * cos(i * (2 * 3.1416) / 10);
    }

    for(int i = 0; i <10; i++){
        //normal calculation
         for(int j = 0; j < 3; j++){
            v_1[j] = pointsLower[i][j] - pointsUpper[i][j];
            if(i == 9) v_2[j] = pointsUpper[0][j] - pointsUpper[i][j];
            else       v_2[j] =pointsUpper[i + 1][j] - pointsUpper[i][j]; 
        }

            normal[0] = v_1[1] * v_2[2] - v_1[2] * v_2[1];
            normal[1] = v_1[2] * v_2[0] - v_1[0] * v_2[2];
            normal[2] = v_1[0] * v_2[1] - v_1[1] * v_2[0];

            float scale = 1 / sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

            normal[0]*= scale;
            normal[1]*= scale;
            normal[2]*= scale;

            

        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = pointsUpper[i][j];
            cylIndex++;
        }
        //normal coordinates
        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = normal[j];
            cylIndex++;
        }       

        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = pointsLower[i][j];
            cylIndex++;
        }
        //normal coordinates
        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = normal[j];
            cylIndex++;
        }

        for(int j = 0; j < 3; j++){
            if( i == 9){
            drawCyl[cylIndex] = pointsUpper[0][j];
            cylIndex++;
                    } else {
                        drawCyl[cylIndex] = pointsUpper[i + 1] [j];
                        cylIndex++;
                    }
    }
    for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = normal[j];
            cylIndex++;
        }

        //right-lower triangles

        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = pointsLower[i][j];
             cylIndex++;
        }

        for(int j = 0; j < 3; j++){
            drawCyl[cylIndex] = normal[j];
            cylIndex++;
        }

        if(i == 9){
            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = pointsUpper[0][j];
                cylIndex++;
        }
            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = normal[j];
                cylIndex++;
            }

            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = pointsLower[0][j];
                cylIndex++;
        }
            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = normal[j];
                cylIndex++;
            }
        } else {
                for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = pointsUpper[i + 1][j];
                cylIndex++;
        }
            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = normal[j];
                cylIndex++;
            }

            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = pointsLower[i + 1][j];
                cylIndex++;
        }
            for(int j = 0; j < 3; j++){
                drawCyl[cylIndex] = normal[j];
                cylIndex++;
            }            
        }

    }
    }


void drawGrid(float grid[]){
    int gridIndex = 0;
    for(float i = -20; i <= 20; i = i + 2){
        grid[gridIndex] = i;
        gridIndex++;
        grid[gridIndex] = -10.0f;
        gridIndex++;
        grid[gridIndex] = 20.0f;
        gridIndex++;

        grid[gridIndex] = i;
        gridIndex++;
        grid[gridIndex] = -10.0f;
        gridIndex++;
        grid[gridIndex] = -20.0f;
        gridIndex++;
//////////////////////

        grid[gridIndex] = 20.0f;
        gridIndex++;
        grid[gridIndex] = -10.0f;
        gridIndex++;
        grid[gridIndex] = i;
        gridIndex++;

        grid[gridIndex] = -20.0f;
        gridIndex++;
        grid[gridIndex] = -10.0f;
        gridIndex++;
        grid[gridIndex] = i;
        gridIndex++;

    }
 
}