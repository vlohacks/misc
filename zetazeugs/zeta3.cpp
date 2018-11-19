#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <SFML/Graphics.hpp>

typedef signed char    s8;
typedef unsigned char  u8;
typedef short          s16;
typedef unsigned short u16;
typedef int            s32;
typedef unsigned int   u32;
typedef long long      s64;
typedef unsigned long long  u64;

double g_pi = 3.1415926535897932;

__complex__ double cexp(__complex__ double z)
{
  __complex__ double rv;

  rv = exp(__real__ z) * (cos(__imag__ z) + (1.0fi * sin(__imag__ z)));
  return(rv);
}

__complex__ double cpow(double b, __complex__ double e)
{
  __complex__ double rv;

  rv = cexp(log(b) * e);
  return(rv);
}

__complex__ double zeta(double si, u32 num_terms) 
{
	u32 i;
	double n;
	__complex__ double t;
	__complex__ double s = 0.5 + (si * 1.0i);
	__complex__ double sum, t1, m;

	m = 1.0 / (1.0 - cpow(2.0, (1.0 - s)));
	n = 1.0;
	t = cpow(n, 0.0 - s);
	sum = t;

	for (i = 0; i < num_terms; ++i) {
		n += 1.0;
		t1 = cpow(n, 0.0 - s);
		n += 1.0;
		t = cpow(n, 0.0 - s) - t1;
		sum += t;
	}
	return m * sum;
}

int main (int argc, char** argv) 
{
	const int wsize = 499;
	const int center = wsize / 2;
	const int dotradius = 3.0f;

	wchar_t str[128];

	u32 j;

	sf::Font font;
	if(!font.loadFromFile("UbuntuMono-R.ttf")) {
		fprintf(stderr, "error loading font...\n");
		return -1;
	}


	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(15);
	text.setFillColor(sf::Color::Black);
	
	sf::RenderWindow window(sf::VideoMode(wsize,wsize), "zeta", sf::Style::Close );
	sf::RenderTexture canvas;


	canvas.create(wsize,wsize);
	canvas.clear(sf::Color::White);

	double i;
	__complex__ double z;
	double zi, zr, zb;
	double zbr, lzbr;
	double ir, lir;
	u32 nz = 0;

	sf::CircleShape shape(dotradius);
	shape.setFillColor(sf::Color(0,0,192));

	sf::RectangleShape rectangle(sf::Vector2f(static_cast<float>(wsize), static_cast<float>(wsize)));
	rectangle.setFillColor(sf::Color(1,1,1));

	sf::Vertex lineR[] = {
		sf::Vertex(sf::Vector2f(0.0f,static_cast<float>(center)), sf::Color(128,128,128)),
		sf::Vertex(sf::Vector2f(static_cast<float>(wsize),static_cast<float>(center)), sf::Color(128,128,128))
	};
	sf::Vertex lineI[] = {
		sf::Vertex(sf::Vector2f(static_cast<float>(center), 0.0f), sf::Color(128,128,128)),
		sf::Vertex(sf::Vector2f(static_cast<float>(center), static_cast<float>(wsize)), sf::Color(128,128,128))
	};

	const float scale = 50.0f;
	sf::RenderStates rs = sf::RenderStates(sf::BlendAdd);

	j = 0;
	i = 1.0f;
	while (window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event))	{
			if (event.type == sf::Event::Closed)
				window.close();
        	}

		z = zeta(i, 1000);
		zi = __imag__ z;
		zr = __real__ z;
		zb = sqrt(zi*zi + zr*zr);

		swprintf(str, 128, L"s        = % 5.3f\n"
				    "z = ζ(s) = % 7.5f+% 7.5fi\n"
				    "|z|      = % 7.5f\n"
				    "lz       = % 5.3f\n"
				    "nz       = % d", i, zi, zr, zb, lir, nz);

		text.setString(str);

		shape.setPosition(((zr*scale + center)-dotradius), ((zi*scale+center)-dotradius));
		if ((j&0b1111111) == 0)
			canvas.draw(rectangle, rs);
		canvas.draw(lineR, 2, sf::Lines);
		canvas.draw(lineI, 2, sf::Lines);
		canvas.draw(shape);

		const sf::Texture& canvastex = canvas.getTexture();
		sf::Sprite sprite(canvastex);
        	window.draw(sprite);
		window.draw(text);
           	window.display();	   

		i += 0.0001; 

		if (zb < 0.01) {
			ir = (int)(i * 10.0) / 10.0;
			if (ir != lir) {
				lir = ir;
				nz++;
				printf("s=%5.15f |ζ(s)|=%5.15f\n", i, zb);
			}

		}

		j += 1;
	}


}



