/*
 * This is a surface of revolution object
 */

//#include <string>
//#include <vector>
//#include <stack>
#include <math.h>
#include <stdio.h>
//#include <GL/freeglut.h>
//#include <GL/glut.h>
//#include "../glm/glm.hpp"
//#include "../glm/ext.hpp"
//#include <fstream>
//#include <sstream>
//#include <exception>
//#include <stdexcept>
//#include <string.h>
//#include <algorithm>
//#include <string>
//#include <vector>
void normalize(float v[3]);
void normCrossProd(float v1[3], float v2[3], float out[3]);

class SOR {

   // these statics are for OpenGL's purpose, so that one surfOfRev object can be used for multiple surfOfRevs
   public:static bool surfOfRevInitialized;
   public:static GLuint surfOfRevVao;
   public:static GLuint surfOfRevVertexBufferObject;
   public:static GLuint surfOfRevIndexBufferObject;

          typedef struct {
          } SurfOfRev;

#define radsPerDegree 3.14159/180.0

          int numPolygons;
          int profPts;
          float angle;
          int numSlices;
          int inc;
          float *xvals;
          float *yvals;
          GLfloat *vertexAtts;
          GLfloat *undeformedVertexAtts;
          GLshort *surfOfRevIndices;

          SOR(float *xvalsP, float *yvalsP, int profilePts, int incr ) {

             // initialize SOR
             profPts = profilePts;
             xvals = xvalsP;
             yvals = yvalsP;
             inc = incr;
             numSlices = inc;
             angle = 360/inc;
          }

          void GenerateSORTriangles() {
             // allocate array for vertices
             const int floatCoords = 3; // number of floats per triangle vertex

             // This function builds a surface of revolution about the Y-axis
             // Assumes a profile of connected (x, y) coordinates with z=0 and x>0.
             // inc is the incremental horizontal angle in degrees through which adjacent
             // facets will be rotated.
             int i, j, slice;
             float xprofLow, yprofLow;
             float d1[3], d2[3], normal[3];

             // initialize array of (profPts-1)*(slices) polygons
             vertexAtts = new GLfloat[(profPts)*(numSlices)*(3+3)]; // location and normal
             undeformedVertexAtts = new GLfloat[(profPts)*(numSlices)*(3+3)]; // location and normal

             int index;
             for (i=0, index = 0; i<profPts; i++) {
                yprofLow = yvals[i];
                xprofLow = xvals[i];

                for (slice=0; slice<numSlices; slice++, index++) {
                   vertexAtts[index*3+0] = xprofLow * cos(radsPerDegree * (slice*angle));
                   vertexAtts[index*3+1] = yprofLow;
                   vertexAtts[index*3+2] = xprofLow * sin(radsPerDegree * (slice*angle));
                   undeformedVertexAtts[index*3+0] = vertexAtts[index*3+0];
                   undeformedVertexAtts[index*3+1] = vertexAtts[index*3+1];
                   undeformedVertexAtts[index*3+2] = vertexAtts[index*3+2];
                }
             }

             // now compute the normal at each vertex
             int normalOffset = numSlices*(profPts) * (3); // offset into buffer where the normals start

             for (i=0, index = 0; i<profPts; i++) {
                for (slice=0; slice<numSlices; slice++, index+=3) {


                   if (i==profPts-1) {
                      // this is the last row of profile points
                      if (slice==numSlices-1) {
                         // check if last row - connect to first row if it is
                         for (j=0; j<3; j++) {
                            d1[j] = vertexAtts[index-((numSlices-1)*3)+j] - vertexAtts[index+j];
                            d2[j] = vertexAtts[index-(numSlices*3)+j] - vertexAtts[index+j];
                         }
                      }
                      else {
                         for (j=0; j<3; j++) {
                            d1[j] = vertexAtts[index+3+j] - vertexAtts[index+j];
                            d2[j] = vertexAtts[index-(numSlices*3)+j] - vertexAtts[index+j];
                         }
                      }
                   }
                   else if (slice==numSlices-1) {
                      // check if last row - connect to first row if it is
                      for (j=0; j<3; j++) {
                         d1[j] = vertexAtts[index-((numSlices-1)*3)+j] - vertexAtts[index+j];
                         d2[j] = vertexAtts[index+(numSlices*3)+j] - vertexAtts[index+j];
                      }
                   }
                   else {
                      // general case
                      for (j=0; j<3; j++) {
                         d1[j] = vertexAtts[index+3+j] - vertexAtts[index+j];
                         d2[j] = vertexAtts[index+(numSlices*3)+j] - vertexAtts[index+j];
                      }
                   }

                   // compute normalized cross product of edge vectors
                   normCrossProd(d2, d1, normal);

                   // this is the normal
                   vertexAtts[index+0 + normalOffset] = normal[0];
                   vertexAtts[index+1 + normalOffset] = normal[1];
                   vertexAtts[index+2 + normalOffset] = normal[2];
                   undeformedVertexAtts[index+0 + normalOffset] = vertexAtts[index+0 + normalOffset];
                   undeformedVertexAtts[index+1 + normalOffset] = vertexAtts[index+1 + normalOffset];
                   undeformedVertexAtts[index+2 + normalOffset] = vertexAtts[index+2 + normalOffset];
                }
             }

             // now the indices
             surfOfRevIndices = new GLshort[(profPts)*(numSlices+1)*(6)]; // two triangles per quad
             index = 0;
             for (i=0; i<profPts-1; i++) {
                int A, B, C, D;
                for (j=0; j<numSlices; j++) {
                   /*if (i==profPts-1) {
                   // this is the last row of profile points
                   if (j==numSlices-1) {
                   // check if last row - connect to first row if it is
                   }
                   else {
                   }
                   }
                   else */
                   if (j==numSlices-1) {
                      // at the end of the row
                      A = i*numSlices + j;
                      B = i*numSlices + j + numSlices;
                      C = i*numSlices + j + 1;
                      D = i*numSlices + j - (numSlices-1);
                   }
                   else {

                      // two triangles per quad - faster but more complex to use triangle strip
                      //ABCD quad, A lower-left corner going clockwise
                      A = i*numSlices + j;
                      B = i*numSlices + j + numSlices;
                      C = i*numSlices + j + 1 + numSlices;
                      D = i*numSlices + j + 1;
                   }
                   // first triangle
                   surfOfRevIndices[index++] = A;   surfOfRevIndices[index++] = C; surfOfRevIndices[index++] = B;
                   // second triangle
                   surfOfRevIndices[index++] = A; surfOfRevIndices[index++] = D; surfOfRevIndices[index++] = C;
                }
             }
          }

          void InitializeSORVertexBuffer()   {
             GenerateSORTriangles();
             glGenBuffers(1, &surfOfRevVertexBufferObject);

             glBindBuffer(GL_ARRAY_BUFFER, surfOfRevVertexBufferObject);
             glBufferData(GL_ARRAY_BUFFER, (numSlices*profPts)*(3*4*2), vertexAtts, GL_STATIC_DRAW); //3 floats per vert, 4 bytes per float, 2 sets of data (loc, normal)
             glBindBuffer(GL_ARRAY_BUFFER, 0);

             glGenBuffers(1, &surfOfRevIndexBufferObject);
             glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surfOfRevIndexBufferObject);
             glBufferData(GL_ELEMENT_ARRAY_BUFFER, (numSlices*(profPts-1))*3*2*2, surfOfRevIndices, GL_STATIC_DRAW); // 3*2 shorts per triangle, 2 bytes per short
             glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

             free(vertexAtts);
             free(surfOfRevIndices);
          }

          void InitializeSORBuffer() {
             InitializeSORVertexBuffer();

             glGenVertexArrays(1, &surfOfRevVao);
             glBindVertexArray(surfOfRevVao);

             // code for surfOfRev
             size_t normalDataOffset = sizeof(float) * (3) * (numSlices*profPts); // three floats per vertex
             glBindBuffer(GL_ARRAY_BUFFER, surfOfRevVertexBufferObject);
             glEnableVertexAttribArray(0);
             glEnableVertexAttribArray(1);
             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
             glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalDataOffset);
             glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surfOfRevIndexBufferObject);
          }

          void DrawUnitSOR()  {

             if (!surfOfRevInitialized) {
                // buffer array for vertices, normals, and indices is shared by all surfOfRevs
                InitializeSORBuffer();
                surfOfRevInitialized = true;
             }

             glBindVertexArray(surfOfRevVao);
             // number of indices - #triangles * verts
             glDrawElements(GL_TRIANGLES, (numSlices*(profPts-1)*3)*2, GL_UNSIGNED_SHORT, 0); // 3 verts per triangle
             glBindVertexArray(0);
          }

          void deformSORVertices(glm::vec3 unDeformedGrid[3][3], glm::vec3 deformedGrid[3][3]) {
             // This method uses the deformed grid to deform all the objects in the scene
             double S[3];
             double U[3];

             // find min and max of undeformed grid
             float xMin = unDeformedGrid[0][0].x;
             float xMax = unDeformedGrid[2][0].x;
             float yMin = unDeformedGrid[0][0].y;
             float yMax = unDeformedGrid[2][2].y;

             for (int i=0; i<profPts*numSlices*3; i+=3) {
                // deform the x coordinate
                float x = vertexAtts[i];
                float y = vertexAtts[i+1];
                // this s is the location parameter within the undeformed grid
                double s = (x - xMin) / (xMax - xMin);
                S[0] = (1-s)*(1-s);
                S[1] = 2*s*(1-s);
                S[2] = s*s;
                double s1 = (y - yMin) / (yMax - yMin);
                U[0] = (1-s1)*(1-s1);
                U[1] = 2*s1*(1-s1);
                U[2] = s1*s1;

                // just change the x coordinate

                for(int j =0; j < 3; j++) {
                   for(int k = 0; k<3; k++) {
                      vertexAtts[i] += S[j]*U[k]*deformedGrid[j][k].x;
                      vertexAtts[i+1] += S[j]*U[k]*deformedGrid[j][k].y;
                   }
                }

             }

             // now redo the normals
             int j;
             float d1[3], d2[3], normal[3];
             int normalOffset = numSlices*(profPts) * (3); // offset into buffer where the normals start

             for (int i=0, index = 0; i<profPts; i++) {
                for (int slice=0; slice<numSlices; slice++, index+=3) {

                   if (i==profPts-1) {
                      // this is the last row of profile points
                      if (slice==numSlices-1) {
                         // check if last row - connect to first row if it is
                         for (j=0; j<3; j++) {
                            d1[j] = vertexAtts[index-((numSlices-1)*3)+j] - vertexAtts[index+j];
                            d2[j] = vertexAtts[index-(numSlices*3)+j] - vertexAtts[index+j];
                         }
                      }
                      else {
                         for (j=0; j<3; j++) {
                            d1[j] = vertexAtts[index+3+j] - vertexAtts[index+j];
                            d2[j] = vertexAtts[index-(numSlices*3)+j] - vertexAtts[index+j];
                         }
                      }
                   }
                   else if (slice==numSlices-1) {
                      // check if last row - connect to first row if it is
                      for (j=0; j<3; j++) {
                         d1[j] = vertexAtts[index-((numSlices-1)*3)+j] - vertexAtts[index+j];
                         d2[j] = vertexAtts[index+(numSlices*3)+j] - vertexAtts[index+j];
                      }
                   }
                   else {
                      // general case
                      for (j=0; j<3; j++) {
                         d1[j] = vertexAtts[index+3+j] - vertexAtts[index+j];
                         d2[j] = vertexAtts[index+(numSlices*3)+j] - vertexAtts[index+j];
                      }
                   }

                   // compute normalized cross product of edge vectors
                   normCrossProd(d2, d1, normal);

                   // this is the normal
                   vertexAtts[index+0 + normalOffset] = normal[0];
                   vertexAtts[index+1 + normalOffset] = normal[1];
                   vertexAtts[index+2 + normalOffset] = normal[2];
                   undeformedVertexAtts[index+0 + normalOffset] = vertexAtts[index+0 + normalOffset];
                   undeformedVertexAtts[index+1 + normalOffset] = vertexAtts[index+1 + normalOffset];
                   undeformedVertexAtts[index+2 + normalOffset] = vertexAtts[index+2 + normalOffset];
                }
             }
          }

          void drawDeformedSOR(glm::vec3 unDeformedGrid[3][3], glm::vec3 deformedGrid[3][3]) {

             GenerateSORTriangles();
             deformSORVertices(unDeformedGrid, deformedGrid);

             glBufferData(GL_ARRAY_BUFFER, (numSlices*profPts)*(3*4*2), vertexAtts, GL_STATIC_DRAW); //3 floats per vert, 4 bytes per float, 2 sets of data (loc, normal)

          }
};

void normalize(float v[3]) {
   // from OpenGL Programming Guide, p. 58
   GLfloat d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
   if (d == 0.0) {
      //      printf("zero length vector");
      return;
   }
   v[0] = v[0]/d; v[1] = v[1]/d; v[2] = v[2]/d;
}

void normCrossProd(float v1[3], float v2[3], float out[3]) {
   // from OpenGL Programming Guide, p. 58
   out[0] = v1[1]*v2[2] - v1[2]*v2[1];
   out[1] = v1[2]*v2[0] - v1[0]*v2[2];
   out[2] = v1[0]*v2[1] - v1[1]*v2[0];
   normalize(out);
}
