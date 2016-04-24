/*
 * main.cpp
 *
 *  Created on: 15/04/2016
 *      Author: josh
 */
#include <iostream>
#include <eigen3/Eigen/Eigen>
#include "Particle.h"
#include "ParticleManager.h"
#include "GLFW/glfw3.h"
#include "GL/glu.h"
#include "constants.h"

float delta_time = 1;
float current_time = 0;

static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//std::cout << "Key: " << (int)(key == GLFW_KEY_ESCAPE) << "\ni";
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        delta_time *= 1.5;
        std::cout << "Time Step: " << delta_time << std::endl;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        delta_time /= 1.5;
        std::cout << "Time Step: " << delta_time << std::endl;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        std::cout << "Current Time: " << (current_time/(3600*24)) << " Days" << std::endl;
    }
}

float rrang(double _min, double _max) {
	return (double)rand() * (_max - _min) / (double)RAND_MAX + _min;
}

float drawFloor() {
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_LINES);

	for(int x=-1000*1000*400; x<=1000*1000*400; x+=1000*1000*50) {
		glVertex3f(x,0,-1000*1000*400);
		glVertex3f(x,0,1000*1000*400);
	}
	for(int z=-1000*1000*400; z<=1000*1000*400; z+=1000*1000*50) {
		glVertex3f(-1000*1000*400,0,z);
		glVertex3f(1000*1000*400,0,z);
	}

	glEnd();

}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glEnable(GL_DEPTH_TEST);

	ParticleManager pManager = ParticleManager();
	/*
	for(unsigned int i=0; i<1000; i++) {
		Eigen::VectorXd location(3);
		location << rrang(-500, 500),rrang(-500, 500),rrang(-500, 500);
		Eigen::VectorXd velocity(3);
		if (location[1] > 0) {
			velocity << 0.0000001,0,0;//rrang(-0.001, 0.001),rrang(-0.001, 0.001),rrang(-0.001, 0.001);
		} else {
			velocity << -0.0000001,0,0;
		}
		pManager.addParticle(new Particle(location, velocity, 40.0f, 0.1f));
	}*/

	//Add Earth
	Eigen::VectorXd location(3);
	location << 0,0,0;
	Eigen::VectorXd velocity(3);
	velocity << 0,0,0;
	pManager.addParticle(new Particle(location, velocity, 5972370000000000000000000.0, 5514));

	//Add Moon
	Eigen::VectorXd locationM(3);
	//locationM << 362600000,0,0;
	locationM <<   405400000,0,0;
	Eigen::VectorXd velocityM(3);
	velocityM << 0,0,964.0;
	pManager.addParticle(new Particle(locationM, velocityM, 73420000000000000000000.0, 3344));

	//Offset moons momentum
	double momentum = pManager.particles[1]->mass * pManager.particles[1]->velocity[2];
	pManager.particles[0]->velocity[2] = -momentum / pManager.particles[0]->mass;

    while (!glfwWindowShouldClose(window)) {
    	//std::cout << delta_time << std::endl;
      	for(unsigned int i=0; i<pManager.particles.size(); i++) {
    		if (pManager.particles[i]->active) {
				std::vector<Particle*> neigbours = pManager.getParticles(pManager.particles[i]);
				for(unsigned int j=0; j<neigbours.size(); j++) {
					if (i != j && pManager.particles[j]->active) {
						pManager.particles[i]->interact(pManager.particles[j],delta_time);

					}
				}
    		}

		}

    	for(unsigned int i=0; i<pManager.particles.size(); i++) {
    		if(pManager.particles[i]->active) {
    			pManager.particles[i]->step(delta_time);
    		}
    	}

    	current_time += delta_time;

        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio*1000*500000, ratio*1000*500000, -1000*500000, 1000*500000, -1000*500000, 1000*500000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(10,10,10,0,0,0,0,1,0);

        GLUquadric *gluQuad = gluNewQuadric();



        for(unsigned int i=0; i<pManager.particles.size(); i++) {
        	Particle *p = pManager.particles[i];
        	if (p->active) {
				glPushMatrix();
					glTranslatef((float)p->location[0], (float)p->location[1], (float)p->location[2]);
					if (p->radius > 10) {
						glColor3f(1.f, 1.f, 1.f);
						gluSphere(gluQuad, p->radius, 10, 10);
					} else  {
						glColor3f(0.f, 1.f, 0.f);
						gluSphere(gluQuad, 10, 10, 10);
					}
				glPopMatrix();
        	}
        }

        /*
        glPointSize(2.0f);
        glColor3f(1.f, 1.f, 1.f);
        glBegin(GL_POINTS);

        for(unsigned int i=0; i<pManager.particles.size(); i++) {
        	Particle *p = pManager.particles[i];
        	glVertex3f((float)p->location[0], (float)p->location[1], (float)p->location[2]);

        }

        glEnd();
*/
        drawFloor();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}



