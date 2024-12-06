#version 330 core

out vec4 outCol; // izlazna boja fragmenta u RGBA formatu

uniform vec3 uColor; // uniform promenljiva, definise boju kao vektor RGB

void main()
{
	outCol = vec4(uColor, 1.0); // postavljanje izlazne boje fragmenta
}

// Kod je preuzet sa vezbi.