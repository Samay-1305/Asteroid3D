/*
ZJ Wood, Dunn, Eckhardt CPE 471 Lab base code
*/

#include <iostream>
#include <glad/glad.h>

#include <queue>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <time.h>

using namespace std;
using namespace glm;

GLFWwindow *activeWindow;

#define PI 3.1415926f

#define WIDTH 1920
#define HEIGHT 1080

std::string resourceDirectory = "../../resources";

float randRange(int low, int high){
    return low + ((((float) rand())/((float) RAND_MAX)) * (high - low));
}

vec3 randVec3(){
    return vec3(randRange(0, PI), randRange(0, PI), randRange(0, PI));
}

vec3 randShift(){
    return vec3(randRange(0, PI)/(PI * 10), randRange(0, PI)/(PI * 10), randRange(0, PI)/(PI * 10));
}

mat4 getPerspectiveMatrix(float width, float height){
    return perspective((float)(PI/ 4.0f), (float)(width/height), 0.1f, 100.0f);
}

mat4 getViewMatrix(float angle){
    return translate(mat4(1), glm::vec3(0, 0, -3));
}

float distance(double X1, double Y1, double X2, double Y2){
    return sqrt(pow(X2 - X1, 2) + pow(Y2 - Y1, 2));
}

float distance(vec3 v1, vec3 v2){
    return sqrt(pow(v2.x - v1.x, 2) + pow(v2.y - v1.y, 2) + pow(v2.z - v1.z, 2));
}

void setColor(char newCol){
    GLfloat cube_colors[] = {
        // front colors
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        // back colors
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
    };
    
    switch (newCol){
        case 'r':
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 1.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 0.0;
            }
            break;
        case 'g':
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 0.0;
                cube_colors[i+1] = 1.0;
                cube_colors[i+2] = 0.0;
            }
            break;
        case 'b':
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 0.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 1.0;
            }
            break;
        case 'x': // Head
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 82.0/255.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 7.0/255.0;
            }
            break;
        case 'y': // Body
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 20.0/255.0;
                cube_colors[i+1] = 16.0/255.0;
                cube_colors[i+2] = 59.0/255.0;
            }
            break;
        case 'z': // Legs
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 0.0/255.0;
                cube_colors[i+1] = 67.0/255.0;
                cube_colors[i+2] = 84.0/255.0;
            }
            break;
        case 'm': // mix
            for (int i=0; i<24; i+=6){
                cube_colors[i] = 1.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 0.0;
            }
            for (int i=3; i<24; i+=6){
                cube_colors[i] = 0.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 1.0;
            }
            break;
        case 'k':
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 0.0;
                cube_colors[i+1] = 0.0;
                cube_colors[i+2] = 0.0;
            }
            break;
        case 'w':
            for (int i=0; i<24; i+=3){
                cube_colors[i] = 1.0;
                cube_colors[i+1] = 1.0;
                cube_colors[i+2] = 1.0;
            }
            break;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_DYNAMIC_DRAW);
}


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}


class Camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
    
	Camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
    
	glm::mat4 process(double ftime)
	{
		float speed = 0;
        float yangle = 0;
		if (w == 1)
		{
			speed = 1 * ftime;
		}
		else if (s == 1)
		{
			speed = -1 * ftime;
		}
		
        if (a == 1){
			yangle = -1 * ftime;
        }
        else if(d == 1){
			yangle = 1 * ftime;
        }
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}
};


typedef struct KeyboardCache{
    int left, right, up, down;
}KeyboardCache;


class ShooterState{
private:
    Shape shooterModel;
    GLuint shooterTexture;
    mat4 I, M;
    vec3 position;
    shared_ptr<Program> program;
    
public:
    ShooterState(){
        I = mat4(1);
    }
    
    void render(vec3 angle){
        program->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shooterTexture);
        M = translate(I, position) *
            rotate(I, -PI/2, vec3(1, 0, 0)) *
            rotate(I, angle.y, vec3(1, 0, 0)) *
            rotate(I, angle.x, vec3(0, 0, 1)) *
            scale(I, vec3(0.25f, 0.25f, 0.25f));
        glUniformMatrix4fv(program->getUniform("M"), 1,
                           GL_FALSE, &M[0][0]);
        shooterModel.draw(program);
        program->unbind();
    }
    
    void move(vec3 pos){
        position += pos;
    }
    
    vec3 getPosition(){
        return position;
    }
    
    void setPosition(vec3 pos){
        position = pos;
    }
    
    void setShooterModel(Shape model){
        shooterModel = model;
    }
    
    void setShooterTexture(GLuint texture){
        shooterTexture = texture;
    }
    
    void setProgram(shared_ptr<Program> prog){
        program = prog;
    }
    
};

class Skybox{
private:
    mat4 I;
    Shape skyboxModel;
    GLuint skyboxTexture;
    shared_ptr<Program> program;
    vec3 stars[100];
public:
    Skybox(){
        I = mat4(1);
        for (int i=0; i<100; i++){
            stars[i] = randVec3();
            stars[i].z = randRange(-3, -5);
        }
    }
    
    void render(){    }
    
    void setSkyboxModel(Shape model){
        skyboxModel = model;
    }
    
    void setSkyboxTexture(GLuint texture){
        skyboxTexture = texture;
    }
    
    void setProgram(shared_ptr<Program> prog){
        program = prog;
    }
};

class Asteroid{
private:
    mat4 I, M;
    Shape asteroidModel;
    GLuint asteroidTexture;
    
    queue<vec3> asteroidList;
    queue<vec4> angleList;
    queue<vec3> drnList;
    vec3 origin;
    shared_ptr<Program> program;
    
    
    bool collides(vec3 d, queue<vec3> w, float dist){
        vec3 q;
        while (w.size() > 0){
            q = w.front();
            w.pop();
            if (distance(d, q) < dist){
                return true;
            }
        }
        return false;
    }
    
public:
    float speedFactor;
    int asteroidCount;
    Asteroid(){
        origin = vec3(0, -0.5, 1);
        asteroidCount = 10;
        I = mat4(1);
        
        create(origin);
        speedFactor = 0.01f;
    }
    
    void create(vec3 ref){
        queue<vec3> e1;
        swap(asteroidList, e1);
        queue<vec3> e2;
        swap(drnList, e2);
        queue<vec4> e3;
        swap(angleList, e3);
        asteroidCount = 10;
        vec3 pos, drn;
        float fct;
        for (int i=0; i<asteroidCount; i++){
            pos = vec3(randRange(-8, 8), randRange(2, 10), randRange(-5, -12));
            asteroidList.push(pos);
            vec3 a = randVec3();
            vec4 ang = vec4(a.x, a.y, a.z, (float) randRange(20, 30)/100.0f);
            angleList.push(ang);
            drn = vec3(pos.x - ref.x, pos.y - ref.y, pos.z - ref.z);
            fct = randRange(5, 9);
            drn.x /= fct;
            drn.y /= fct;
            drn.z /= fct;
            drnList.push(drn);
        }
    }
    
    void setAsteroidModel(Shape model){
        asteroidModel = model;
    }
    
    void setAsteroidTexture(GLuint texture){
        asteroidTexture = texture;
    }
    
    void setProgram(shared_ptr<Program> prog){
        program = prog;
    }
    
    bool render(queue<vec3> ref, vec3 o){
        bool coll = false;
        vec3 pos, shift;
        vec4 angle;
        program->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroidTexture);
        for (int i=0; i<asteroidCount; i++){
            pos = asteroidList.front();
            angle = angleList.front();
            shift = drnList.front();
            asteroidList.pop();
            angleList.pop();
            drnList.pop();
            float s = angle.w;
            M = translate(I, pos) *
                rotate(I, angle.x, vec3(1, 0, 0)) *
                rotate(I, angle.y, vec3(0, 1, 0)) *
                rotate(I, angle.z, vec3(0, 0, 1)) *
                scale(I, vec3(s, s, s));
            glUniformMatrix4fv(program->getUniform("M"), 1,
                               GL_FALSE, &M[0][0]);
            asteroidModel.draw(program);
            pos.x -= shift.x * speedFactor;
            pos.y -= shift.y * speedFactor;
            pos.z -= shift.z * speedFactor;
            vec3 a = randShift();
            angle.x += a.x;
            queue<vec3> w(ref);
            if (!coll){
                coll = (distance(pos, o) < 0.45f);
            }
            if (collides(pos, w, 0.5f) or abs(pos.z) > 10){
                asteroidCount--;
            }else{
                asteroidList.push(pos);
                angleList.push(angle);
                drnList.push(shift);
            }
        }
        program->unbind();
        return coll;
    }
};


class Lasers{
    queue<vec3> laserPos;
    queue<vec3> laserDrn;
    shared_ptr<Program> program;
    int laserCount;
    Shape laserModel;
    GLuint laserTexture;
    mat4 I;

public:
    Lasers(){
        laserCount = 0;
        I = mat4(1);
    }
    
    queue<vec3> getPositions(){
        return laserPos;
    }
    
    void setLaserModel(Shape model){
        laserModel = model;
    }
    
    void setLaserTexture(GLuint texture){
        laserTexture = texture;
    }
    
    void setProgram(shared_ptr<Program> prog){
        program = prog;
    }
    
    void render(){
        vec3 pos, drn;
        mat4 M;
        program->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, laserTexture);
        for (int i=0; i<laserCount; i++){
            pos = laserPos.front();
            drn = laserDrn.front();
            laserPos.pop();
            laserDrn.pop();
            
            M = translate(I, pos) *
                //rotate(I, drn.x, vec3(0, 0, 1)) *
                //rotate(I, drn.y, vec3(1, 0, 0)) *
                //rotate(I, drn.z, vec3(0, 0, 1)) *
                scale(I, vec3(0.025f, 0.025f, 0.025f));
            glUniformMatrix4fv(program->getUniform("M"), 1,
                               GL_FALSE, &M[0][0]);
            
            laserModel.draw(program);
            
            pos += drn/10.0f;
            
            laserPos.push(pos);
            laserDrn.push(drn);
        }
        program->unbind();
    }
    
    void spawn(vec3 pos, vec3 drn){
        laserDrn.push(vec3(-drn.x, drn.y, drn.z));
        laserPos.push(pos);
        laserCount += 1;
    }
};


class GameState{
private:
    KeyboardCache kbd;
    double posX, posY;
    vec3 angle = vec3(0, 0, 0);
    bool gameOver;
    int ref;
    int startTime;
    
    void updateShooterPos(){
        vec3 shift = vec3(0, 0, 0);
        if (kbd.left){
            shift.x -= 0.01f;
        }
        if (kbd.right){
            shift.x += 0.01f;
        }
        if (kbd.up){
            shift.z -= 0.01f;
        }
        if (kbd.down){
            shift.z += 0.01f;
        }
        playerShooter.move(shift);
    }
public:
    Skybox sb;
    ShooterState playerShooter;
    Asteroid asteroid;
    Lasers ls;
    
    GameState(){
        gameOver = false;
        ref = time(NULL) + 3;
        startTime = -1;
    }
    
    void render(){
        if (time(NULL) < ref) return;
        if (startTime == -1) {
            startTime = time(NULL);
            asteroid.create(playerShooter.getPosition());
        }
        vec3 pos = playerShooter.getPosition();
        float x, y, dx, dy;
        dx = pos.x + WIDTH/2 - posX;
        dy = posY - pos.y;
        x = asin(dx/(sqrt(pow(dx, 2) + pow(dy, 2))));
        if (x > -PI/2 and x < PI/2){
            angle.x = x;
        }
        dy = HEIGHT - (887 - posY);
        y = PI/2 - (dy/(HEIGHT)) * PI/2;
        if (y > 0 and y < PI/2){
            angle.y = y;
        }
        sb.render();
        playerShooter.render(angle);
        gameOver = asteroid.render(ls.getPositions(),
                                    playerShooter.getPosition());
        ls.render();
        if (asteroid.asteroidCount == 0){
            asteroid.create(playerShooter.getPosition());
        }
        if (gameOver){
            cout << "GAME OVER!\n";
            cout << "You lasted " << time(NULL) - startTime << "s!\n";
            gameOver = false;
            ref = time(NULL) + 3;
            startTime = -1;
        }
    }
    
    void update(){
        glfwGetCursorPos(activeWindow, &posX, &posY);
        updateShooterPos();
    }
    
    void setMousePosition(double X, double Y){
        posX = X;
        posY = Y;
    }
    
    void setKeyEvent(int key, int value){
        switch (key) {
            case GLFW_KEY_A:
                kbd.left = value;
                break;
            case GLFW_KEY_D:
                kbd.right = value;
                break;
            case GLFW_KEY_W:
                kbd.up = value;
                break;
            case GLFW_KEY_S:
                kbd.down = value;
                break;
        }
    }
    
    void onMousePress(double pX, double pY){
        vec3 pos = playerShooter.getPosition();
        ls.spawn(pos, angle);
    }
};


Camera mycam;
GameState game;

class Application : public EventCallbacks
{

public:
    WindowManager * windowManager = nullptr;
    
    GLuint planeTexture;
    GLuint laserTexture;
    GLuint skyboxTexture;
    GLuint asteroidTexture;

	std::shared_ptr<Program> prog;
    std::shared_ptr<Program> prog1;
    std::shared_ptr<Program> prog2;
    std::shared_ptr<Program> prog3;
    std::shared_ptr<Program> prog4;
    
    Shape planeObj;
    Shape laserObj;
    Shape skyboxObj;
    Shape asteroidObj;

	GLuint VertexArrayID;
    GLuint VertexBufferID;
    GLuint VertexColorIDBox;
    GLuint IndexBufferIDBox;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
        game.setKeyEvent(key, action);
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		if (action == GLFW_PRESS)
		{
            glfwGetCursorPos(window, &posX, &posY);
            game.onMousePress(posX, posY);
		}
	}

	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	void initGeom()
	{
        int w, h, c;
        char fl[128];
        unsigned char *data;
        
        string planeTexPath = resourceDirectory + "/my_tex.jpeg";
        string skyboxTexPath = resourceDirectory + "/space_tex.jpeg";
        string asteroidTexPath = resourceDirectory + "/asteroid_tex.jpeg";
        string laserTexPath = resourceDirectory + "/red_tex.png";
        
        skyboxObj.loadMesh(resourceDirectory + "/sphere.obj");
        skyboxObj.resize();
        skyboxObj.init();
        
        laserObj.loadMesh(resourceDirectory + "/sphere.obj");
        laserObj.resize();
        laserObj.init();
        
        planeObj.loadMesh(resourceDirectory + "/FA18.obj");
        planeObj.resize();
        planeObj.init();
        
        asteroidObj.loadMesh(resourceDirectory + "/asteroid.obj");
        asteroidObj.resize();
        asteroidObj.init();
        
        strcpy(fl, skyboxTexPath.c_str());
        data = stbi_load(fl, &w, &h, &c, 4);
        glGenTextures(1, &skyboxTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint T0L = glGetUniformLocation(prog1->pid, "tex");
        glUseProgram(prog1->pid);
        glUniform1i(T0L, 0);
        
        strcpy(fl, planeTexPath.c_str());
        data = stbi_load(fl, &w, &h, &c, 4);
        glGenTextures(1, &planeTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint T1L = glGetUniformLocation(prog2->pid, "tex");
        glUseProgram(prog2->pid);
        glUniform1i(T1L, 0);
        
        strcpy(fl, laserTexPath.c_str());
        data = stbi_load(fl, &w, &h, &c, 4);
        glGenTextures(1, &laserTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, laserTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint T2L = glGetUniformLocation(prog3->pid, "tex");
        glUseProgram(prog3->pid);
        glUniform1i(T2L, 0);

        
        strcpy(fl, asteroidTexPath.c_str());
        data = stbi_load(fl, &w, &h, &c, 4);
        glGenTextures(1, &asteroidTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroidTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint T3L = glGetUniformLocation(prog4->pid, "tex");
        glUseProgram(prog4->pid);
        glUniform1i(T3L, 0);

        
        glUseProgram(prog1->pid);
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        
        glGenBuffers(1, &VertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        
        GLfloat cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,
            1.0, -1.0,  1.0,
            1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,
            // back
            -1.0, -1.0, -1.0,
            1.0, -1.0, -1.0,
            1.0,  1.0, -1.0,
            -1.0,  1.0, -1.0
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        
        GLfloat cube_colors[] = {
            // front colors
            1.0, 0.0, 0.5,
            1.0, 0.0, 0.5,
            1.0, 0.0, 0.5,
            1.0, 0.0, 0.5,
            // back colors
            0.5, 0.5, 0.0,
            0.5, 0.5, 0.0,
            0.5, 0.5, 0.0,
            0.5, 0.5, 0.0,
        };
        glGenBuffers(1, &VertexColorIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &IndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
        GLushort cube_elements[] = {
            // front
            0, 1, 2,
            2, 3, 0,
            // top
            1, 5, 6,
            6, 2, 1,
            // back
            7, 6, 5,
            5, 4, 7,
            // bottom
            4, 0, 3,
            3, 7, 4,
            // left
            4, 5, 1,
            1, 0, 4,
            // right
            3, 2, 6,
            6, 7, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        
        game.sb.setSkyboxModel(skyboxObj);
        game.sb.setSkyboxTexture(skyboxTexture);

        game.playerShooter.setShooterModel(planeObj);
        game.playerShooter.setShooterTexture(planeTexture);
        
        game.asteroid.setAsteroidModel(asteroidObj);
        game.asteroid.setAsteroidTexture(asteroidTexture);
        
        game.ls.setLaserModel(laserObj);
        game.ls.setLaserTexture(laserTexture);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		glClearColor(0.1f, 0.09f, 0.09f, 1.0f);
        
		glEnable(GL_DEPTH_TEST);

		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
        prog->addAttribute("vertColor");
        
        prog1 = std::make_shared<Program>();
        prog1->setVerbose(true);
        prog1->setShaderNames(resourceDirectory + "/shader_vertex2.glsl", resourceDirectory + "/shader_fragment2.glsl");
        prog1->init();
        prog1->addUniform("P");
        prog1->addUniform("V");
        prog1->addUniform("M");
        prog1->addUniform("alpha");
        prog1->addUniform("lightPos");
        prog1->addAttribute("vertPos");
        prog1->addAttribute("vertNor");
        prog1->addAttribute("vertTex");
        
        prog2 = std::make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/shader_vertex2.glsl", resourceDirectory + "/shader_fragment2.glsl");
        prog2->init();
        prog2->addUniform("P");
        prog2->addUniform("V");
        prog2->addUniform("M");
        prog2->addUniform("alpha");
        prog2->addUniform("lightPos");
        prog2->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
        prog2->addAttribute("vertTex");
        
        prog3 = std::make_shared<Program>();
        prog3->setVerbose(true);
        prog3->setShaderNames(resourceDirectory + "/shader_vertex2.glsl", resourceDirectory + "/shader_fragment2.glsl");
        prog3->init();
        prog3->addUniform("P");
        prog3->addUniform("V");
        prog3->addUniform("M");
        prog3->addUniform("alpha");
        prog3->addUniform("lightPos");
        prog3->addAttribute("vertPos");
        prog3->addAttribute("vertNor");
        prog3->addAttribute("vertTex");
        
        prog4 = std::make_shared<Program>();
        prog4->setVerbose(true);
        prog4->setShaderNames(resourceDirectory + "/shader_vertex2.glsl", resourceDirectory + "/shader_fragment2.glsl");
        prog4->init();
        prog4->addUniform("P");
        prog4->addUniform("V");
        prog4->addUniform("M");
        prog4->addUniform("alpha");
        prog4->addUniform("lightPos");
        prog4->addAttribute("vertPos");
        prog4->addAttribute("vertNor");
        prog4->addAttribute("vertTex");
        
        game.sb.setProgram(prog1);
        game.playerShooter.setProgram(prog2);
        game.ls.setProgram(prog3);
        game.asteroid.setProgram(prog4);
        
        game.playerShooter.setPosition(vec3(0, -0.5, 1));
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		
		
		//float aspect = width/(float)height;
        
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable(GL_DEPTH_TEST);

		mat4 P, V, M, I; //Perspective, View, Model and Identity matrix
        I = mat4(1);
        P = getPerspectiveMatrix(width, height);
        V = getViewMatrix(0); //
        //V = mycam.process(frametime);
		M = I;
        
        prog1->bind();
        
        glBindVertexArray(VertexArrayID);
        M = mat4(1);
        
        glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog1->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog1->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform1f(prog1->getUniform("alpha"), 1.0f);
        glUniform3f(prog1->getUniform("lightPos"), 0.0f, 0.0f, 3.0f);
        prog1->unbind();
        
		prog2->bind();
        
        glBindVertexArray(VertexArrayID);
        M = mat4(1);
        
        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform1f(prog2->getUniform("alpha"), 1.0f);
        glUniform3f(prog2->getUniform("lightPos"), 0.0f, 0.0f, 3.0f);
        prog2->unbind();
        
        prog3->bind();
        glBindVertexArray(VertexArrayID);
        M = mat4(1);
        
        glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog3->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog3->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform1f(prog3->getUniform("alpha"), 1.0f);
        glUniform3f(prog3->getUniform("lightPos"), 0.0f, 0.0f, 3.0f);

        prog3->unbind();
        
        prog4->bind();
        glBindVertexArray(VertexArrayID);
        M = mat4(1);
        
        glUniformMatrix4fv(prog4->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog4->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog4->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform1f(prog4->getUniform("alpha"), 1.0f);
        glUniform3f(prog4->getUniform("lightPos"), 0.0f, 0.0f, 3.0f);

        prog4->unbind();
        
        game.render();
        game.update();
        
        glBindVertexArray(0);
        
        
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	if (argc >= 2)
	{
		resourceDirectory = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDirectory);
	application->initGeom();
    activeWindow = windowManager->getHandle();
	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
