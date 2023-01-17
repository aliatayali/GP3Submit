#pragma once

#include "Mesh.h"
#include "transform.h"
#include "Shader.h"


class GameObject
{

public:
	GameObject();
	void transformPositions(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
	void update(Mesh* mesh);
	void draw(Mesh* mesh);
	void setActive(bool set);

	bool getActive() { return active; }
	glm::mat4 getModel() { return tObject.GetModel(); }
	Transform getTM() { return tObject; }


private:
	Transform tObject;
	Camera cameraIn;
	Shader shaderIn;
	
	bool active;
	float counter = 0.0f;

	float deltaTime;
};


