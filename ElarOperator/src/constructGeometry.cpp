#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "elar_operator.h"
#include "half_edge_structure.h"
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "shader_m.h"
#include <stack>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
	//构建底面
	double point1[3] = {-0.5,-0.5,-0.8};
	Vertex* v1;
	Solid* s1;
	ElarOperator* elar = new ElarOperator();
	s1 = elar->mvfs(point1,v1);

	double point2[3] = {0.5,-0.5,-0.8};
	Loop* lp = s1->faces->out_lp;
	HalfEdge* he1 = elar->mev(v1,point2,lp);

	double point3[3] = {0.5,0.5,-0.8};
	lp = s1->faces->out_lp;
	HalfEdge* he2 = elar->mev(he1->ev,point3,lp);

	double point4[3] = {-0.5,0.5,-0.8};
	lp = s1->faces->out_lp;
	HalfEdge* he3 = elar->mev(he2->ev,point4,lp);

	lp = s1->faces->out_lp;
	lp = elar->mef(he3->ev,he1->sv,lp,true);

	//加入内环
	double inp1[3] = { -0.375,-0.25,-0.8 };
	lp = s1->faces->out_lp;
	HalfEdge* het = elar->mev(v1, inp1, lp);
	Vertex* inv1 = het->ev;

	elar->kemr(v1, inv1, lp);

	double inp2[3] = { 0.125,-0.25,-0.8 };
	Loop* inlp1 = s1->faces->next->inner_lp;
	inlp1->halfedges = NULL;
	//Loop* inlp1 = new Loop();
	//s1->faces->inner_lp = inlp1;
	//inlp1->face = s1->faces;
	HalfEdge* inhe1 = elar->mev(inv1, inp2, inlp1);

	double inp3[3] = { 0.125,0.25,-0.8 };
	HalfEdge* inhe2 = elar->mev(inhe1->ev, inp3, inlp1);

	double inp4[3] = { -0.375,0.25,-0.8 };
	HalfEdge* inhe3 = elar->mev(inhe2->ev, inp4, inlp1);

	inlp1 = elar->mef(inhe3->ev, inhe1->sv, inlp1, false);

	elar->kfmrh(s1->faces,s1->faces->next->next);

	double dir[3] = { 0,0,1 }; double d = 1.6;//扫成的方向和距离
	elar->sweep(dir,d);

	
	bool flag = false;
	vector<double*> vs;
	Face* f = s1->faces; int faceNum = 0;
	for (; f != NULL; f = f->next)
	{
		faceNum++;
		Loop* lp = f->out_lp;
		Loop* inlp = f->inner_lp;
		HalfEdge* he = lp->halfedges->pre;
		Vertex* sv = he->sv; Vertex* nextv = he->ev;

		vector<double*> parray;//保存外环四个点
		double *p1 = new double[3]{sv->coordinate[0],sv->coordinate[1],sv->coordinate[2]};
		cout << p1[0] << " " << p1[1] << " " << p1[2] << endl;
		parray.push_back(p1);
		for (; nextv != sv;)
		{
			double* tmpp = new double[3]{ nextv->coordinate[0],nextv->coordinate[1],nextv->coordinate[2] };
			parray.push_back(tmpp);
			cout << tmpp[0] << " " << tmpp[1] << " "<<tmpp[2] << endl;

			he = he->next;
			nextv = he->ev;
		}
		
		if (inlp != NULL)
		{
			vector<double*> inarray;//保存内环顶点
			HalfEdge* inhe;
			if (!flag)
			{
				inhe = inlp->halfedges;
				flag = true;
			}
			else
			{
				inhe = inlp->halfedges->next->next;
			}
			Vertex* insv = inhe->sv; Vertex* innext = inhe->ev;
			double* inp1 = new double[3]{ insv->coordinate[0],insv->coordinate[1],insv->coordinate[2] };
			cout << inp1[0] << " " << inp1[1] << " " << inp1[2] << endl;
			inarray.push_back(inp1);
			stack<double*> stk;
			for (; innext != insv;)
			{
				double* inp = new double[3]{innext->coordinate[0],innext->coordinate[1],innext->coordinate[2]};
				stk.push(inp);
				

				inhe = inhe->next;
				innext = inhe->ev;
			}
			for (int k = 0; k < 3; k++)
			{
				double* tmp = stk.top();
				inarray.push_back(tmp);
				cout << tmp[0] << " " << tmp[1] << " " << tmp[2] << endl;
				stk.pop();
			}
			cout << inarray.size();

			for (int i = 0; i < 4; i++)
			{
				int j = (i + 1) % 4;
				vs.push_back(parray[i]); //cout << parray[i][0] << " " << parray[i][1] << " " << parray[i][2] << endl;
				vs.push_back(inarray[j]); //cout << inarray[j][0] << " " << inarray[j][1] << " " << inarray[j][2] << endl;
				vs.push_back(inarray[i]); //cout << inarray[i][0] << " " << inarray[i][1] << " " << inarray[i][2] << endl;
			}
			for (int i = 0; i < 4; i++)
			{
				int j = (i + 1) % 4;
				vs.push_back(parray[i]);
				vs.push_back(parray[j]);
				vs.push_back(inarray[j]);
			}
			//f = f->next;
		}
		else
		{
			vs.push_back(parray[0]); vs.push_back(parray[1]); vs.push_back(parray[3]);
			vs.push_back(parray[1]); vs.push_back(parray[2]); vs.push_back(parray[3]);
		}

	}
	cout << "fNum: " << faceNum << endl;
	int vNum = vs.size(); cout << "vNum: " << vNum<<endl;
	float* vertices = new float[vNum*3];
	int cnt = 0;
	for (int idx = 0; idx < vNum * 3; idx += 3)
	{
		vertices[idx] = vs[cnt][0];
		vertices[idx + 1] = vs[cnt][1];
		vertices[idx + 2] = vs[cnt][2];
		cnt++;
		cout << vertices[idx] << " " << vertices[idx + 1]<<" " << vertices[idx + 2]<<endl;
	}
	
	
	//OpenGL设置
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ElarOperator", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("src/cam1.txt", "src/cam2.txt");

	

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(vNum * 3), vertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	//glBindVertexArray(0);
	glm::vec3 cubePositions = {
		glm::vec3(0.0f,  0.0f,  0.0f) };
	
	
	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		ourShader.use();

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		// camera/view transformation
		glm::mat4 view = glm::lookAt(cameraPos, -cameraPos + glm::vec3(0,0,0), cameraUp);
		ourShader.setMat4("view", view);

		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, cubePositions);
		float angle = 20.0f * 0;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		ourShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, vNum);
		
		// glBindVertexArray(0); // no need to unbind it every time 

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = static_cast<float>(2.5 * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
		//cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}