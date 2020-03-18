// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>
#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#define PI 3.1415926535897
const int window_width = 1024, window_height = 768;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], unsigned short[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void rotateCam(void);
void deselectObjs(void);
void translateBase(void);
void rotateTop(void);
void rotateArm1(void);
void rotateArm2(void);
void rotatePen(void);
void teleport(void);

// GLOBAL VARIABLES
GLFWwindow* window;
glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;
GLuint gPickedIndex = -1;
std::string gMessage;
GLuint programID;
GLuint pickingProgramID;
GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
GLuint LightID1;
GLuint AmbientPowID;
GLuint LightPowID;

const GLuint NumObjects = 17;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
GLuint VertexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
GLuint IndexBufferId[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
size_t NumIndices[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
size_t VertexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
size_t IndexBufferSize[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
size_t NumVertices[NumObjects] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16  };

// teleport
bool jump = false;
GLuint tele = 1, animate = 0;
GLfloat jumpsin = 0.0f, tele1 = 0.0f, tele2 = 0.01f, tele3 = 0.0f, jumpcos = 0.0f, jumpa = 0.0f, jumpb = 0.0f, jumpc = 0.0f, jumpx = 0.0f, jumpz = 0.0f, jumpy = 0.0f, gravity = 9.8f;
glm::vec3 jumpvector, vector;
glm::vec4 initial, jumpinitial;

// objs vars
GLfloat BaseX = 0.0f, BaseZ = 0.0f, TopX = 0.0f, Arm1Z = 0.0f, Arm2Z = 0.0f, PenZ = 0.0f, PenY = 0.0f, PenX = 0.0f;
int keyPressed = 0;
int shiftPressed = 0;
Vertex* Verts1, *Verts2, *Verts3, *Verts4, *Verts5, *Verts6, *Verts7, *Verts8, *Verts9, *Verts10, *Verts11, *Verts12, *Verts13, *Verts14, *Verts15;
GLushort* Idcs1, *Idcs2, *Idcs3, *Idcs4, *Idcs5, *Idcs6, *Idcs7, *Idcs8, *Idcs9, *Idcs10, *Idcs11, *Idcs12, *Idcs13, *Idcs14, *Idcs15;

// animation control
bool animation = false;
GLfloat phi = 0.0;

// camera vars
int direction = 0;
GLfloat cameraAngleX = PI / 4;
GLfloat cameraAngleY = asin(1 / (sqrt(3)));
GLfloat cameraRadius = sqrt(300);

// objs indicies
unsigned int BaseIndex = 2;
unsigned int Arm1Index = 4;
unsigned int Arm2Index = 6;
unsigned int ButtonIndex = 8;
unsigned int JointIndex = 5;
unsigned int PenIndex = 7;
unsigned int TopIndex = 3;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
	NumVertices[ObjectId] = vertCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//
	Vertex Grid[44];
	int j = 0;
	for (float i = -5.0; i <= 5.0; i++) {
		Grid[4 * j] = { {i, 0.0, -5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * j + 1] = { {i, 0.0, 5.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * j + 2] = { {-5.0, 0.0, i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		Grid[4 * j + 3] = { {5.0, 0.0, i, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0, 1.0} };
		j++;
	}

	// ATTN: create your grid vertices here!
	VertexBufferSize[1] = sizeof(Grid);
	createVAOs(Grid, NULL, 1);

	// base objs
	loadObject("models/base.obj", glm::vec4(0.0, 0.0, 0.0, 1.0), Verts1, Idcs1, 2);
	createVAOs(Verts1, Idcs1, 2);
	loadObject("models/top.obj", glm::vec4(0.29, 0.29, 0.29, 1.0), Verts2, Idcs2, 3);
	createVAOs(Verts2, Idcs2, 3);
	loadObject("models/arm1.obj", glm::vec4(0.39, 0.0, 0.0, 1.0), Verts3, Idcs3, 4);
	createVAOs(Verts3, Idcs3, 4);
	loadObject("models/joint.obj", glm::vec4(0.29, 0.29, 0.29, 1.0), Verts4, Idcs4, 5);
	createVAOs(Verts4, Idcs4, 5);
	loadObject("models/arm2.obj", glm::vec4(0.39, 0.0, 0.0, 1.0), Verts5, Idcs5, 6);
	createVAOs(Verts5, Idcs5, 6);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 0.0, 1.0), Verts6, Idcs6, 7);
	createVAOs(Verts6, Idcs6, 7);
	loadObject("models/button.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts7, Idcs7, 8);
	createVAOs(Verts7, Idcs7, 8);

	// selected objs
	loadObject("models/base.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts9, Idcs9, 9);
	createVAOs(Verts9, Idcs9, 9);
	loadObject("models/top.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts10, Idcs10, 10);
	createVAOs(Verts10, Idcs10, 10);
	loadObject("models/arm1.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts11, Idcs11, 11);
	createVAOs(Verts11, Idcs11, 11);
	loadObject("models/joint.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts12, Idcs12, 12);
	createVAOs(Verts12, Idcs12, 12);
	loadObject("models/arm2.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts13, Idcs13, 13);
	createVAOs(Verts13, Idcs13, 13);
	loadObject("models/pen.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts14, Idcs14, 14);
	createVAOs(Verts14, Idcs14, 14);
	loadObject("models/button.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts15, Idcs15, 15);
	createVAOs(Verts15, Idcs15, 15);

	// bullet
	loadObject("models/bullet.obj", glm::vec4(0.0, 1.0, 1.0, 1.0), Verts8, Idcs8, 16);
	createVAOs(Verts8, Idcs8, 16);
}

void teleport() {
	if (animate == 1) {
		if (tele == 1) {
			jumpa = jumpinitial[0] - initial[0];
			jumpb = jumpinitial[1] - initial[1];
			jumpc = jumpinitial[2] - initial[2];
			vector = glm::normalize(vec3(jumpa, jumpb, jumpc));
			jumpvector = glm::normalize(vec3(jumpa, 0, jumpc));
			tele = 0;
		}
		if (jumpinitial[0] - initial[0] >= 0) {
			jumpsin = glm::dot(vector, vec3(0, 1, 0));
			tele2 = glm::dot(jumpvector, vec3(0, 0, 1));
			jumpcos = cosf(glm::asin(jumpsin));
			jumpz = 2.7f*jumpcos*tele1 * tele2;
			tele3 = cosf(glm::asin(tele2));
			jumpx = 2.7f*jumpcos*tele1 * tele3;
			jumpy = (2.7f*jumpsin*tele1) - ((0.5f)*gravity*tele1*tele1);
			tele1 += 0.01f;
		}
		else if (jumpinitial[0] - initial[0] < 0) {
			jumpsin = glm::dot(vector, vec3(0, 1, 0));
			tele2 = glm::dot(jumpvector, vec3(0, 0, 1));
			jumpcos = cosf(glm::asin(jumpsin));
			jumpz = 2.7f*jumpcos*tele1 * tele2;
			tele3 = cosf(glm::asin(tele2));
			jumpx = -(2.7f*jumpcos*tele1*tele3);
			jumpy = (2.7f*jumpsin*tele1) - ((0.5f)*gravity*tele1*tele1);
			tele1 += 0.01f;
		}
	}
}

void rotatePen(void) {
	if (direction == 1) { //left
		if (shiftPressed) {
			PenY -= 0.05f;
		}
		else {
			PenX -= 0.01f;
		}
	}
	if (direction == 2) { //right
		if (shiftPressed) {
			PenY += 0.05f;
		}
		else {
			PenX += 0.01f;
		}
	}
	if (direction == 3) { //up
		PenZ += 0.01f;
	}
	if (direction == 4) { //down
		PenZ -= 0.01f;
	}
}

void rotateArm2(void) {
	if (direction == 3) { //up
		Arm2Z += 0.01f;
	}
	if (direction == 4) { //down
		Arm2Z -= 0.01f;
	}
}

void rotateArm1(void) {
	if (direction == 3) { //up
		Arm1Z += 0.01f;
	}
	if (direction == 4) { //down
		Arm1Z -= 0.01f;
	}
}

void rotateTop(void) {
	if (direction == 1) { //left
		TopX -= 0.01f;
	}
	if (direction == 2) { //right
		TopX += 0.01f;
	}
}

void translateBase(void) {
	if (direction == 1 && BaseX >= -5.0f) { //left
		BaseX -= 0.1f;
	}
	if (direction == 2 && BaseX <= 5.0f) { //right
		BaseX += 0.1f;
	}
	if (direction == 3 && BaseZ >= -5.0f) { //up
		BaseZ -= 0.1f;
	}
	if (direction == 4 && BaseZ <= 5.0f) { //down
		BaseZ += 0.1f;
	}
}

void rotateCam() {
	if (direction == 1) {
		cameraAngleX -= 0.01;
	}
	if (direction == 2) {
		cameraAngleX += 0.01;
	}
	if (direction == 3) {
		cameraAngleY += 0.01;
	}
	if (direction == 4) {
		cameraAngleY -= 0.01;
	}
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
	float cameraX = cameraRadius * cos(cameraAngleY) * sin(cameraAngleX);
	float cameraY = cameraRadius * sin(cameraAngleY);
	float cameraZ = cameraRadius * cos(cameraAngleY) * cos(cameraAngleX);
	gViewMatrix = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glm::vec3 lightPos = glm::vec3(4, 4, -4);
		glm::vec3 lightPos1 = glm::vec3(-4, 4, 4);
		glUniform1f(LightPowID, 50.0f);
		glUniform1f(AmbientPowID, 0.3f);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(LightID1, lightPos1.x, lightPos1.y, lightPos1.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(VertexArrayId[1]);   // draw grid
		glDrawArrays(GL_LINES, 0, 44);

		// base
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(BaseX, 0.0f, BaseZ));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[BaseIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[BaseIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts1), Verts1);
		glDrawElements(GL_TRIANGLES, NumIndices[BaseIndex], GL_UNSIGNED_SHORT, (void *)0);
		// top
		ModelMatrix = glm::rotate(ModelMatrix, TopX, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[TopIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[TopIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts2), Verts2);
		glDrawElements(GL_TRIANGLES, NumIndices[TopIndex], GL_UNSIGNED_SHORT, (void *)0);
		// arm1
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 2.11f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, Arm1Z, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -2.11f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[Arm1Index]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[Arm1Index]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts3), Verts3);
		glDrawElements(GL_TRIANGLES, NumIndices[Arm1Index], GL_UNSIGNED_SHORT, (void *)0);
		// joint
		glBindVertexArray(VertexArrayId[JointIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[JointIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts4), Verts4);
		glDrawElements(GL_TRIANGLES, NumIndices[JointIndex], GL_UNSIGNED_SHORT, (void *)0);
		// arm2
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.63f, 2.07f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, Arm2Z, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-1.63f, -2.07f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[Arm2Index]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[Arm2Index]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts5), Verts5);
		glDrawElements(GL_TRIANGLES, NumIndices[Arm2Index], GL_UNSIGNED_SHORT, (void *)0);
		// pen
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.63f, 1.05f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenX, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenZ, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenY, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-1.63f, -1.05f, 0.0f));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glBindVertexArray(VertexArrayId[PenIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[PenIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts6), Verts6);
		glDrawElements(GL_TRIANGLES, NumIndices[PenIndex], GL_UNSIGNED_SHORT, (void *)0);
		// button
		glBindVertexArray(VertexArrayId[ButtonIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ButtonIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts7), Verts7);
		glDrawElements(GL_TRIANGLES, NumIndices[ButtonIndex], GL_UNSIGNED_SHORT, (void *)0);

		// teleport
		initial = ModelMatrix * (glm::vec4(1.5f, 1.0f, 0.0f, 1.0f));
		jumpinitial = ModelMatrix * (glm::vec4(2.7f, 1.0f, 0.0f, 1.0f));
		if (jump == true) {
			glm::mat4 rotation = glm::translate(glm::mat4(), glm::vec3(jumpx, jumpy, jumpz));
			ModelMatrix = rotation * ModelMatrix;
			glm::vec4 Pos = glm::vec4(Verts8[0].Position[0], Verts8[0].Position[1], Verts8[0].Position[2], Verts8[0].Position[3]);
			glm::vec4 jumpPos = ModelMatrix * Pos;
			if (jumpPos[1] <= 0) {
				animate = 0;
				BaseX = jumpPos[0];
				BaseZ = jumpPos[2];
				if (BaseX > 5.0f || BaseX < -5.0f) {
					BaseX = 0;
					BaseZ = 0;
				}
				if (BaseZ > 5.0f || BaseZ < -5.0f) {
					BaseZ = 0;
					BaseX = 0;
				}
				jump = false;
			}
			// bullet
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glBindVertexArray(VertexArrayId[16]);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[16]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts8), Verts8);
			glDrawElements(GL_TRIANGLES, NumIndices[16], GL_UNSIGNED_SHORT, (void *)0);
		}
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void deselectObjs() {
	BaseIndex = 2;
	Arm1Index = 4;
	Arm2Index = 6;
	ButtonIndex = 8;
	JointIndex = 5;
	PenIndex = 7;
	TopIndex = 3;
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		// base
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(BaseX, 0.0f, BaseZ));
		glBindVertexArray(VertexArrayId[BaseIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[BaseIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts9), Verts9);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, BaseIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[BaseIndex], GL_UNSIGNED_SHORT, (void *)0);
		// top
		ModelMatrix = glm::rotate(ModelMatrix, TopX, glm::vec3(0.0f, 1.0f, 0.0f));
		glBindVertexArray(VertexArrayId[TopIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[TopIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts10), Verts10);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, TopIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[TopIndex], GL_UNSIGNED_SHORT, (void *)0);
		// arm1
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 2.11f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, Arm1Z, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -2.11f, 0.0f));
		glBindVertexArray(VertexArrayId[Arm1Index]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[Arm1Index]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts11), Verts11);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, Arm1Index / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[Arm1Index], GL_UNSIGNED_SHORT, (void *)0);
		// joint
		glBindVertexArray(VertexArrayId[JointIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[JointIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts12), Verts12);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, JointIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[JointIndex], GL_UNSIGNED_SHORT, (void *)0);
		// arm2
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.63f, 2.07f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, Arm2Z, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-1.63f, -2.07f, 0.0f));
		glBindVertexArray(VertexArrayId[Arm2Index]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[Arm2Index]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts13), Verts13);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, Arm2Index / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[Arm2Index], GL_UNSIGNED_SHORT, (void *)0);
		// pen
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.63f, 1.05f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenX, glm::vec3(0.0f, 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenZ, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, PenY, glm::vec3(1.0f, 0.0f, 0.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-1.63f, -1.05f, 0.0f));
		glBindVertexArray(VertexArrayId[PenIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[PenIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts14), Verts14);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, PenIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[PenIndex], GL_UNSIGNED_SHORT, (void *)0);
		// button
		glBindVertexArray(VertexArrayId[ButtonIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ButtonIndex]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts15), Verts15);
		MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(pickingColorID, ButtonIndex / 255.0f);
		glDrawElements(GL_TRIANGLES, NumIndices[ButtonIndex], GL_UNSIGNED_SHORT, (void *)0);

		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 255) { // Full white, must be the background !
		gMessage = "background";
		deselectObjs();
	}
	else {
		std::ostringstream oss;
		switch (gPickedIndex) {
		case 2:
			oss << "Base";
			deselectObjs();
			keyPressed = 5;
			BaseIndex = 9;
			break;
		case 3:
			oss << "Top";
			deselectObjs();
			keyPressed = 6;
			TopIndex = 10;
			break;
		case 4:
			oss << "Arm1";
			deselectObjs();
			keyPressed = 1;
			Arm1Index = 11;
			break;
		case 5:
			oss << "Joint";
			deselectObjs();
			JointIndex = 12;
			break;
		case 6:
			oss << "Arm2";
			deselectObjs();
			keyPressed = 2;
			Arm2Index = 13;
			break;
		case 7:
			oss << "Pen";
			deselectObjs();
			keyPressed = 4;
			PenIndex = 14;
			break;
		case 8:
			oss << "Button";
			deselectObjs();
			ButtonIndex = 15;
			break;
		case 9:
			oss << "Base";
			break;
		case 10:
			oss << "Top";
			break;
		case 11:
			oss << "Arm1";
			break;
		case 12:
			oss << "Joint";
			break;
		case 13:
			oss << "Arm2";
			break;
		case 14:
			oss << "Pen";
			break;
		case 15:
			oss << "Button";
			break;
		default:
			oss << "point " << gPickedIndex;
		}
		gMessage = oss.str();
	}
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Yuhan Hou (20199280)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates
	// camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID1 = glGetUniformLocation(programID, "LightPosition_worldspace1");
	AmbientPowID = glGetUniformLocation(programID, "AmbientPower");
	LightPowID = glGetUniformLocation(programID, "LightPower");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_B:
			deselectObjs();
			if (keyPressed != 5) {
				keyPressed = 5;
				BaseIndex = 9;
				printf("Base is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Base is deselected\n");
			}
			break;
		case GLFW_KEY_T:
			deselectObjs();
			if (keyPressed != 6) {
				keyPressed = 6;
				TopIndex = 10;
				printf("Top is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Top is deselected\n");
			}
			break;
		case GLFW_KEY_1:
			deselectObjs();
			if (keyPressed != 1) {
				keyPressed = 1;
				Arm1Index = 11;
				printf("Arm1 is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Arm1 is deselected\n");
			}
			break;
		case GLFW_KEY_2:
			deselectObjs();
			if (keyPressed != 2) {
				keyPressed = 2;
				Arm2Index = 13;
				printf("Arm2 is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Arm2 is deselected\n");
			}
			break;
		case GLFW_KEY_P:
			deselectObjs();
			if (keyPressed != 4) {
				keyPressed = 4;
				PenIndex = 14;
				printf("Pen is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Pen is deselected\n");
			}
			break;
		case GLFW_KEY_C:
			deselectObjs();
			if (keyPressed != 3) {
				keyPressed = 3;
				printf("Camera is selected\n");
			}
			else {
				keyPressed = 0;
				printf("Camera is deselected\n");
			}
			break;
		case GLFW_KEY_LEFT:
			direction = 1;
			break;
		case GLFW_KEY_RIGHT:
			direction = 2;
			break;
		case GLFW_KEY_UP:
			direction = 3;
			break;
		case GLFW_KEY_DOWN:
			direction = 4;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			shiftPressed = 1;
			printf("Left shift key pressed\n");
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			shiftPressed = 1;
			printf("Right shift key pressed\n");
			break;
		case GLFW_KEY_J:
			animate = 1;
			jump = true;
			tele1 = 0.01f;
			tele = 1;
			break;
		default:
			break;
		}
	}
	if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_LEFT_SHIFT:
			shiftPressed = 0;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			shiftPressed = 0;
			break;
		default:
			direction = 0;
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}
		
		if (animation) {
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}
		teleport();
		if (keyPressed == 1) {
			rotateArm1();
		}
		if (keyPressed == 2) {
			rotateArm2();
		}
		if (keyPressed == 3) {
			rotateCam();
		}
		if (keyPressed == 4) {
			rotatePen();
		}
		if (keyPressed == 5) {
			translateBase();
		}
		if (keyPressed == 6) {
			rotateTop();
		}
		// DRAWING POINTS
		renderScene();
		//getch();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}