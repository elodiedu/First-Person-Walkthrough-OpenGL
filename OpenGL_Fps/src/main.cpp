// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <cmath>


// Includes
#include "trimesh.hpp"
#include "shader.hpp"
#include <cstring> // memcpy

// Constants
#define WIN_WIDTH 500
#define WIN_HEIGHT 500
struct Vec3 {
	float x, y, z;

	Vec3() {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};
class Mat4x4 {
public:

	float m[16];

	Mat4x4(){ // Default: Identity
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void make_identity(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void print(){
		std::cout << m[0] << ' ' <<  m[4] << ' ' <<  m[8]  << ' ' <<  m[12] << "\n";
		std::cout << m[1] << ' ' <<   m[5] << ' ' <<  m[9]  << ' ' <<   m[13] << "\n";
		std::cout << m[2] << ' ' <<   m[6] << ' ' <<  m[10] << ' ' <<   m[14] << "\n";
		std::cout << m[3] << ' ' <<   m[7] << ' ' <<  m[11] << ' ' <<   m[15] << "\n";
	}

	void make_scale(float x, float y, float z){
		make_identity();
		m[0] = x; m[5] = y; m[10] = z;
	}
};
static inline const Vec3f operator*(const Mat4x4 &m, const Vec3f &v){
	Vec3f r( m.m[0]*v[0]+m.m[4]*v[1]+m.m[8]*v[2],
		m.m[1]*v[0]+m.m[5]*v[1]+m.m[9]*v[2],
		m.m[2]*v[0]+m.m[6]*v[1]+m.m[10]*v[2] );
	return r;
}


//
//	Global state variables
//
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height; // window size
	float aspect;
	GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	TriMesh mesh;
	float EyeSpeed = 0.5f;
	float RotateSpeed = 5.0f;
	Vec3 forward;
	Vec3 u, v, w;
	//  Model, view and projection matrices, initialized to the identity
	Mat4x4 model;
	Mat4x4 view;
	Mat4x4 projection;
	//Implement parameters on Cam, 
	Vec3 eye, ViewDir, UpDir;

}


//
float radians(float deg)
{
	return deg * 3.1415926535f / 180.0f;
}
static Vec3 Normalize_Vec(const Vec3& v)
{
	
	float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return Vec3(v.x / len, v.y / len, v.z / len);
}
static Vec3 cross_vec(const Vec3& a, const Vec3& b)
{
	return Vec3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}
static float Dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
void Yaw(float angle)
{
	float a = radians(angle);
	float c = cos(a);
	float s = sin(a);

	Vec3 u = Globals::u;
	Vec3 v = Globals::v;
	Vec3 w = Globals::w;

	Vec3 u_new = Vec3(
		u.x * c + (v.y * u.z - v.z * u.y) * s + v.x * Dot(v, u) * (1 - c),
		u.y * c + (v.z * u.x - v.x * u.z) * s + v.y * Dot(v, u) * (1 - c),
		u.z * c + (v.x * u.y - v.y * u.x) * s + v.z * Dot(v, u) * (1 - c)
	);

	Vec3 w_new = Vec3(
		w.x * c + (v.y * w.z - v.z * w.y) * s + v.x * Dot(v, w) * (1 - c),
		w.y * c + (v.z * w.x - v.x * w.z) * s + v.y * Dot(v, w) * (1 - c),
		w.z * c + (v.x * w.y - v.y * w.x) * s + v.z * Dot(v, w) * (1 - c)
	);

	Globals::u = Normalize_Vec(u_new);
	Globals::w = Normalize_Vec(w_new);
	Globals::v = Normalize_Vec(cross_vec(Globals::w, Globals::u));

	Globals::ViewDir = Vec3(
		-Globals::w.x,
		-Globals::w.y,
		-Globals::w.z
	);
}

void Pitch(float angle)
{
	float a = radians(angle);
	float c = cos(a);
	float s = sin(a);

	Vec3 u = Globals::u;
	Vec3 v = Globals::v;
	Vec3 w = Globals::w;
	Vec3 v_new = Vec3(
		v.x * c + (u.y * v.z - u.z * v.y) * s + u.x * (u.x * v.x + u.y * v.y + u.z * v.z) * (1 - c),
		v.y * c + (u.z * v.x - u.x * v.z) * s + u.y * (u.x * v.x + u.y * v.y + u.z * v.z) * (1 - c),
		v.z * c + (u.x * v.y - u.y * v.x) * s + u.z * (u.x * v.x + u.y * v.y + u.z * v.z) * (1 - c)
	);
	Vec3 w_new = Vec3(
		w.x * c + (u.y * w.z - u.z * w.y) * s + u.x * (u.x * w.x + u.y * w.y + u.z * w.z) * (1 - c),
		w.y * c + (u.z * w.x - u.x * w.z) * s + u.y * (u.x * w.x + u.y * w.y + u.z * w.z) * (1 - c),
		w.z * c + (u.x * w.y - u.y * w.x) * s + u.z * (u.x * w.x + u.y * w.y + u.z * w.z) * (1 - c)
	);

	Globals::v = Normalize_Vec(v_new);
	Globals::w = Normalize_Vec(w_new);
	Globals::u = Normalize_Vec(cross_vec(Globals::v, Globals::w));
	Globals::ViewDir = Vec3(
		-Globals::w.x,
		-Globals::w.y,
		-Globals::w.z
	);
}

//Update the Object to Camera Matrix
static void Update_View()
{
	

	Globals::view.m[0] = Globals::u.x;
	Globals::view.m[1] = Globals::v.x;
	Globals::view.m[2] = Globals::w.x;
	Globals::view.m[3] = 0.0f;

	Globals::view.m[4] = Globals::u.y;
	Globals::view.m[5] = Globals::v.y;
	Globals::view.m[6] = Globals::w.y;
	Globals::view.m[7] = 0.0f;

	Globals::view.m[8] = Globals::u.z;
	Globals::view.m[9] = Globals::v.z;
	Globals::view.m[10] = Globals::w.z;
	Globals::view.m[11] = 0.0f;

	Globals::view.m[12] = -Dot(Globals::u, Globals::eye);
	Globals::view.m[13] = -Dot(Globals::v, Globals::eye);
	Globals::view.m[14] = -Dot(Globals::w, Globals::eye);
	Globals::view.m[15] = 1.0f;
}
//Calculate the View projection matrix
static void Update_Proj(float width, float height)
{
	float aspect = width / height;
	float nearPlane = 0.05f;
	float farPlane = 100.0f;
	float fov = radians(60.0f);
	float f = 1.0f / tan(fov / 2.0f);

	Globals::projection.m[0] = f / aspect;
	Globals::projection.m[1] = 0.0f;
	Globals::projection.m[2] = 0.0f;
	Globals::projection.m[3] = 0.0f;

	Globals::projection.m[4] = 0.0f;
	Globals::projection.m[5] = f;
	Globals::projection.m[6] = 0.0f;
	Globals::projection.m[7] = 0.0f;

	Globals::projection.m[8] = 0.0f;
	Globals::projection.m[9] = 0.0f;
	Globals::projection.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
	Globals::projection.m[11] = -1.0f;

	Globals::projection.m[12] = 0.0f;
	Globals::projection.m[13] = 0.0f;
	Globals::projection.m[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
	Globals::projection.m[15] = 0.0f;



}
//	Callbacks
//
static void error_callback(int error, const char* description){ fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	Vec3 forward = Normalize_Vec(Globals::ViewDir);
	Vec3 w = Normalize_Vec(Vec3(
		-Globals::ViewDir.x,
		-Globals::ViewDir.y,
		-Globals::ViewDir.z
	));
	Vec3 right = Normalize_Vec(cross_vec(Vec3(0,1,0), w));
	// Close on escape or Q
	if( action == GLFW_PRESS ){
		switch ( key ) {
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_W:
				Globals::eye.x += Globals::EyeSpeed * forward.x;
				Globals::eye.y += Globals::EyeSpeed * forward.y;
				Globals::eye.z += Globals::EyeSpeed * forward.z;
				Update_View();
				break;
				//Update the forward and back movement
			case GLFW_KEY_S:
				Globals::eye.x -= Globals::EyeSpeed * forward.x;
				Globals::eye.y -= Globals::EyeSpeed * forward.y;
				Globals::eye.z -= Globals::EyeSpeed * forward.z;
				Update_View();
				break;
				//Update the left and right movement
			case GLFW_KEY_A:
				Globals::eye.x -= Globals::EyeSpeed * right.x;
				Globals::eye.y -= Globals::EyeSpeed * right.y;
				Globals::eye.z -= Globals::EyeSpeed * right.z;
				Update_View();
				break;
				
			case GLFW_KEY_D:
				Globals::eye.x += Globals::EyeSpeed * right.x;
				Globals::eye.y += Globals::EyeSpeed * right.y;
				Globals::eye.z += Globals::EyeSpeed * right.z;
				Update_View();
				break;
				//Update the pan movement
			case GLFW_KEY_UP:
				Pitch(Globals::RotateSpeed);
				Update_View();
				break;

			case GLFW_KEY_DOWN:
				Pitch(-Globals::RotateSpeed);
				Update_View();
				break;
			case GLFW_KEY_LEFT:
				Yaw(Globals::RotateSpeed);
				Update_View();
				break;

			case GLFW_KEY_RIGHT:
				Yaw(- Globals::RotateSpeed);
				Update_View();
				break;
			case GLFW_KEY_LEFT_BRACKET: 
				Globals::eye.y -= Globals::EyeSpeed;
				Update_View();
				break;

			case GLFW_KEY_RIGHT_BRACKET:
				Globals::eye.y += Globals::EyeSpeed;
				Update_View();
				break;
            // ToDo: update the viewing transformation matrix according to key presses
		}
	}

}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    int win_width, win_height;
    float aspect;
    glfwGetFramebufferSize(window, &win_width, &win_height);

    aspect = (float)win_width/(float)win_height;
    
    glViewport(0,0,width,height);
	Update_Proj(float(width), float(height));

    // ToDo: update the perspective matrix as the window size changes

}


// Function to set up geometry
void init_scene();


//
//	Main
//
int main(int argc, char *argv[]){

	// Load the mesh
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	if( !Globals::mesh.load_obj( obj_file.str() ) ){ return 0; }
	Globals::mesh.print_details();

	// Forcibly scale the mesh vertices so that the entire model fits within a (-1,1) volume: the code below is a temporary measure that is needed to enable the entire model to be visible in the template app, before the student has defined the proper viewing and projection matrices
    	// This code should eventually be replaced by the use of an appropriate projection matrix
    	// FYI: the model dimensions are: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0
    // find the extremum of the vertex locations (this approach works because the model is known to be centered; a more complicated method would be required in the general case)
   
    // The above can be removed once a proper projection matrix is defined

	// Set up the window variable
	GLFWwindow* window;
    
    // Define the error callback function
	glfwSetErrorCallback(&error_callback);

	// Initialize glfw
	if( !glfwInit() ){ return EXIT_FAILURE; }

	// Ask for OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	Globals::win_width = WIN_WIDTH;
	Globals::win_height = WIN_HEIGHT;
	window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2b", NULL, NULL);
	if( !window ){ glfwTerminate(); return EXIT_FAILURE; }

	// Define callbacks to handle user input and window resizing
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// More setup stuff
	glfwMakeContextCurrent(window); // Make the window current
    glfwSwapInterval(1); // Set the swap interval

	// make sure the openGL code can be found; folks using Windows need this
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Initialize the shaders
	// MY_SRC_DIR was defined in CMakeLists.txt
	// it specifies the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss; ss << MY_SRC_DIR << "shader.";
	shader.init_from_files( ss.str()+"vert", ss.str()+"frag" );

	// Initialize the scene
	init_scene();
	Update_View();
	Update_Proj(float(Globals::win_width), float(Globals::win_height));
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Perform some OpenGL initializations
	glEnable(GL_DEPTH_TEST);  // turn hidden surfce removal on
	glClearColor(1.f,1.f,1.f,1.f);  // set the background to white

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();

	// Bind buffers
	glBindVertexArray(Globals::tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);
    
	// Game loop
	while( !glfwWindowShouldClose(window) ){

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send updated info to the GPU
		glUniformMatrix4fv( shader.uniform("model"), 1, GL_FALSE, Globals::model.m  ); // model transformation
		glUniformMatrix4fv( shader.uniform("view"), 1, GL_FALSE, Globals::view.m  ); // viewing transformation
		glUniformMatrix4fv( shader.uniform("projection"), 1, GL_FALSE, Globals::projection.m ); // projection matrix

		// Draw
		glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size()*3, GL_UNSIGNED_INT, 0);

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();
    
	return EXIT_SUCCESS;
}


void init_scene(){

	using namespace Globals;
	//Define camera position and orientations
	Globals::eye = Vec3(10.0f, -12.0f, 0.0f);
	Globals::ViewDir = Normalize_Vec(Vec3(-1.0f, 0.0f, 0.0f));


	Globals::w = Normalize_Vec(Vec3(
		-Globals::ViewDir.x,
		-Globals::ViewDir.y,
		-Globals::ViewDir.z
	));

	Globals::u = Normalize_Vec(cross_vec(Vec3(0.0f, 1.0f, 0.0f), Globals::w));
	Globals::v = Normalize_Vec(cross_vec(Globals::w, Globals::u));
	// Create the buffer for vertices
	glGenBuffers(1, verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.colors.size()*sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size()*sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size()*sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
	glBindVertexArray(tris_vao);

	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

	// Done setting data for the vao
	glBindVertexArray(0);

}

