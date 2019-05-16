#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#include"GlobalVariable.h"
#include <utilies/shader.h>
#include "SpriteRenderer.h"
#include "ResourceManager.h"
class GameObject {
public:
	// Object state
	glm::vec3   position, size;
	float     rotation;
	bool   destroyed;
	// Render state
	std::string modelName;
	// Constructor(s)
	GameObject();
	GameObject(glm::vec3 pos, glm::vec3 size, float rotation, std::string modelName);
	// Draw sprite
	virtual void Draw(Shader& shader,bool isShadow);
};


#endif // !GAME_OBJECT_H