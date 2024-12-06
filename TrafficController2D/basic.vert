#version 330 core

layout(location = 0) in vec2 inPos; // 2D pozicija verteksa, preuzima se iz VBO-a

uniform vec2 uPos; // uniform promenljiva koja definise pomeraj (offset) za verteks pozicije

uniform float uFixedPos; // uniform promenljiva koja predstavlja "fiksnu" x-koordinatu

void main()
{
	if (inPos.x != uFixedPos)	// Provera da li je x-koordinata trenutnog verteksa razlicita od fiksirane vrednosti
		// Ovo omogucava selektivno pomeranje samo odredjenih verteksa
		gl_Position = vec4(inPos + uPos, 0.0, 1.0); // ako je uslov ispunjen, verteks se pomera za UPos
	else
		gl_Position = vec4(inPos, 0.0, 1.0); // ako uslov nije ispunjen, ostaje na istoj poziciji
}

// Ovaj kod je preuzet sa vezbi i malo izmenjen kako bi se prilagodio potrebama projektnog zadatka.