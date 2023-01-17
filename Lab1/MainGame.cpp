#include "MainGame.h"
#include "Camera.h"
#include <iostream>
#include <string>


MainGame::MainGame()
{
	_gameState = GameState::PLAY;
	Display* _gameDisplay = new Display(); //new display
}

MainGame::~MainGame()
{
}

void MainGame::run()
{
	initSystems(); 
	gameLoop();
}

void MainGame::initSystems()
{
	_gameDisplay.initDisplay(); 
	whistle = audioDevice.loadSound("..\\res\\whistle.wav");
	backGroundMusic = audioDevice.loadSound("..\\res\\background.wav");
	texture.load("..\\res\\bricks.jpg");
	rockMesh.loadModel("..\\res\\Rock1.obj");
	shipMesh.loadModel("..\\res\\R33.obj");
	missileMesh.loadModel("..\\res\\R33.obj");
	fogShader.init("..\\res\\fogShader.vert", "..\\res\\fogShader.frag"); //new shader
	toonShader.init("..\\res\\shaderToon.vert", "..\\res\\shaderToon.frag"); //new shader
	rimShader.init("..\\res\\shaderRim.vert", "..\\res\\shaderRim.frag");
	eMapping.init("..\\res\\shaderReflection.vert", "..\\res\\shaderReflection.frag");
	FBOShader.init("..\\res\\FBOShader.vert", "..\\res\\FBOShader.frag");

	initModels(asteroid);
	playAudio(backGroundMusic, glm::vec3(0,0,0));
	geoShader.initGeo();

	myCamera.initCamera(glm::vec3(0, 0, -50), 70.0f, (float)_gameDisplay.getWidth()/_gameDisplay.getHeight(), 0.01f, 1000.0f);

	thirdCamera.initCamera(glm::vec3(0, -10, 0), 70.0f, (float)_gameDisplay.getWidth() / _gameDisplay.getHeight(), 0.01f, 1000.0f);
	thirdCamera.setLook(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z));

	povCamera.initCamera(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z + 3.0f * deltaTime)
		, 70.0f, (float)_gameDisplay.getWidth() / _gameDisplay.getHeight(), 0.01f, 1000.0f);
	povCamera.setPos(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z + 3.0f * deltaTime));
	povCamera.setLook(glm::vec3(0, 20, 0));
	

	activeCamera = myCamera;

	generateFBO(_gameDisplay.getWidth(), _gameDisplay.getHeight());

	createScreenQuad();

	counter = 1.0f;

	

	vector<std::string> faces
	{
		"..\\res\\skybox\\right.png",
		"..\\res\\skybox\\left.png",
		"..\\res\\skybox\\top.png",
		"..\\res\\skybox\\bottom.png",
		"..\\res\\skybox\\front.png",
		"..\\res\\skybox\\back.png"
	};

	skybox.init(faces);
}

void MainGame::createScreenQuad()
{
	float quadVertices[] = { 
		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		 //positions   // texCoords
		//-1.0f,  1.0f,  0.0f, 1.0f,
		//-1.0f, -1.0f,  0.0f, 0.0f,
		// 1.0f, -1.0f,  1.0f, 0.0f,

		//-1.0f,  1.0f,  0.0f, 1.0f,
		// 1.0f, -1.0f,  1.0f, 0.0f,
		// 1.0f,  1.0f,  1.0f, 1.0f

		//// vertex attributes for a quad that fills the half of the screen
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f,  0.25f,  0.0f, 0.0f,
		-0.25f,  0.25f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		-0.25f,  0.25f,  1.0f, 0.0f,
		-0.25f,  1.0f,  1.0f, 1.0f
	};
	// cube VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //stride offset example
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

}


void MainGame::gameLoop()
{
	while (_gameState != GameState::EXIT)
	{
		takeInput();
		currentCamPos = activeCamera.getPos();
		drawGame();
		updateDelta();

		for (size_t i = 0; i < 20; i++)
		{

			collision(*asteroid[i].getTM().GetPos(),rockMesh.getSphereRadius(), shipMesh.getSpherePos(), shipMesh.getSphereRadius());
			/*if (collision(*asteroid[i].getTM().GetPos(), rockMesh.getSphereRadius(), shipMesh.getSpherePos(), shipMesh.getSphereRadius()))
				asteroid[i].setActive(false);*/
		}
		
	}
}

void MainGame::takeInput()
{
	SDL_Event evnt;

	while (SDL_PollEvent(&evnt)) //get and process events
	{
		switch (evnt.type)
		{
		case SDL_MOUSEWHEEL:
			activeCamera.MoveBack(evnt.wheel.y);
			break;
		default:
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (evnt.button.button)
			{
			case SDL_BUTTON_LEFT:
				fireShots(shipMissiles);
				break;
			case SDL_BUTTON_RIGHT:
				break;
			case SDL_BUTTON_MIDDLE:
				break;
			default:
				break;
			}
		case SDL_KEYDOWN:
			switch (evnt.key.keysym.sym)
			{
		
			case SDLK_w:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y + 10.0f * deltaTime, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y/*hereeeee*/, ship.getTM().GetRot()->z)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				myCamera.MoveUp(10.0f * deltaTime);
				thirdCamera.MoveUp(10.0f * deltaTime);
				povCamera.MoveUp(10.0f * deltaTime);
				activeCamera.MoveUp(10.0f * deltaTime);
				break;
			case SDLK_a:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x + 10.0f * deltaTime, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				myCamera.MoveRight(10.0f * deltaTime);
				thirdCamera.MoveRight(10.0f * deltaTime);
				povCamera.MoveRight(10.0f * deltaTime);
				activeCamera.MoveRight(10.0f * deltaTime);
				break;
			case SDLK_s:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y - 10.0f * deltaTime, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				myCamera.MoveDown(10.0f * deltaTime);
				thirdCamera.MoveDown(10.0f * deltaTime);
				povCamera.MoveDown(10.0f * deltaTime);
				activeCamera.MoveDown(10.0f * deltaTime);
				break;
			case SDLK_d:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x - 10.0f * deltaTime, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				myCamera.MoveLeft(10.0f * deltaTime);
				thirdCamera.MoveLeft(10.0f * deltaTime);
				povCamera.MoveLeft(10.0f * deltaTime);
				activeCamera.MoveLeft(10.0f * deltaTime);
				break;
			case SDLK_e:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z + 1.0f * deltaTime)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				break;
			case SDLK_q:
				ship.transformPositions(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y, ship.getTM().GetPos()->z)
					, glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z - 1.0f * deltaTime)
					, glm::vec3(ship.getTM().GetScale()->x, ship.getTM().GetScale()->y, ship.getTM().GetScale()->z));
				break;

			case SDLK_1:
				activeCamera = myCamera;
				
				break;
			case SDLK_2:
				activeCamera = povCamera;
				
				break;
			case SDLK_3:
				activeCamera = thirdCamera;
				break;
			case SDLK_SPACE:
				if (look)
					look = false;
				else
					look = true;
				break;
			default:
				break;
			case SDL_QUIT:
				_gameState = GameState::EXIT;
				break;
			}
		}
		
	}
}

void MainGame::initModels(GameObject*& asteroid)
{
	for (int i = 0; i < 20; ++i)
	{
		float rX = -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));
		float rY= -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));
		float rZ = -1.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0 - -1.0)));

		asteroid[i].transformPositions(glm::vec3(2.0 * i * rX, 2.0 * i * rY, 2.0 * i * rZ), glm::vec3(rX, rY, rZ), glm::vec3(0.3, 0.3, 0.3));
		asteroid[i].update(&rockMesh);		
	}

	ship.transformPositions(glm::vec3(0.0, 0.0, 2.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.2,0.2,0.2));
	
	for (int i = 0; i < 20; ++i)
	{
		missiles[i].setActive(0);
	}
}

void MainGame::drawRocks()
{
	texture.Bind(0);
	eMapping.Bind();
	linkEmapping();

	glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, texture.getID());

	glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getID());

	for (int i = 0; i < 20; ++i)
	{
		asteroid[i].transformPositions(glm::vec3(*asteroid[i].getTM().GetPos()), glm::vec3(asteroid[i].getTM().GetRot()->x + deltaTime, asteroid[i].getTM().GetRot()->y + deltaTime, asteroid[i].getTM().GetRot()->z + deltaTime), glm::vec3(0.1, 0.1, 0.1));
		asteroid[i].draw(&rockMesh);
		asteroid[i].update(&rockMesh);
		eMapping.Update(asteroid[i].getTM(), activeCamera);
	}
}

void MainGame::drawMissiles()
{
	texture.Bind(0);
	rimShader.Bind();
	linkRimLighting();

	for (int i = 0; i < 20; ++i)
	{
		if (missiles[i].getActive())
		{
			missiles[i].transformPositions(glm::vec3(missiles[i].getTM().GetPos()->x, missiles[i].getTM().GetPos()->y + 5.0f * deltaTime, missiles[i].getTM().GetPos()->z), 
				glm::vec3(missiles[i].getTM().GetRot()->x, missiles[i].getTM().GetRot()->y, missiles[i].getTM().GetRot()->z), 
				glm::vec3(0.1f, 0.1f, 0.1f));
			missiles[i].draw(&missileMesh);
			missiles[i].update(&missileMesh);
			eMapping.Update(missiles[i].getTM(), activeCamera); /*added*/
			rimShader.Update(missiles[i].getTM(), activeCamera);
		}
	}
}

void MainGame::fireShots(int i) 
{
	missiles[i].transformPositions(glm::vec3(ship.getTM().GetPos()->x, ship.getTM().GetPos()->y + 3.5f, ship.getTM().GetPos()->z),
		glm::vec3(ship.getTM().GetRot()->x, ship.getTM().GetRot()->y, ship.getTM().GetRot()->z),
		glm::vec3(0.1f, 0.1f, 0.1f));
	missiles[i].getActive();
	missiles[i].setActive(true);
	i += 1;
	shipMissiles = i;
	playAudio(whistle, glm::vec3(0, 0, 0));
}

void MainGame::drawShip()
{
	toonShader.Bind();
	linkToon();

	ship.draw(&shipMesh);
	ship.update(&shipMesh);
	toonShader.Update(ship.getTM(), activeCamera);
	
}


void MainGame::drawSkyBox()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);

	counter = counter + 0.02f;

	skybox.draw(&activeCamera);

	activeCamera.setPos(currentCamPos);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnd();
}


bool MainGame::collision(glm::vec3 m1Pos, float m1Rad, glm::vec3 m2Pos, float m2Rad)
{
	float distance = ((m2Pos.x - m1Pos.x)*(m2Pos.x - m1Pos.x) + (m2Pos.y - m1Pos.y)*(m2Pos.y - m1Pos.y) + (m2Pos.z - m1Pos.z)*(m2Pos.z - m1Pos.z));

	if (distance*distance < (m1Rad + m2Rad))
	{
		printf("HITTTTTTT");
		return true;
	}
	else
	{
		return false;
	}
}

void MainGame::playAudio(unsigned int Source, glm::vec3 pos)
{
	
	ALint state; 
	alGetSourcei(Source, AL_SOURCE_STATE, &state);
	
	//Possible values of state
		AL_INITIAL;
		AL_STOPPED;
		AL_PLAYING;
		AL_PAUSED;
	
	if (AL_PLAYING != state)
	{
		audioDevice.playSound(Source, pos);
	}
}

void MainGame::linkFogShader()
{
	fogShader.setFloat("maxDist", 20.0f);
	fogShader.setFloat("minDist", 0.0f);
	fogShader.setVec3("fogColor", glm::vec3(0.0f, 0.0f, 0.0f));
}

void MainGame::linkToon()
{
	toonShader.setVec3("lightDir", glm::vec3(0.5f, 0.5f, 0.5f));
}

void MainGame::linkGeo()
{
	float randX = ((float)rand() / (RAND_MAX));
	float randY = ((float)rand() / (RAND_MAX));
	float randZ = ((float)rand() / (RAND_MAX));
	// Frag: uniform float randColourX; uniform float randColourY; uniform float randColourZ;
	geoShader.setFloat("randColourX", randX);
	geoShader.setFloat("randColourY", randY);
	geoShader.setFloat("randColourZ", randZ);
	// Geom: uniform float time;
	geoShader.setFloat("time", counter);
}

void MainGame::linkRimLighting()
{
	glm::vec3 camDir;
	camDir = shipMesh.getSpherePos() - activeCamera.getPos();
	camDir = glm::normalize(camDir);
	rimShader.setMat4("u_pm", activeCamera.getProjection());
	rimShader.setMat4("u_vm", activeCamera.getView());
	rimShader.setMat4("model", transform.GetModel());
	rimShader.setMat4("view", activeCamera.getView());
	rimShader.setVec3("lightDir", glm::vec3(0.5f, 0.5f, 0.5f));
}

void MainGame::linkEmapping()
{
	eMapping.setMat4("model", asteroid[0].getModel());
}

void MainGame::updateDelta()
{
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	deltaTime = (float)((NOW - LAST) / (float)SDL_GetPerformanceFrequency());
}

void MainGame::bindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
}

void MainGame::unbindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void MainGame::generateFBO(float w, float h)
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// create a colorbuffer for attachment texture
	glGenTextures(1, &CBO);
	glBindTexture(GL_TEXTURE_2D, CBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CBO, 0);

	// create a renderbuffer object for depth and stencil attachment 
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h); // use a single renderbuffer object for both a depth AND stencil buffer.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO); // now actually attach it


// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		cout << "FRAMEBUFFER:: Framebuffer is complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MainGame::renderFBO()
{
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	FBOShader.Bind();
	FBOShader.setFloat("time", counter);
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, CBO);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void MainGame::drawGame()
{
	_gameDisplay.clearDisplay(0.8f, 0.4f, 0.4f, 1.0f); //sets our background colour	

	bindFBO();

	drawRocks();
	drawShip();
	drawSkyBox();
	drawMissiles();

	unbindFBO();

	renderFBO();

	glEnable(GL_DEPTH_TEST);

	drawRocks();
	drawShip();
	drawSkyBox();
	drawMissiles();
	
	_gameDisplay.swapBuffer();		

} 

