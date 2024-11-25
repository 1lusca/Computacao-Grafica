
#include <iostream>
#include <string>
#include <assert.h>

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>



//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//STB_IMAGE
#include <stb_image/stb_image.h>

//Classe gerenciadora de shaders
#include "Shader.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupGeometry();
int loadSimpleOBJ(string filePATH, int &nVertices);
GLuint loadTexture(string filePath, int &width, int &height);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 2560, HEIGHT = 1600;

// rotacao
bool rotateX=false, rotateY=false, rotateZ=false;

// translation
bool translateX = false, translateY = false, translateZ = false;

float auxTransAX = 1.0f, auxTransAY = 0.0f, auxTransAZ = 0.0f;
float auxTransBX = -1.0f, auxTransBY = 0.0f, auxTransBZ = 0.0f;

// scale
bool scaleX = false, scaleY = false, scaleZ = false;
float auxScaleX = 1.0f, auxScaleY = 1.0f, auxScaleZ = 1.0f;

// parada
bool stop = false;

//Variáveis globais da câmera
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

struct Object
{
	GLuint VAO; //Índice do buffer de geometria
	GLuint texID; //Identificador da textura carregada
	int nVertices; //nro de vértices
	glm::mat4 model; //matriz de transformações do objeto
	float ka, kd, ks; //coeficientes de iluminação - material do objeto

};



// Fun��o MAIN
int main()
{
	// Inicializa��o da GLFW
	glfwInit();

	//Muita aten��o aqui: alguns ambientes n�o aceitam essas configura��es
	//Voc� deve adaptar para a vers�o do OpenGL suportada por sua placa
	//Sugest�o: comente essas linhas de c�digo para desobrir a vers�o e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Trabalho GB", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	Shader shader("phong.vs","phong.fs");





	Object obj;
    Object obj2;

	obj.VAO = loadSimpleOBJ("assets/obj/SuzanneHigh.obj",obj.nVertices);
    int texWidth,texHeight;
	obj.texID = loadTexture("assets/textures/Suzanne.png",texWidth,texHeight);


    obj2.VAO = loadSimpleOBJ("assets/obj/planeta.obj", obj2.nVertices);
    int texWidth2, texHeight2;
    obj2.texID = loadTexture("assets/textures/Terra.jpg", texWidth2, texHeight2);




	glUseProgram(shader.ID);

    //Matriz de modelo
	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //Matriz de view
	glm::mat4 view = glm::lookAt(cameraPos,glm::vec3(0.0f,0.0f,0.0f),cameraUp);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	//Matriz de projeção
	//glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(39.6f),(float)WIDTH/HEIGHT,0.1f,100.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //Buffer de textura no shader
	glUniform1i(glGetUniformLocation(shader.ID, "texBuffer"), 0);

	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);

	//Propriedades da superfície
	shader.setFloat("ka",0.2);
	shader.setFloat("ks", 0.5);
	shader.setFloat("kd", 0.5);
	shader.setFloat("q", 10.0);

	//Propriedades da fonte de luz
	shader.setVec3("lightPos",-2.0, 10.0, 3.0);
	shader.setVec3("lightColor",1.0, 1.0, 1.0);


    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        float angle = (GLfloat)glfwGetTime();

        obj.model = glm::mat4(1.0f);

        // rotation obj1
        if (rotateX)
            obj.model = glm::rotate(obj.model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotateY)
            obj.model = glm::rotate(obj.model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotateZ)
            obj.model = glm::rotate(obj.model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        // translation e scale obj1
        obj.model = glm::translate(obj.model, glm::vec3(auxTransAX, auxTransAY, auxTransAZ));
        obj.model = glm::scale(obj.model, glm::vec3(auxScaleX, auxScaleY, auxScaleZ));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));

        glBindVertexArray(obj.VAO);
        glBindTexture(GL_TEXTURE_2D, obj.texID);
        glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);

        obj2.model = glm::mat4(1.0f);

        // rotation obj2
        if (rotateX)
            obj2.model = glm::rotate(obj2.model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        else if (rotateY)
            obj2.model = glm::rotate(obj2.model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        else if (rotateZ)
            obj2.model = glm::rotate(obj2.model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        // translation e scale obj2
        obj2.model = glm::translate(obj2.model, glm::vec3(auxTransBX, auxTransBY, auxTransBZ));
        obj2.model = glm::scale(obj2.model, glm::vec3(auxScaleX, auxScaleY, auxScaleZ));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj2.model));

        glBindVertexArray(obj2.VAO);
        glBindTexture(GL_TEXTURE_2D, obj2.texID);
        glDrawArrays(GL_TRIANGLES, 0, obj2.nVertices);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

        shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

        glfwSwapBuffers(window);
    }
    
    // Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &obj.VAO);
    glDeleteVertexArrays(1, &obj2.VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// rotation
	// z = eixo Z
	// x = eixo X
	// c = eixo Y
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	// translation
	// i = eixo X positivo	|	j = eixo X negativo
	// o = eixo Y positivo	|	k = eixo Y negativo
	// p = eixo Z positivo	|	l = eixo Z negativo
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		translateX = true;
		translateY = false;
		translateZ = false;
		auxTransAX += 0.1f;
        auxTransBX -= 0.1f;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		translateX = true;
		translateY = false;
		translateZ = false;
		auxTransAX -= 0.1f;
        auxTransBX += 0.1f;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		translateX = false;
		translateY = true;
		translateZ = false;
		auxTransAY += 0.1f;
        auxTransBY -= 0.1f;
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		translateX = false;
		translateY = true;
		translateZ = false;
		auxTransAY -= 0.1f;
        auxTransBY += 0.1f;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		translateX = false;
		translateY = false;
		translateZ = true;
		auxTransAZ += 0.1f;
        auxTransBZ -= 0.1f;
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		translateX = false;
		translateY = false;
		translateZ = true;
		auxTransAZ -= 0.1f;
        auxTransBZ += 0.1f;
	}

	// scale
	// t = eixo X positivo	| f = eixo X negativo
	// y = eixo Y positivo	| g = eixo Y negativo
	// u = eixo Z positivo	| h = eixo Z negativo
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        scaleX = true;
        scaleY = false;
        scaleZ = false;
        auxScaleX += 0.1f;  // Aumenta a escala em X
    }
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        scaleX = true;
        scaleY = false;
        scaleZ = false;
        auxScaleX -= 0.1f;  // Aumenta a escala em X
    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        scaleX = false;
        scaleY = true;
        scaleZ = false;
        auxScaleY += 0.1f;  // Aumenta a escala em Y
    }
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        scaleX = false;
        scaleY = true;
        scaleZ = false;
        auxScaleY -= 0.1f;  // Aumenta a escala em Y
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        scaleX = false;
        scaleY = false;
        scaleZ = true;
        auxScaleZ += 0.1f;  // Aumenta a escala em Z
    }
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        scaleX = false;
        scaleY = false;
        scaleZ = true;
        auxScaleZ -= 0.1f;  // Aumenta a escala em Z
    }

	// camera
	float cameraSpeed = 0.05f;
	if ((key == GLFW_KEY_W || key == GLFW_KEY_UP) && action == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && action == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && action == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && action == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	// critério de parada = q
	if (key == GLFW_KEY_Q) {
		rotateX = false;
        rotateY = false;
        rotateZ = false;
        translateX = false;
        translateY = false;
        translateZ = false;
        scaleX = false;
        scaleY = false;
        scaleZ = false;
        auxTransAX = 1.0f;
        auxTransAY = 0.0f;
        auxTransAZ = 0.0f;
        auxTransBX = -1.0f;
        auxTransBY = 0.0f;
        auxTransBZ = 0.0f;
        auxScaleX = 1.0f;
        auxScaleY = 1.0f;
        auxScaleZ = 1.0f;
	}

}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry() {

	GLfloat vertices[] = {

		//x    	y    	z    	r    	g    	b
		-0.5,	-0.5,	-0.5,	1.0,	1.0,	0.0,
		-0.5,	-0.5,	0.5,	0.0,	1.0,	1.0,
		0.5,	-0.5,	-0.5,	1.0,	0.0,	1.0,

		-0.5,	-0.5,	0.5,	1.0,	1.0,	0.0,
		0.5,	-0.5,	0.5,	0.0,	1.0,	1.0,
		0.5,	-0.5,	-0.5,	1.0,	0.0,	1.0,

		-0.5,	-0.5,	-0.5,	1.0,	1.0,	0.0,
		0.0,	0.5,	0.0,	1.0,	1.0,	0.0,
		0.5,	-0.5,	-0.5,	1.0,	1.0,	0.0,

		-0.5,	-0.5,	-0.5,	1.0,	0.0,	1.0,
		0.0,	0.5,	0.0,	1.0,	0.0,	1.0,
		-0.5,	-0.5,	0.5,	1.0,	0.0,	1.0,

		-0.5,	-0.5,	0.5,	1.0,	1.0,	0.0,
		0.0,	0.5,	0.0,	1.0,	1.0,	0.0,
		0.5,	-0.5,	0.5,	1.0,	1.0,	0.0,

		0.5,	-0.5,	0.5,	0.0,	1.0,	1.0,
		0.0,	0.5,	0.0,	0.0,	1.0,	1.0,
		0.5,	-0.5,	-0.5,	0.0,	1.0,	1.0,
	};

		GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}


int loadSimpleOBJ(string filePath, int &nVertices)
{
	vector <glm::vec3> vertices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vBuffer;

	glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);

	ifstream arqEntrada;

	arqEntrada.open(filePath.c_str());
	if (arqEntrada.is_open())
	{
		//Fazer o parsing
		string line;
		while (!arqEntrada.eof())
		{
			getline(arqEntrada,line);
			istringstream ssline(line);
			string word;
			ssline >> word;
			if (word == "v")
			{
				glm::vec3 vertice;
				ssline >> vertice.x >> vertice.y >> vertice.z;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				vertices.push_back(vertice);

			}
			if (word == "vt")
			{
				glm::vec2 vt;
				ssline >> vt.s >> vt.t;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				texCoords.push_back(vt);

			}
			if (word == "vn")
			{
				glm::vec3 normal;
				ssline >> normal.x >> normal.y >> normal.z;
				//cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
				normals.push_back(normal);

			}
			else if (word == "f")
			{
				while (ssline >> word) 
				{
					int vi, ti, ni;
					istringstream ss(word);
    				std::string index;

    				// Pega o índice do vértice
    				std::getline(ss, index, '/');
    				vi = std::stoi(index) - 1;  // Ajusta para índice 0

    				// Pega o índice da coordenada de textura
    				std::getline(ss, index, '/');
    				ti = std::stoi(index) - 1;

    				// Pega o índice da normal
    				std::getline(ss, index);
    				ni = std::stoi(index) - 1;

					//Recuperando os vértices do indice lido
					vBuffer.push_back(vertices[vi].x);
					vBuffer.push_back(vertices[vi].y);
					vBuffer.push_back(vertices[vi].z);
					
					//Atributo cor
					vBuffer.push_back(color.r);
					vBuffer.push_back(color.g);
					vBuffer.push_back(color.b);

					//Atributo coordenada de textura
					vBuffer.push_back(texCoords[ti].s);
					vBuffer.push_back(texCoords[ti].t);

					//Atributo vetor normal
					vBuffer.push_back(normals[ni].x);
					vBuffer.push_back(normals[ni].y);
					vBuffer.push_back(normals[ni].z);
					
        			
        			// Exibindo os índices para verificação
       				// std::cout << "v: " << vi << ", vt: " << ti << ", vn: " << ni << std::endl;
    			}
				
			}
		}

		arqEntrada.close();

		cout << "Gerando o buffer de geometria..." << endl;
		GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo coordenada de textura - s, t
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Atributo vetor normal - x, y, z
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8*sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	nVertices = vBuffer.size() / 11;
	return VAO;

	}
	else
	{
		cout << "Erro ao tentar ler o arquivo " << filePath << endl;
		return -1;
	}
}






GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}





