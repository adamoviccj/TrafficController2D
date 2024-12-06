// Autor: Jelena Adamovic, SV 6/2021
// Opis: Kontroler saobracaja u 2D

#define _CRT_SECURE_NO_WARNINGS

// C++ biblioteke za rad sa ispisima, fajlovima, nitima itd.
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

// Ukljucivanje biblioteka za rad sa OpenGL-om
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Ukljucivanje biblioteke za rad sa random vrednostima
#include <random>

// Definicija makroa STB_IMAGE_IMPLEMENTATION
// omogucava implementaciju funkcionalnosti iz stb_image biblioteke
#define STB_IMAGE_IMPLEMENTATION

// Ukljucivanje stb_image biblioteke za rad sa slikama
// ucitavanje slika u memoriju kao teksture itd.
#include "stb_image.h"

using namespace std;

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
void createSemafor(float semafor[], float r, float center_x, float center_y_crveno, float spacing);
void createKrug(float krug[], float r, float center_x_crveno, float center_y_crveno);
void createSemaforArray(float r, float spacing, int count, float positions[][2], float* allVertices);
void setLightColor(GLint uColorLoc, int state, int lightIndex);
unsigned int findNextSemaforState(int state);
float findNextTrajanjeSvetla(int state, float promenaTajmeraCrveno);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void checkSegmentClickAndHover(float mouseX, float mouseY);
void processMouseHold(GLFWwindow* window);
void setTajmerColor(GLint uColorLoc, int state);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static unsigned loadImageToTexture(const char* filePath);

// definicija makroa CRES, koristi se da bi se definisao broj tacaka koje se koriste
// za iscrtavanje kruga
#define CRES 30

// definicija boolean promenljivih koje ce se koristiti za proveru
// da li je pritisnuto levi ili desni klik misa i da li je segment hoverovan
// definisu se kao globalne promenljive kako bi bile dostupne u svim delovima koda
bool isLeftMouseButtonPressed = false;
bool isRightMouseButtonPressed = false;
bool isSegmentHovered = false;

// stepeni prigusenosti bulevara i ulica (po segmentima)
// na pocetku su sve vrednosti postavljene na 0.0
// ove promenljive se takodje definisu kao globalne kako bi bile dostupne u svim delovima koda
// kasnije ce se ove vrednosti menjati u skladu sa svetlima semafora ili
// pritiskom na levi ili klik misa
float prigusenostBulevarDole[] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

float prigusenostBulevarGore[] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

float prigusenostUliceLevo[] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

float prigusenostUliceDesno[] = {
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

// ova vrednost oznacava brzinu promene prigusenosti na segmentu ulice ili bulevara
// inicijalna vrednost ove promenljive je postavljena na 0.005
// ova vrednost ce se kasnije menjati skrolovanjem misa
// ova promenljiva se takodje definise kao globalna da bi bila prisutna iz svih delova koda
float brzinaPromenePrigusenosti = 0.005;

// broj teksture koja je trenutno hoverovana
// inicijalna vrednost je postavljena na -1 jer ni jedna tekstura nije hoverovana
// po default-u
int hoveredTexture = -1;

// enum koji definise sva moguca stanja semafora
enum SemaforState { RED, RED_YELLOW, GREEN, YELLOW };

int main(void)
{
	// inicijalizacija GLFW biblioteke koja omogucava rad sa prozorima i unosima
	if (!glfwInit())
	{
		std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
		return 1;
	}

	// postavljanje verzije OpenGL-a koju cemo koristiti
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // Glavna verzija OpenGL-a (major)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // Sporedna verzija OpenGL-a (minor)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Koristimo core profile (moderni OpenGL)

	// kreiranje prozora
	GLFWwindow* window; // Pokazivac na GLFW prozor
	unsigned int wWidth = 900; // sirina prozora
	unsigned int wHeight = 900; // visina prozora
	const char wTitle[] = "Kontroler saobracaja"; // naslov prozora
	window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); // Kreiranje prozora (NULL za fullscreen i deljenje konteksta)
	if (window == NULL)
	{
		// ako prozor nije kreiran, ispisi poruku o gresci, terminira GLFW i zavrsava program
		std::cout << "Prozor nije napravljen! :(\n";
		glfwTerminate(); // ciscenje resursa GLFW-a
		return 2; // kod greske
	}

	// Postavljanje kreiranog prozora kao aktivnog OpenGL konteksta
	glfwMakeContextCurrent(window);	

	// inicijalizacija GLEW biblioteke koja omogucava dinamicko ucitavanje OpenGL funkcija
	if (glewInit() != GLEW_OK)
	{
		// Ako ucitavanje GLEW-a nije uspelo, ispisuje se poruka o gresci i program se zavrsava
		std::cout << "GLEW nije mogao da se ucita! :'(\n";
		return 3; // Kod greske
	}

	// koordinate segmenata za levu traku bulevara, gde se vozila krecu u smeru ka dole
	float bulevarSmerDole[] = {	//levo
		//	  x	   y		
			-0.02, 1.0,		 ///segment 0
			-0.02, 0.75,

			-0.02, 0.75,	//1
			-0.02, 0.5,

			-0.02, 0.5,		//2
			-0.02, 0.25,

			-0.02, 0.25,	//3
			-0.02, 0.0,

			-0.02, 0.0,		//4
			-0.02, -0.25,

			-0.02, -0.25,	//5
			-0.02, -0.5,

			-0.02, -0.5,	//6
			-0.02, -0.75,

			-0.02, -0.75,	//7
			-0.02, -1.0,
	};

	// koordinate segmenata za desnu traku bulevara,
	// gde se vozila krecu u smeru ka gore
	float bulevarSmerGore[] = {		//desno
		0.02, 1.0,		//0
		0.02,0.75,

		0.02,0.75,		//1
		0.02,0.5,

		0.02,0.5,		//2
		0.02,0.25,

		0.02,0.25,		//3
		0.02, 0.0,

		0.02, 0.0,		//4
		0.02, -0.25,

		0.02, -0.25,	//5
		0.02, -0.5,

		0.02, -0.5,		//6
		0.02, -0.75,

		0.02, -0.75,	//7
		0.02, -1.0,
	};

	// koordinate verteksa za segmente ulica na levoj strani bulevara
	float uliceSegmentiLevo[] = {
		-1.0, 0.75,		//0
		-0.035, 0.75,

		-1.0, 0.5,		//1
		-0.035, 0.5,

		-1.0, 0.25,		//2
		-0.035, 0.25,

		-1.0, 0.0,		//3
		-0.035, 0.0,

		-1.0, -0.25,	//4
		-0.035, -0.25,

		-1.0, -0.5,		//5
		-0.035, -0.5,

		-1.0, -0.75,	//6
		-0.035, -0.75,
	};

	// koordinate verteksa za segmente ulica na desnoj strani bulevara
	float uliceSegmentiDesno[] = {
		1.0, 0.75,
		0.035, 0.75,

		1.0, 0.5,
		0.035, 0.5,

		1.0, 0.25,
		0.035, 0.25,

		1.0, 0.0,
		0.035, 0.0,

		1.0, -0.25,
		0.035, -0.25,

		1.0, -0.5,
		0.035, -0.5,

		1.0, -0.75,
		0.035, -0.75,
	};

	// Velicina jednog verteksa u bajtovima (2 float-a, svaka koordinata x i y je 4 bajta)
	unsigned int stride = 2 * sizeof(float);

	// koordinate centara semafora za traku bulevara ka gore
	float pozicijeSemaforaSmerDole[7][2] = {           // Centar crvenog svetla
		{-0.02, 0.85},
		{-0.02, 0.6},
		{-0.02, 0.35},
		{-0.02, 0.1},
		{-0.02, -0.15},
		{-0.02, -0.4},
		{-0.02, -0.65},
	};

	// koordinate centara semafora za traku bulevara ka gore
	float pozicijeSemaforaSmerGore[7][2] = {           // Centar crvenog svetla
		{0.02, 0.75},
		{0.02, 0.5},
		{0.02, 0.25},
		{0.02, 0.0},
		{0.02, -0.25},
		{0.02, -0.5},
		{0.02, -0.75},
	};

	// svetla semafora su predstavljena krugovima
	float r = 0.018f;	// poluprecnik jednog svetla semafora
	float spacing = 2 * r; // razmak izmedju svetala
	int brojSemafora = 7; // broj semafora po traci

	// nizovi za cuvanje koordinata svetala semafora
	float semaforiBulevarDole[7 * 3 * (2 + 2 * (CRES + 1))]; // 7 semafora, po 3 kruga
	float semaforiBulevarGore[7 * 3 * (2 + 2 * (CRES + 1))]; // 7 semafora, po 3 kruga

	// kreiranje niza semafora za svaku traku bulevara
	createSemaforArray(r, spacing, brojSemafora, pozicijeSemaforaSmerDole, semaforiBulevarDole);
	createSemaforArray(r, spacing, brojSemafora, pozicijeSemaforaSmerGore, semaforiBulevarGore);

	// Koordinate za tajmere duz trake ka dole
	float tajmeriBulevarDole[] =
	{
		-0.5, 0.9,
		-0.02, 0.9,

		-0.5, 0.65,
		-0.02, 0.65,

		-0.5, 0.4,
		-0.02, 0.4,

		-0.5, 0.15,
		-0.02, 0.15,

		-0.5, -0.1,
		-0.02, -0.1,

		-0.5, -0.35,
		-0.02, -0.35,

		-0.5, -0.6,
		-0.02, -0.6,
	};

	// Koordinate tajmera duz trake ka gore
	float tajmeriBulevarGore[] =
	{
		0.02, 0.8,
		0.5, 0.8,

		0.02, 0.55,
		0.5, 0.55,

		0.02, 0.3,
		0.5, 0.3,

		0.02, 0.05,
		0.5, 0.05,

		0.02, -0.2,
		0.5, -0.2,

		0.02, -0.45,
		0.5, -0.45,

		0.02, -0.7,
		0.5, -0.7,
	};

	float sirinaTajmera = 0.48;  // sirina svakog tajmera

	// koordinate za ime ulice (ovo je tekstura)
	float imeUlice[] = {
		-0.9, -1.0,             0.0, 0.0,
		 -0.2, -1.0,			1.0, 0.0,
		-0.9, -0.9,			    0.0, 1.0,
		 -0.2, -0.9,		    1.0, 1.0,
	};

	// koordinate za potpis (ovo je tekstura)
	float potpis[] = {
		//  x     y                 s   t
			0.3, -1.0,             0.0, 0.0,
			 1.0, -1.0,			    1.0, 0.0,
			0.3, -0.9,			    0.0, 1.0,
			1.0, -0.9,		        1.0, 1.0,
	};

	// Koordinate semafora za skretanje ka desno (sa strelicama)
	// u smeru ka dole
	// ovo je tekstura
	float skretanjeDesnoBulevarDole[] = {
		-0.12,  0.77,							0.0, 0.0,
		 -0.12 + 4 * r,  0.77,			    1.0, 0.0,
		-0.12,  0.77 + 4 * r,					0.0, 1.0,
		 -0.12 + 4 * r, 0.77 + 4 * r,		    1.0, 1.0,
	};

	// koordinate semafora za skretanje desno u smeru ka gore (sa strelicama)
	// ovo je tekstura
	float skretanjeDesnoBulevarGore[] = {
		0.12 - 4 * r, 0.67,						 0.0, 0.0,
		0.12 , 0.67,								1.0, 0.0,
		0.12 - 4 * r, 0.67 + 4 * r,			     0.0, 1.0,
		0.12 , 0.67 + 4 * r,						1.0, 1.0,
	};

	// Definisanje velicine stride za teksture (velicina jednog elementa teksture u bajtovima)
	unsigned int strideTexture = (2 + 2) * sizeof(float);  // 2 koordinate pozicije + 2 koordinate teksture

	// kreiranje nizova za cuvanje ertex Array Object (VAO) i Vertex Buffer Object (VBO)
	unsigned VAO[25];
	glGenVertexArrays(25, VAO); // generise 25 VAO objekata
	unsigned VBO[25];
	glGenBuffers(25, VBO); // generise 25 VBO objekata

	// Postavljanje VBO i VAO za razlicite skupove podataka

	// 1. Postavljanje VAO za traku bulevara u smeru ka dole
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulevarSmerDole), bulevarSmerDole, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 2. Postavljanje VAO za traku bulevara u smeru ka gore
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bulevarSmerGore), bulevarSmerGore, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 3. Postavljanje VAO za segmente ulica sa leve strane bulevara
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uliceSegmentiLevo), uliceSegmentiLevo, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 4. Postavljanje VAO za segmente ulica sa desne strane bulevara
	glBindVertexArray(VAO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uliceSegmentiDesno), uliceSegmentiDesno, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 5. Postavljanje VAO za semafore na bulevaru (smer ka dole)
	glBindVertexArray(VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(semaforiBulevarDole), semaforiBulevarDole, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 6. Postavljanje VAO za semafore na bulevaru (smer ka gore)
	glBindVertexArray(VAO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(semaforiBulevarGore), semaforiBulevarGore, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 7. Postavljanje VAO za tajmere semafora na bulevaru (smer ka dole)
	glBindVertexArray(VAO[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tajmeriBulevarDole), tajmeriBulevarDole, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 8. Postavljanje VAO za tajmere semafora na bulevaru (smer ka gore)
	glBindVertexArray(VAO[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tajmeriBulevarGore), tajmeriBulevarGore, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	
	// 9. Postavljanje VAO za ime ulice (koristi teksture) 
	glBindVertexArray(VAO[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(imeUlice), imeUlice, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)0); // Pozicioni atribut
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)(2 * sizeof(float))); // Teksturni atribut
	glEnableVertexAttribArray(1);
	
	// 10. Postavljanje VAO za potpis (koristi teksture)
	glBindVertexArray(VAO[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(potpis), potpis, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// 11. Postavljanje VAO za semafore za skretanje desno na bulevaru (smer ka dole)
	glBindVertexArray(VAO[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skretanjeDesnoBulevarDole), skretanjeDesnoBulevarDole, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// 12. Postavljanje VAO za semafore za skretanje desno na bulevaru (smer ka gore)
	glBindVertexArray(VAO[11]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skretanjeDesnoBulevarGore), skretanjeDesnoBulevarGore, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideTexture, (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Ponistavanje vezivanja bafera i VAO kako ne bi doslo do slucajnih izmena
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Ucitaj i kompajliraj verteks i fragment sejdere (napravi se unified sejder) za osnovno i renderovanje teksture
	unsigned int basicShader = createShader("basic.vert", "basic.frag"); // osnovni sejder za objekte za objekte bez tekstura
	unsigned int textureShader = createShader("texture.vert", "texture.frag"); // sejder za renderovanje teksturisanih objekata

	// dobavi lokacije uniformi iz basic sejdera
	unsigned int uPosLoc = glGetUniformLocation(basicShader, "uPos"); // za offset pozicije
	unsigned int uColorLoc = glGetUniformLocation(basicShader, "uColor"); // za boju
	unsigned int uFixedPosLoc = glGetUniformLocation(basicShader, "uFixedPos"); // za fiksirane pozicije
	
	// dobavljanje lokacija uniformi iz texture sejdera
	unsigned int uPosTexLoc = glGetUniformLocation(textureShader, "uPos"); // za texture position offset

	// ucitavamo teksturu za potpis
	unsigned potpisTexture = loadImageToTexture("res/indeks.PNG"); // ucitavamo teksturu od slike
	glBindTexture(GL_TEXTURE_2D, potpisTexture); // bajndujemo teksturu
	glGenerateMipmap(GL_TEXTURE_2D); // generisemo mipmapu za bolje skaliranje

	// postavljanje parametara za wrapping i filtriranje
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // ponavljanje teksture horizontalno
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // ponavljanje tekstura vertikalno
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // nearest neighbour for minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // nearest neighbor for magnification
	glBindTexture(GL_TEXTURE_2D, 0); // otkaci teksturu

	// niz za cuvanje tekstura za nazive ulica
	unsigned uliceTextures[8];
	for (int i = 0; i < 8; ++i)
	{
		// konstruisemo putanju za svaku sliku naziva ulice
		string filePath = "res/ulica" + to_string(i) + ".PNG";

		// ucitavamo teksturu za trenutni naziv ulice
		uliceTextures[i] = loadImageToTexture(filePath.c_str());

		glBindTexture(GL_TEXTURE_2D, uliceTextures[i]); // bajndujemo teksturu
		glGenerateMipmap(GL_TEXTURE_2D); // generisemo mipmap za bolje skaliranje
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // ponavljanje teksture horizontalno
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // ponavljanje teksture vertikalno
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // nearest neighbor for minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // nearest neighbor for magnification
		glBindTexture(GL_TEXTURE_2D, 0); // otkaci teksturu
	}

	// ucitavanje i konfiguracija teksture za strelicu na levo
	unsigned leftArrowTexture = loadImageToTexture("res/leftArrow.PNG");
	glBindTexture(GL_TEXTURE_2D, leftArrowTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// ucitavanje i konfiguracija teksture za strelicu na desno
	unsigned rightArrowTexture = loadImageToTexture("res/rightArrow.PNG");
	glBindTexture(GL_TEXTURE_2D, rightArrowTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ukupan broj verteksa po krugu, ukljucujuci centar i obod
	// CRES: broj segmenata za krug (rezolucija)
	int totalVerticesPerCircle = CRES + 2; // (2 + 2 * (CRES + 1) ) / 2

	// Ukupan broj verteksa potrebnih za tri kruga za semafor (crveno, zuto i zeleno svetlo)
	int totalVerticesPerSemafor = 3 * totalVerticesPerCircle; // Vertices for 3 circles

	// inicijalna stanja semafora (na pocetku je kod svih crveno) za oba smera (ka dole i ka gore)
	SemaforState semaforiState_Dole[] = { RED, RED, RED, RED, RED, RED, RED }; // semafori u smeru ka dole
	SemaforState semaforiState_Gore[] = { RED, RED, RED, RED, RED, RED, RED }; // semafori u smeru ka gore

	// Horizontalni offseti za prikazivanje tajmera na bulevaru za oba smera
	float tajmeriBulevarDole_x[] = { 0, 0, 0, 0, 0, 0, 0 };  // pomeranje leve tacke tajmera po x osi
	float tajmeriBulevarGore_x[] = { 0, 0, 0, 0, 0, 0, 0 };

	// Trajanje crvenog svetla za semafore u oba smera (u sekundama)
	int trajanjeCrvenogBulevarDole[7];  // u sekundama, random, izmedju 20 i 40 sekundi
	int trajanjeCrvenogBulevarGore[7];

	// Random number generation setup
	std::random_device rd;	// Seed for random number generation
	std::mt19937 gen(rd()); // Mersenne Twister random number generator
	std::uniform_int_distribution<> distrib(20, 40);  // Uniform distribution between 20 and 40 seconds

	// Generisemo random trajanja crvenog svetla za semafore u smeru ka dole
	for (int i = 0; i < 7; i++)
	{
		trajanjeCrvenogBulevarDole[i] = distrib(gen); // dodeli random trajanje za svaki semafor
	}

	// Generisemo random trajanje crvenog svetla za semafore u smeru ka gore
	for (int i = 0; i < 7; i++)
	{
		trajanjeCrvenogBulevarGore[i] = distrib(gen); // dodela random trajanja crvenog svetla semaforu
	}

	// Niz za cuvanje trajanja trenutnog svetla u smeru ka dole
	float trajanjeTrenutnogSvetlaBulevarDole[sizeof(trajanjeCrvenogBulevarDole) / sizeof(trajanjeCrvenogBulevarDole[0])];
	// Inicijalizujemo trenutna trajanja svetla na trajanje crvenog svetla
	std::copy(std::begin(trajanjeCrvenogBulevarDole), std::end(trajanjeCrvenogBulevarDole), trajanjeTrenutnogSvetlaBulevarDole);  // kopiranje niza trajanjeCrvenogBulevarDole u trajanjeTrenutnogSvetlaBulevarDole; na pocetku programa je ukljuceno crveno svetlo na svim semaforima
	
	// Niz za cuvanje trajanja trenutnog svetla u smeru ka gore
	float trajanjeTrenutnogSvetlaBulevarGore[sizeof(trajanjeCrvenogBulevarGore) / sizeof(trajanjeCrvenogBulevarGore[0])];
	// Inicijalizujemo trenutno trajanje svetla vrednostima za crveno svetlo
	std::copy(std::begin(trajanjeCrvenogBulevarGore), std::end(trajanjeCrvenogBulevarGore), trajanjeTrenutnogSvetlaBulevarGore);  

	// Tajmer za promene svetla
	auto tajmerPreviousTime = glfwGetTime(); // uzima vreme kada je program startovan

	// tajmer za promene frejmova
	double previousFrameTime = glfwGetTime(); // uzima vreme kada je program startovan

	// postavljanje callback funkcije za dogadjaje tastera misa
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// postavljanje callback funkcije za dogadjaje skrola misa
	glfwSetScrollCallback(window, scroll_callback);

	// Definisemo ciljani frame rate (60 frejmova u sekundi)
	const int framesPerSec = 60;
	const double frameTime = 1.0 / framesPerSec; // vreme za svaki frejm u sekundama

	// postavljanje podrazumevane boje pozadine za OpenGL kontekst (tamno siva)
	glClearColor(0.2, 0.2, 0.2, 1.0);

	// grlavna petlja programa (render loop)
	while (!glfwWindowShouldClose(window))
	{
		// obrada dogadjaja sa tastature, misa itd.
		glfwPollEvents();

		// obrada drzanja tastera misa
		processMouseHold(window);

		// ako je pritisnut ESC, zatvaramo prozor
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// dohvatanje trenutnog vremena
		auto currentTime = glfwGetTime();

		// ogranicenje na 60 frejmova u sekundi (FPS)
		double elapsedFrameTime = currentTime - previousFrameTime;

		if (elapsedFrameTime < frameTime)
		{
			// pauza (uspavljivanje niti) ako frejm nije dostigao 60 FPS
			std::this_thread::sleep_for(
				std::chrono::duration<double>(frameTime - elapsedFrameTime));
			continue;
		}
		previousFrameTime = currentTime;

		// logika za tajmere semafora
		auto elapsedTajmerTime = currentTime - tajmerPreviousTime;

		// obrada tajmera za semafore na bulevaru (dole)
		for (int i = 0; i < 7; i++)
		{
			// Sprecavanje da tajmer predje maksimalnu sirinu
			if (tajmeriBulevarDole_x[i] < -sirinaTajmera) {
				tajmeriBulevarDole_x[i] = -sirinaTajmera;
			}

			// Racunanje pomeraja tajmera po sirini
			float podeokTajmera = sirinaTajmera / trajanjeTrenutnogSvetlaBulevarDole[i];
			if (elapsedTajmerTime >= 1)  // azuriramo tajmer svake sekunde
			{
				// smanjujemo sirinu tajmera dok ne dostigne nulu
				if (tajmeriBulevarDole_x[i] > -sirinaTajmera) {
					tajmeriBulevarDole_x[i] -= podeokTajmera;
					tajmerPreviousTime = currentTime;
				}
				else if (tajmeriBulevarDole_x[i] == -sirinaTajmera)  // menjanje stanja semafora kada tajmer istekne
				{
					// Prelaz na sledece stanje semafora
					semaforiState_Dole[i] = static_cast<SemaforState>(findNextSemaforState(semaforiState_Dole[i]));

					// podesavanje trajanja trenutnog svetla
					trajanjeTrenutnogSvetlaBulevarDole[i] = findNextTrajanjeSvetla(semaforiState_Dole[i], trajanjeCrvenogBulevarDole[i]);

					// resetovanje tajmera
					tajmeriBulevarDole_x[i] = 0.0;
				}
			}
		}

		// isto za semafore na bulevaru (gore)
		for (int i = 0; i < 7; i++)
		{
			if (tajmeriBulevarGore_x[i] < -sirinaTajmera) {
				tajmeriBulevarGore_x[i] = -sirinaTajmera;
			}
			float podeokTajmera = sirinaTajmera / trajanjeTrenutnogSvetlaBulevarGore[i];
			if (elapsedTajmerTime >= 1)  // update tajmer svake sekunde
			{
				if (tajmeriBulevarGore_x[i] > -sirinaTajmera) {
					tajmeriBulevarGore_x[i] -= podeokTajmera;
					tajmerPreviousTime = currentTime;
				}
				else if (tajmeriBulevarGore_x[i] == -sirinaTajmera) // promena stanja semafora i skale odbrojavanja
				{
					semaforiState_Gore[i] = static_cast<SemaforState>(findNextSemaforState(semaforiState_Gore[i]));
					trajanjeTrenutnogSvetlaBulevarGore[i] = findNextTrajanjeSvetla(semaforiState_Gore[i], trajanjeCrvenogBulevarGore[i]);
					tajmeriBulevarGore_x[i] = 0.0;
				}
			}
		}

		// logika za prigusenje (redistribucija izmedju segmenata)
		for (int i = 0; i < 7; i++) // iteriramo do 7 (a ne i 7) jer poslednji segment samo opada tokom vremena, nema raspodele prigusenosti na sledece segmente (jer ni nema sledecih segmenata)
		{
			if (semaforiState_Dole[i] == GREEN && prigusenostBulevarDole[i] > 0)
			{
				float raspodela = brzinaPromenePrigusenosti * prigusenostBulevarDole[i] / 3;

				// Redistribucija prigusenja na susedne segmente
				prigusenostBulevarDole[i + 1] = (prigusenostBulevarDole[i + 1] + raspodela < 1) ? prigusenostBulevarDole[i + 1] + raspodela : 1;
				prigusenostUliceLevo[i] = (prigusenostUliceLevo[i] + raspodela < 1) ? prigusenostUliceLevo[i] + raspodela : 1;
				prigusenostUliceDesno[i] = (prigusenostUliceDesno[i] + raspodela < 1) ? prigusenostUliceDesno[i] : 1;

				prigusenostBulevarDole[i] = (prigusenostBulevarDole[i] - 3 * raspodela > 0) ? prigusenostBulevarDole[i] - 3 * raspodela : 0;
			}
		}

		// redistribucija za bulevar gore
		for (int i = 6; i >= 0; i--)
		{
			if (semaforiState_Gore[i] == GREEN && prigusenostBulevarGore[i + 1] > 0) {
				float raspodela = brzinaPromenePrigusenosti * prigusenostBulevarGore[i + 1] / 3;

				prigusenostBulevarGore[i] = (prigusenostBulevarGore[i] + raspodela < 1) ? prigusenostBulevarGore[i] : 1;
				prigusenostUliceLevo[i] = (prigusenostUliceLevo[i] + raspodela < 1) ? prigusenostUliceLevo[i] + raspodela : 1;
				prigusenostUliceDesno[i] = (prigusenostUliceDesno[i] + raspodela < 1) ? prigusenostUliceDesno[i] + raspodela : 1;

				prigusenostBulevarGore[i + 1] = (prigusenostBulevarGore[i + 1] - 3 * raspodela > 0) ? prigusenostBulevarGore[i + 1] - 3 * raspodela : 0;
			}
		}

		// "Oticanje" prigusenja izvan ekrana (smanjenje preostalih vrednosti)
		float oticanjeIzvanEkrana = brzinaPromenePrigusenosti / 10;  // delimo da bi se dolazni segmenti "praznili" sporije nego sto se pune (da bi se bolje videla raspodela prigusenosti)
		prigusenostBulevarDole[7] = (prigusenostBulevarDole[7] > 0) ? prigusenostBulevarDole[7] - oticanjeIzvanEkrana : 0;
		prigusenostBulevarGore[7] = (prigusenostBulevarGore[7] > 0) ? prigusenostBulevarGore[7] - oticanjeIzvanEkrana : 0;

		for (int i = 0; i < 7; i++)
		{
			prigusenostUliceLevo[i] = (prigusenostUliceLevo[i] > 0) ? prigusenostUliceLevo[i] - oticanjeIzvanEkrana : 0;
			prigusenostUliceDesno[i] = (prigusenostUliceDesno[i] > 0) ? prigusenostUliceDesno[i] - oticanjeIzvanEkrana : 0;
		}

		// crtanje

		// Ocistimo ekran i postavimo osnovni sejder
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(basicShader);

		// Crtanje segmenata ulica sa leve strane
		glBindVertexArray(VAO[2]);  // VAO[2] sadrzi podatke za segmente ulica sa leve strane
		glLineWidth(10); // postavljamo sirinu linija za crtanje segmenata ulica
		for (int i = 0; i < 7; i++)  // 7 segmenata ulica sa leev strane
		{
			glUniform3f(uColorLoc, prigusenostUliceLevo[i], 1.0 - prigusenostUliceLevo[i], 0.0); // boja se prilagodjava na osnovu prigusenosti
			glDrawArrays(GL_LINES, i * 2, 2); // crtamo liniju za svaki segment
		}
		
		// crtanje segmenata ulica sa desne strane
		glBindVertexArray(VAO[3]); // VAO[3] sadrzi podatke za segmente ulica sa desne strane
		glLineWidth(10);
		for (int i = 0; i < 7; i++) 
		{
			glUniform3f(uColorLoc, prigusenostUliceDesno[i], 1.0 - prigusenostUliceDesno[i], 0.0);
			glDrawArrays(GL_LINES, i * 2, 2);
		}
		
		// Crtanje trake bulevara u smeru ka dole
		glBindVertexArray(VAO[0]); // VAO[0] sadrzi podatke za traku bulevara u smeru ka gore
		glLineWidth(15); // sirina linije je veca za bulevare
		for (int i = 0; i < 8; i++)  // bulevar ima 8 segmenata
		{
			glUniform3f(uColorLoc, prigusenostBulevarDole[i], 1.0 - prigusenostBulevarDole[i], 0.0);
			glDrawArrays(GL_LINES, i * 2, 2);
		}
		
		// crtanje trake bulevara u smeru ka gore
		glBindVertexArray(VAO[1]); // VAO[1] sadrzi podatke za traku bulevara u smeru ka gore
		glLineWidth(15);
		for (int i = 0; i < 8; i++)  // bulevar ima 8 segmenata
		{
			glUniform3f(uColorLoc, prigusenostBulevarGore[i], 1.0 - prigusenostBulevarGore[i], 0.0);
			glDrawArrays(GL_LINES, i * 2, 2);
		}

		// crtanje semafora uz traku bulevara u smeru ka dole
		glBindVertexArray(VAO[4]); // VAO[4] sadrzi podatke za semafore u smeru ka dole
		glUseProgram(basicShader);
		for (int i = 0; i < 7; i++) // imamo 7 semafora sa leve strane, svaki semafor ima 3 svetla (crveno, zuto, zeleno)
		{
			int semaforStartIndex = i * totalVerticesPerSemafor; // indeks za pocetak crtanja semafora
			for (int j = 0; j < 3; j++) // prolazimo kroz svetla svakog semafora
			{
				glUniform3f(uColorLoc, 0.2, 0.2, 0.2); // osnovna boja je prigusena, siva (iskljucena svetla)
				setLightColor(uColorLoc, semaforiState_Dole[i], j); // podesavamo boju na osnovu stanja semafora

				int svetloStartIndex = semaforStartIndex + j * totalVerticesPerCircle; // indeks za svetlo
				glDrawArrays(GL_TRIANGLE_FAN, svetloStartIndex, totalVerticesPerCircle); // crtamo svetlo kao krug
			}
		}
		
		// crtanje semafora za traku u smeru ka gore (isto kao za smer na dole)
		glBindVertexArray(VAO[5]);
		glUseProgram(basicShader);
		for (int i = 0; i < 7; i++)
		{
			int semaforStartIndex = i * totalVerticesPerSemafor;
			for (int j = 0; j < 3; j++)
			{
				glUniform3f(uColorLoc, 0.2, 0.2, 0.2);
				setLightColor(uColorLoc, semaforiState_Dole[i], j);

				int svetloStartIndex = semaforStartIndex + j * totalVerticesPerCircle;
				glDrawArrays(GL_TRIANGLE_FAN, svetloStartIndex, totalVerticesPerCircle);
			}
		}

		// crtanje tajmera za traku bulevara ka dole
		glBindVertexArray(VAO[6]);  // VAO[6] sadrzi podatke za tajmere za traku bulevara u smeru ka dole
		glLineWidth(10);
		for (int i = 0; i < 7; i++) {
			glUniform3f(uColorLoc, 0, 0, 1.0);	// crtamo plavi tajmer koji ne odbrojava sekunde
			glUniform2f(uPosLoc, 0, 0.03);		// transliramo ga za 0.03 po y-osi
			glUniform1f(uFixedPosLoc, 1);		// ovde nije bitna tacna vrednost, samo da x ne upada u opseg tajmera, da bi se tajmer mogao translirati ka gore
			glDrawArrays(GL_LINES, i * 2, 2);

			// ovaj tajmer odbrojava
			setTajmerColor(uColorLoc, semaforiState_Dole[i]); // podesavamo boju tajmera na osnovu stanja semafora
			glUniform2f(uPosLoc, tajmeriBulevarDole_x[i], 0);  // translira
			glUniform1f(uFixedPosLoc, tajmeriBulevarDole[0]);  // ova vrednost x-a se ne sabira sa uPos (leva tacka tajmera), pa dobijamo efekat odbrojavanja jer se samo desna tacka translira. Da ne fiksiramo ovu tacku, cela linija bi se pomerala u desnu stranu.
			glDrawArrays(GL_LINES, i * 2, 2);
		}
		
		glUniform2f(uPosLoc, 0, 0);
		
		// slicno i za tajmere u smeru ka gore
		glBindVertexArray(VAO[7]);
		glLineWidth(10);
		for (int i = 0; i < 7; i++) {
			glUniform3f(uColorLoc, 0, 0, 1.0);	// crtamo plavi tajmer koji ne odbrojava sekunde
			glUniform2f(uPosLoc, 0, 0.03);		// transliramo ga za 0.03 po y-osi
			glUniform1f(uFixedPosLoc, 1);		// ovde nije bitna tacna vrednost, samo da x ne upada u opseg tajmera, da bi se tajmer mogao translirati ka gore
			glDrawArrays(GL_LINES, i * 2, 2);

			// ovaj tajmer odbrojava
			setTajmerColor(uColorLoc, semaforiState_Gore[i]);
			glUniform2f(uPosLoc, tajmeriBulevarGore_x[i], 0);  // translira
			glUniform1f(uFixedPosLoc, tajmeriBulevarGore[0]);  // ova vrednost x-a se ne sabira sa uPos (leva tacka tajmera), pa dobijamo efekat odbrojavanja jer se samo desna tacka translira. Da ne fiksiramo ovu tacku, cela linija bi se pomerala u desnu stranu.
			glDrawArrays(GL_LINES, i * 2, 2);
		}
		
		glUniform2f(uPosLoc, 0, 0);

		// crtanje hover tekstura za ulice
		if (hoveredTexture != -1)
		{
			glUseProgram(textureShader); // koristimo sejder za teksture
			glBindVertexArray(VAO[8]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, uliceTextures[hoveredTexture]); // bindujemo teksturu
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // crtamo kao cetvorougao
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// crtanje potpisa
		glUseProgram(textureShader);
		glBindVertexArray(VAO[9]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, potpisTexture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindTexture(GL_TEXTURE_2D, 0);

		// crtanje semafora za skretanje u smeru ka dole
		glBindVertexArray(VAO[10]);
		for (int i = 0; i < 7; i++)
		{
			if (semaforiState_Dole[i] == RED) { // strelica se prikazuje kada je crveno svetlo
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, leftArrowTexture); // strelica za levo
				glUniform2f(uPosTexLoc, 0, -0.25 * i); // pomeramo strelicu za odgovarajucu poziciju

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else if (semaforiState_Dole[i] == YELLOW && static_cast<int>(glfwGetTime() * 2) % 2 == 0) // treperi kada je zuto svetlo, treperi na 0.5 sekundi
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, leftArrowTexture);
				glUniform2f(uPosTexLoc, 0, -0.25 * i);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		glUniform2f(uPosTexLoc, 0, 0);
		
		// crtanje semafora za skretanje za smer ka gore (isto kao za smer na dole)
		glBindVertexArray(VAO[11]);
		for (int i = 0; i < 7; i++)
		{
			if (semaforiState_Gore[i] == RED) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rightArrowTexture);
				glUniform2f(uPosTexLoc, 0, -0.25 * i);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else if (semaforiState_Gore[i] == YELLOW && static_cast<int>(glfwGetTime() * 2) % 2 == 0) // treperi na 0.5 sekundi
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rightArrowTexture);
				glUniform2f(uPosTexLoc, 0, -0.25 * i);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		glUniform2f(uPosTexLoc, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window);
	}

	// oslobadjanje memorije
	glDeleteBuffers(25, VBO);
	glDeleteVertexArrays(25, VAO);
	glDeleteProgram(basicShader);

	// ako je sve ok, gasimo program
	glfwTerminate();

	return 0;
}

void createKrug(float krug[], float r, float center_x, float center_y) {
	// Postavljamo centar kruga na zadate koordinate
	krug[0] = center_x; // x koordinata centra kruga
	krug[1] = center_y; // y koordinata centra kruga
	int i;
	// iteracija kroz tacke na obodu kruga
	for (i = 0; i <= CRES; i++) // CRES je broj segmenata koji odredjuje preciznost kruga
	{
		// racuna x koordinatu tacke na obodu kruga
		// prema formuli: x = x_centar + r * cos(ugao)
		// formula radi sa radijanima
		krug[2 + 2 * i] = krug[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));

		// racuna y koordinatu tacke na obodu kruga
		// formula: y = y_centar + r * sin(ugao)
		krug[2 + 2 * i + 1] = krug[1] + r * sin((3.141592 / 180) * (i * 360 / CRES));
	}
}

void createSemafor(float semafor[], float r, float center_x, float center_y_crveno, float spacing) {
	// Ukupan broj tacaka za svaki krug (centar + obod)
	int totalVerticesPerCircle = 2 + 2 * (CRES + 1);

	// Kreiramo prvi krug za crveno svetlo semafora
	// Pocetak niza semafor[] se koristi za koordinate ovog kruga
	createKrug(&semafor[0], r, center_x, center_y_crveno);	// Crveno svetlo

	// Kreiramo drugi krug za zuto svetlo semafora
	// Pocetak za ovaj krug u nizu se pomera za totalVerticesPerCircle (da ne preklopi crveno svetlo)
	// Y-koordinata je pomerena za -spacing ispod crvenog svetla
	createKrug(&semafor[totalVerticesPerCircle], r, center_x, center_y_crveno - spacing); // Yellow

	// Kreiramo treci krug za zeleno svetlo semafora
	// Pocetak za ovaj krug u nizu se pomera za 2 * totalVerticesPerCircle (da ne preklopi prethodne krugove)
	// Y koordinata je pomerena za 2 * spacing ispod zutog svetla
	createKrug(&semafor[2 * totalVerticesPerCircle], r, center_x, center_y_crveno - 2 * spacing); // Green
	
}

void createSemaforArray(float r, float spacing, int count, float positions[][2], float* allVertices) {
	// Ukupan broj koordinata za sva 3 kruga semafora (crveni, zuti i zeleni)
	int totalVerticesPoSemaforu = 3 * (2 + 2 * (CRES + 1)); // verteksi za 3 kruga

	// Petlja kroz sve semafore koje zelimo da kreiramo
	for (int i = 0; i < count; i++)
	{
		// dobijamo X i Y koordinate centra trenutnog semafora
		float baseX = positions[i][0];
		float baseY = positions[i][1];

		// Pozivamo funkciju createSemafor za svaki semafor.
		// &allVertices[i * totalVerticesPoSemaforu] znaci da za svaki semafor smestamo koordinate u odgovarajuci deo niza allVertices
		// na osnovu i (indeksa semafora), pomeramo pocetak niza za svaki sledeci semafor 
		createSemafor(&allVertices[i * totalVerticesPoSemaforu], r, baseX, baseY, spacing);
	}
}

void setLightColor(GLint uColorLoc, int state, int lightIndex) {
	// Funkcija postavlja boju svetla na semaforu na osnovu trenutnog stanja
	switch (state)
	{
	// Ako je stanje "RED" (crveno svetlo)
	case RED:
		if (lightIndex == 0)
		{
			// Ako je indeks svetla 0 (crveno svetlo), postavi boju na crvenu (1.0, 0.0, 0.0)
			glUniform3f(uColorLoc, 1.0, 0.0, 0.0);	// crveno svetlo
		}
		break;
	// Ako je stanje "RED_YELLOW" (crveno-zuto stanje)
	case RED_YELLOW:
		if (lightIndex == 0)
		{
			// Ako je indeks svetla 0 (crveno svetlo), postavi boju svetla na crvenu (1.0, 0.0, 0.0)
			glUniform3f(uColorLoc, 1.0, 0.0, 0.0);	// crveno svetlo
		}
		if (lightIndex == 1)
		{
			// Ako je indeks svetla 1 (zuto svetlo), postavi boju na zutu (1.0, 1.0, 0.0)
			glUniform3f(uColorLoc, 1.0, 1.0, 0.0);	// zuto svetlo
		}
		break;
	// Ako je stanje "GREEN" (zeleno svetlo)
	case GREEN:
		if (lightIndex == 2)
		{
			// Ako je indeks svetla 2 (zeleno svetlo), postavi boju na zelenu (0.0, 1.0, 0.0)
			glUniform3f(uColorLoc, 0.0, 1.0, 0.0);	// zeleno svetlo
		}
		break;
	// Ako je stanje "YELLOW" (samo zuto svetlo)
	case YELLOW:
		if (lightIndex == 1)
		{
			// Ako je indeks svetla 1 (zuto svetlo), postavi boju na zutu (1.0, 1.0, 0.0)
			glUniform3f(uColorLoc, 1.0, 1.0, 0.0);	// zuto svetlo
		}
		break;
	// Podrazumevani slucaj, ako stanje nije prepoznato, svetlo je ugaseno
	default:
		break; // default: svetlo je ugaseno (semafor je siv)
	}
}

void setTajmerColor(GLint uColorLoc, int state) {
	// Funkcija postavlja boju tajmera na osnovu trenutnog stanja semafora (state).

	switch (state)
	{
	// Ako je stanje "RED" (crveno svetlo)
	case RED:
		// Postavi boju na crveno (1.0, 0.0, 0.0)
		glUniform3f(uColorLoc, 1.0, 0.0, 0.0);
		break;
	// Ako je stanje "RED_YELLOW" (crveno-zuto stanje)
	case RED_YELLOW:
		// Postavi boju na zuto (1.0, 1.0, 0.0)
		glUniform3f(uColorLoc, 1.0, 1.0, 0.0);
		break;
	// Ako je stanje "GREEN" (zeleno svetlo)
	case GREEN:
		// Postavi boju na zeleno (0.0, 1.0, 0.0)
		glUniform3f(uColorLoc, 0.0, 1.0, 0.0);
		break;
	// Ako je stanje "YELLOW" (zuto svetlo)
	case YELLOW:
		// Postavi boju na zuto (1.0, 0.0, 0.0)
		glUniform3f(uColorLoc, 1.0, 1.0, 0.0);
		break;
	// Podrazumevani slucaj, ako stanje nije prepoznato - boja tajmera se ne menja
	default:
		break;
	}
}

unsigned int findNextSemaforState(int state) {
	// Funkcija vraca sledece stanje semafora na osnovu trenutnog stanja
	switch (state)
	{
		// Ako je trenutno stanje "RED" (crveno svetlo), sledece stanje je "RED_YELLOW" (crveno i zuto svetlo)
	case RED:
		return RED_YELLOW;
		// Ako je trenutno stanje "RED_YELLOW" (crveno i zuto svetlo), sledece stanje je "GREEN" (zeleno svetlo)
	case RED_YELLOW:
		return GREEN;
		// Ako je trenutno stanje "GREEN" (zeleno svetlo), sledece stanje je "YELLOW" (zuto svetlo)
	case GREEN:
		return YELLOW;
		// Ako je trenutno stanje "YELLOW" (zuto svetlo), sledece stanje je "RED" (crveno svetlo)
	case YELLOW:
		return RED;
		// Ako je trenutno stanje nepoznato (default), vraca se "RED" kao podrazumevano stanje
	default:
		return RED;
	}
}

float findNextTrajanjeSvetla(int state, float trajanjeCrvenogSvetla) {
	// Funkcija vraca trajanje sledeceg svetla (u sekundama) na osnovu trenutnog stanja semafora
	// Ulazni parametri:
	//	- state: Trenutno stanje semafora (RED, RED_YELLOW, GREEN, YELLOW)
	//	- trajanjeCrvenogSvetla: Trajanje crvenog svetla (u sekundama)
	switch (state)
	{
		// Ako je trenutno stanje "RED" (crveno svetlo), sledece stanje je isto kao trajanje crvenog svetla
	case RED:
		return trajanjeCrvenogSvetla;
		// Ako je trenutno stanje "RED_YELLOW" (crveno i zuto svetlo), sledece trajanje je fiksno 3 sekunde
	case RED_YELLOW:
		return 3;
		// Ako je trenutno stanje "GREEN" (zeleno svetlo), sledece trajanje svetla je polovina trajanja crvenog svetla 
	case GREEN:
		return trajanjeCrvenogSvetla / 2;
		// Ako je trenutno stanje "YELLOW" (zuto svetlo), sledece trajanje je fiksno 3 sekunde
	case YELLOW:
		return 3;
		// Ako stanje nije prepoznato, vraca trajanje crvenog svetla kao podrazumevano
	default:
		return trajanjeCrvenogSvetla;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// Ova funkcija se poziva svaki put kada se pritisne ili pusti dugme misa u prozoru

	// Proverava da li je pritisnuto ili pusteno levo dugme misa.
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// Ako je levo dugme misa pritisnuto (GLFW_PRESS)
		if (action == GLFW_PRESS) {
			isLeftMouseButtonPressed = true; // Postavi flag da je levo dugme pritisnuto
		}
		// Ako je levo dugme misa pusteno (GLFW_RELEASE)
		else if (action == GLFW_RELEASE) {
			isLeftMouseButtonPressed = false; // Postavi flag da je levo dugme pusteno
		}
	}

	// Proverava da li je desno dugme misa pritisnuto ili pusteno
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		// Ako je desno dugme misa pritisnuto (GLFW_PRESS)
		if (action == GLFW_PRESS) {
			isRightMouseButtonPressed = true;	// Postavi flag da je desno dugme misa pritisnuto
		}
		// Ako je desno dugme misa pusteno (GLFW_RELEASE)
		else if (action == GLFW_RELEASE) {
			isRightMouseButtonPressed = false; // Postavi flag da je desno dugme misa pusteno
		}
	}
}

void processMouseHold(GLFWwindow* window) {
	// Poziva se kada je mis pritisnut i pomera se (drzi dugme misa).

	// Dohvatanje trenutne pozicije kursora misa u prozoru.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Dohvatanje velicine prozora (sirina i visina)
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	// Mapiranje pozicije kursora sa koordinatama prozora u opseg [-1, 1]
	// Normalizovanje X pozicije kursora u opseg [-1, 1] (koriscenje sirine prozora)
	float normalizedX = (xpos / width) * 2.0f - 1.0f;	// Mapiraj na [-1, 1]

	// Normalizovanje Y pozicije kursora u opseg [-1, 1], ali invertuje Y
	// (pocetak Y koordinata na vrhu prozora je uopsteno u grafici, ali mi zelimo da budu u donjem levom uglu) 
	float normalizedY = 1.0f - (ypos / height) * 2.0f;	// Invertuj Y i mapiraj na [-1, 1]

	// Pozivanje funkcije koja proverava da li je korisnik kliknuo ili presao preko odredjenog segmenta 
	checkSegmentClickAndHover(normalizedX, normalizedY);
}

void checkSegmentClickAndHover(float mouseX, float mouseY) {
	// Funkcija koja proverava da li je mis presao preko ili kliknuo na odredjeni segment
	// na osnovu normalizovanih koordinata misa (mouseX, mouseY)
	float bulevarSegmentDuzina = 0.25f; // duzina svakog segmenta bulevara (vertikalni pravougaonici)

	// Pozicije segmenata bulevara u smeru ka dole
	float segmentPositionsBulevarDole[][2] = {
		{ -0.02, 1.0},		//bulevar dole
		{-0.02, 0.75 },
		{-0.02, 0.5 },
		{-0.02, 0.25 },
		{-0.02, 0.0 },
		{-0.02, -0.25 },
		{-0.02, -0.5 },
		{-0.02, -0.75 },
	};

	// Pozicije segmenata bulevara u smeru ka gore
	float segmentPositionsBulevarGore[][2] = {
		{ 0.02, 1.0},		//bulevar gore
		{ 0.02, 0.75},
		{ 0.02, 0.5},
		{ 0.02, 0.25},
		{ 0.02, 0.0},
		{ 0.02, -0.25},
		{ 0.02, -0.5},
		{ 0.02, -0.75},
	};

	// Pozicije segmenata ulica sa leve strane
	float segmentPositionsUliceLevo[][2] = {
		{ -1.0, 0.75},		//ulice levo
		{ -1.0, 0.5},
		{ -1.0, 0.25},
		{ -1.0, 0.0},
		{ -1.0, -0.25},
		{ -1.0, -0.5},
		{ -1.0, -0.75},
	};

	// Pozicije segmenata ulica sa desne strane
	float segmentPositionsUliceDesno[][2] = {
		{ 1.0, 0.75},		//ulice desno
		{ 1.0, 0.5},
		{ 1.0, 0.25},
		{ 1.0, 0.0},
		{ 1.0, -0.25},
		{ 1.0, -0.5},
		{ 1.0, -0.75},
	};

	hoveredTexture = -1; // broj teksture koja je postavlja prilikom hovera na neki segment
	// kada je vrednost -1, nista nije hoverovano i nije postavljena nijedna tekstura

	// Iteracija kroz segmente bulevara koji idu dole
	for (int i = 0; i < 8; i++)	// bulevar dole; iteriramo do 8 jer imamo 7 segmenata
	{
		// Definisemo granice trenutnog segmenta
		float xMin = segmentPositionsBulevarDole[i][0] - 0.02;
		float xMax = xMin + 0.038; // sirina segmenta
		float yMax = segmentPositionsBulevarDole[i][1];
		float yMin = yMax - bulevarSegmentDuzina;

		// Proveravamo da li se mis nalazi unutar granica segmenta
		if (mouseX >= xMin && mouseX <= xMax && mouseY >= yMin && mouseY <= yMax) {
			printf("Bulevar dole segment %d hoverovan!\n", i);
			hoveredTexture = 0; // Postavljamo teksturu za hover

			// Povecavamo prigusenost ako je levi taster misa pritisnut
			if (isLeftMouseButtonPressed && prigusenostBulevarDole[i] < 1)
			{
				// povecaj prigusenost bulevara dole; mora biti globalna promenljiva
				prigusenostBulevarDole[i] += brzinaPromenePrigusenosti;
				return; // zaustavljamo dalju obradu
			}

			// Smanjujemo prigusenost ako je desni taster misa pritisnut
			if (isRightMouseButtonPressed && prigusenostBulevarDole[i] > 0)
			{
				// smanji prigusenost bulevara dole; mora biti globalna promenljiva
				prigusenostBulevarDole[i] -= brzinaPromenePrigusenosti;
				return; // zaustavljamo dalju obradu
			}
		}
	}

	// slican proces za segmente desne trake bulevara (vozila idu u smeru ka gore)
	for (int i = 0; i < 8; i++) // bulevar gore
	{
		float xMin = segmentPositionsBulevarGore[i][0] - 0.02;
		float xMax = xMin + 0.038;
		float yMax = segmentPositionsBulevarGore[i][1];
		float yMin = yMax - bulevarSegmentDuzina;

		if (mouseX >= xMin && mouseX <= xMax && mouseY >= yMin && mouseY <= yMax)
		{
			printf("Bulevar dole segment %d hoverovan!\n", i);
			hoveredTexture = 0;

			// povecaj prigusenost bulevara gore; mora biti globalna promenljiva
			if (isLeftMouseButtonPressed && prigusenostBulevarGore[i] < 1)
			{
				prigusenostBulevarGore[i] += brzinaPromenePrigusenosti;
				return;
			}

			if (isRightMouseButtonPressed && prigusenostBulevarGore[i] > 0)
			{
				prigusenostBulevarGore[i] -= brzinaPromenePrigusenosti;
				return;
			}
		}
	}

	// iteracija kroz horizontalne segmente ulica na levoj strani
	for (int i = 0; i < 7; i++)	// ulice levo
	{
		float xMin = segmentPositionsUliceLevo[i][0];
		float xMax = segmentPositionsUliceLevo[i][0] + 0.95; // duzina segmenta
		float yMax = segmentPositionsUliceLevo[i][1] + 0.02;
		float yMin = yMax - 0.04; // visina segmenta

		if (mouseX >= xMin && mouseX <= xMax && mouseY >= yMin && mouseY <= yMax)
		{
			printf("Ulica levo %d hoverovana!\n", i);
			hoveredTexture = i + 1;

			if (isLeftMouseButtonPressed && prigusenostUliceLevo[i] < 1)
			{
				prigusenostUliceLevo[i] += brzinaPromenePrigusenosti;
				return;
			}

			if (isRightMouseButtonPressed && prigusenostUliceLevo[i] > 0) {}
			{
				prigusenostUliceLevo[i] -= brzinaPromenePrigusenosti;
				return;
			}
		}
	}

	// Iteracija kroz horizontalne segmente ulica na desnoj strani
	for (int i = 0; i < 7; i++) // ulice desno
	{
		float xMin = segmentPositionsUliceDesno[i][0] - 0.95;
		float xMax = segmentPositionsUliceDesno[i][0];
		float yMax = segmentPositionsUliceDesno[i][1] + 0.02;
		float yMin = yMax - 0.04;

		if (mouseX >= xMin && mouseX <= xMax && mouseY >= yMin && mouseY <= yMax)
		{
			printf("Ulica desno %d hoverovana!\n", i);
			hoveredTexture = i + 1;

			if (isLeftMouseButtonPressed && prigusenostUliceDesno[i] < 1)
			{
				prigusenostUliceDesno[i] += brzinaPromenePrigusenosti;
				return;
			}

			if (isRightMouseButtonPressed && prigusenostUliceDesno[i] > 0)
			{
				prigusenostUliceDesno[i] -= brzinaPromenePrigusenosti;
				return;
			}
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// Provera da li je skrolovanje izvrseno na gore
	if (yoffset > 0)
	{
		// Skrolovanjem na gore povecavamo brzinu promene prigusenosti
		brzinaPromenePrigusenosti += 0.0001f;
	}
	// Provera da li je skrolovanje izvrseno na dole
	else if (yoffset < 0)
	{
		// Skrolovanjem na dole smanjujemo brzinu promene prigusenosti
		brzinaPromenePrigusenosti -= 0.0001f;
	}

	// Ogranicenje brzine promene prigusenosti na opseg izmedju 0.0 i 1.0
	if (brzinaPromenePrigusenosti < 0.0f)
	{
		// Ako je brzina manja od 0.0, postavi je na minimalnu dozvoljenu vrednost 0.0
		brzinaPromenePrigusenosti = 0.0f;
	}
	else if (brzinaPromenePrigusenosti > 1.0f)
	{
		// Ako je brzina veca od 1.0, postavi je na maksimalnu dozvoljenu vrednost 1.0
		brzinaPromenePrigusenosti = 1.0f;
	}

	// Ispis u konzolu za prikaz trenutne vrednosti brzine promene prigusenosti
	printf("Brzina promene prigusenosti: %f\n", brzinaPromenePrigusenosti);
}

// Kod ove funkcije je preuzet sa vezbi.
// Funkcija za kompajliranje OpenGL sejdera iz izvornog koda
unsigned int compileShader(GLenum type, const char* source)
{
	// Inicijalizacija stringova za citanje sadrzaja sejder fajla
	std::string content = "";
	std::ifstream file(source); // Otvaranje fajla sa prosledjenom putanjom
	std::stringstream ss;

	// Provera da li je fajl uspesno otvoren
	if (file.is_open())
	{
		// Citanje sadrzaja fajla u string stream
		ss << file.rdbuf();
		file.close();  // Zatvaranje fajla
		std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
	}
	else {
		// Ako fajl ne moze da se orvori, postavlja se prazan string
		ss << "";
		std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
	}

	// Konvertovanje sadrzaja fajla u string
	std::string temp = ss.str();
	const char* sourceCode = temp.c_str();  // Pretvaranje u pokazivac na niz karaktera (C-style string)

	// Kreiranje sejdera specificnog tipa (verteks ili fragment sejder)
	int shader = glCreateShader(type);

	// Deklaracija promenljivih za proveru uspesnosti kompajliranja i za log gresaka
	int success;
	char infoLog[512];

	// Povezivanje izvornog koda sa sejder objektom
	glShaderSource(shader, 1, &sourceCode, NULL);

	// Kompajliranje sejdera
	glCompileShader(shader);

	// Provera uspesnosti kompajliranja
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)  // Ako kompajliranje nije bilo uspesno
	{
		// Dobavljanje informacija o gresci
		glGetShaderInfoLog(shader, 512, NULL, infoLog);

		// identifikacija tipa sejdera koji ima gresku
		if (type == GL_VERTEX_SHADER)
			printf("VERTEX");  // ako je vertex sejder
		else if (type == GL_FRAGMENT_SHADER)
			printf("FRAGMENT");  // ako je fragment sejder

		// ispis poruke o gresci
		printf(" sejder ima gresku! Greska: \n");
		printf(infoLog); // ispis detaljnog opisa greske
	}

	// Vracanje ID-a kompajliranog sejdera
	return shader;
}

// Kod ove funkcije je preuzet sa vezbi.
// Funkcija za kreiranje i povezivanje vertex i fragment sejdera u jedan OpenGL program
unsigned int createShader(const char* vsSource, const char* fsSource)
{
	// Promenljive za ID-ove sejdera i programa
	unsigned int program;
	unsigned int vertexShader;
	unsigned int fragmentShader;

	// Kreiranje sejder programa (objekat koji sadrzi objedinjene sejdere)
	program = glCreateProgram();

	// Kompajliranje vertex sejdera
	vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);

	// Kompajliranje fragment sejdera
	fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

	// Dodavanje (prikljucivanje) kompajliranih sejdera u sejder program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// Linkovanje sejder programa (povezivanje sejdera u funkcionalnu celinu)
	glLinkProgram(program);

	// Validacija sejder programa (provera da li sejder program moze da se koristi)
	glValidateProgram(program);

	// Provera uspesnosti validacije
	int success;
	char infoLog[512];
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (success == GL_FALSE)  // Ako validacija nije uspesna
	{
		// Dobavljanje loga sa greskama
		glGetShaderInfoLog(program, 512, NULL, infoLog);
		// Ispis poruke o gresci
		std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
		std::cout << infoLog << std::endl;
	}

	// Odvajanje (detach) sejdera od programa
	// Nakon sto su sejderi linkovani, mogu se ukloniti jer ih program vise ne koristi direktno
	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);  // Brisanje vertex sejdera


	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);  // Brisanje fragment sejdera

	// Vracanje ID-a kreiranog sejder programa
	return program;
}

// Kod ove funkcije je preuzet sa vezbi.
// Funkcija za ucitavanje slike iz fajla i njeno pretvaranje u OpenGL teksturu
static unsigned loadImageToTexture(const char* filePath) {
	// Promenljive za dimenzije teksture i broj kanala boja
	int TextureWidth;
	int TextureHeight;
	int TextureChannels;

	// Ucitavanje slike sa putanje koristeci stbi_load (biblioteka stb_image)
	// stbi_load vraca pokazivac na sirove podatke slike i popunjava dimenzije i broj kanala
	unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);

	// Provera da li je slika uspesno ucitana
	if (ImageData != NULL)
	{
		//Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
		stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

		// Provjerava koji je format boja ucitane slike
		GLint InternalFormat = -1;
		switch (TextureChannels) {
		case 1: InternalFormat = GL_RED; break;
		case 2: InternalFormat = GL_RG; break;
		case 3: InternalFormat = GL_RGB; break;
		case 4: InternalFormat = GL_RGBA; break;
		default: InternalFormat = GL_RGB; break;
		}

		// Generisanje OpenGL teksture
		unsigned int Texture;
		glGenTextures(1, &Texture);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
		glBindTexture(GL_TEXTURE_2D, 0);
		// oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
		stbi_image_free(ImageData);
		return Texture;
	}
	else
	{
		std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
		stbi_image_free(ImageData);
		return 0;
	}
}