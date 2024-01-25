//libraries
#include "Platform/Platform.hpp"

#include <cmath>
#include <deque>
#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>
#include <string>

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

// complex number class for dealing with fractal generates via them

struct complex
{
	double real;
	double imag;
};

// button class for easier dealing and creating buttons used in the app

class button
{
public:
	int x;
	int y;
	int width;
	int height;
	bool pressed = 0;
	sf::RectangleShape rectangle;

	// constructors

	button(int x_, int y_, int width_, int height_, int rectangle_border = 5)
	{
		x = x_;
		y = y_;
		width = width_;
		height = height_;
		rectangle.setPosition(x + rectangle_border, y + rectangle_border);
		rectangle.setSize(sf::Vector2f(width - 2 * rectangle_border, height - 2 * rectangle_border));
		rectangle.setOutlineThickness(rectangle_border);
	}
	button(sf::Vector2f position, sf::Vector2f size, int rectangle_border = 5)
	{
		x = position.x;
		y = position.y;
		width = size.x;
		height = size.y;
		rectangle.setPosition(x + rectangle_border, y + rectangle_border);
		rectangle.setSize(sf::Vector2f(width - 2 * rectangle_border, height - 2 * rectangle_border));
		rectangle.setOutlineThickness(rectangle_border);
	}

	// a function to detect if the button is pressed

	bool is_pressed(sf::Vector2i mouse_pos)
	{
		if (x < mouse_pos.x && mouse_pos.x < x + width)
		{
			if (y < mouse_pos.y && mouse_pos.y < y + height)
			{
				return true;
			}
		}
		return false;
	}

	// for then the button changes in size or location

	void update(int new_x, int new_y, int new_width, int new_height, int rectangle_border = 5)
	{
		x = new_x;
		y = new_y;
		width = new_width;
		height = new_height;
		rectangle.setPosition(x + rectangle_border, y + rectangle_border);
		rectangle.setSize(sf::Vector2f(width - 2 * rectangle_border, height - 2 * rectangle_border));
		rectangle.setOutlineThickness(rectangle_border);
	}
};

//
//  initialization of all the functions that this program uses
// their individual purposes are written below with their function definitions
//

uint mendel_iter(complex pos, uint max_iterations);
sf::Color colour_palette(uint iterations);
template <typename IterFunction, typename... Args>
sf::Texture generate(uint width, uint height, complex top_left, complex bottom_right, uint max_iterations, IterFunction iter_fun, Args... args);
uint mandelbrot_julia_iter(complex pos, uint max_iterations, complex point);
sf::Texture which(uint which_one, int width, int height, complex top_left, complex bottom_right, uint max_iterations, complex julia_param);
void update_julia_param(complex& julia_param, int width, int height, complex top_left, complex bottom_right, sf::Vector2i mouse_pos);
void zoom(complex& top_left, complex& bottom_right, int width, int height, sf::Vector2i mouse_pos, sf::Event event, double zoom, double& zoomlvl);
uint burning_ship_iter(complex pos, uint max_iterations);
uint burning_ship_julia_iter(complex pos, uint max_iterations, complex point);
std::string zoom_string(double zoom_lvl);
std::string com_to_nice_str(complex position);
void resizing(sf::RenderWindow& window, sf::Event& event, complex& top_left, complex& bottom_right, int& width, int& height, int& window_x, int& window_y);
int file_count(std::string path);
void save_image(sf::Texture& txt);
void reset_view(complex& top_left, complex& bottom_right, double& zoomlvl);

// function for calculating the number of iterations for some position pos in mandelbrot fractal

uint mendel_iter(complex pos, uint max_iterations)
{
	// some variables that are used in the function or are used 2 times to slightly speed up the process
	complex pos_copy = pos;
	complex square;
	square.real = pos.real * pos.real;
	square.imag = pos.imag * pos.imag;
	double xy = pos.imag * pos.real;

	uint iter = 0;

	// the formula for the mendelbrot fractal is z = z^2 + z0
	// z is a complex number
	// z0 is a starting number in this function 'pos'

	while (square.real + square.imag < 4 && iter < max_iterations) // condition for escaping
	{
		// calculating z^2 + z0
		double temp = square.real - square.imag + pos_copy.real;
		pos.imag = xy + xy + pos_copy.imag;
		pos.real = temp;
		iter++;
		//calculating the squared values for the check and the next iteration
		square.real = pos.real * pos.real;
		square.imag = pos.imag * pos.imag;
		xy = pos.imag * pos.real;
	}

	return iter;
}

// function for calculating the number of iterations for some position pos in julia version of mandelbrot fractal with some point

uint mandelbrot_julia_iter(complex pos, uint max_iterations, complex point)
{
	//some variables for slight optimization
	complex square;
	square.real = pos.real * pos.real;
	square.imag = pos.imag * pos.imag;
	double xy = pos.real * pos.imag;
	uint iter = 0;
	while (square.real + square.imag < 4 && iter < max_iterations) // escape condition
	{
		// z = z^2 + z_p
		double temp = square.real - square.imag + point.real;
		pos.imag = xy + xy + point.imag;
		pos.real = temp;
		iter++;
		// calculating the square for the check the next iteration
		square.real = pos.real * pos.real;
		square.imag = pos.imag * pos.imag;
		xy = pos.real * pos.imag;
	}

	return iter;
}

// function for calculating the number of iterations for some position pos of burning ship fractal

uint burning_ship_iter(complex pos, uint max_iterations)
{
	// come variables for slight performance increase
	complex copy = pos;
	complex square;
	square.real = pos.real * pos.real;
	square.imag = pos.imag * pos.imag;
	uint iter = 0;
	while (square.real + square.imag < 4 && iter < max_iterations)
	{
		// z = (|a| + |b|i)^2 + z0
		double temp = square.real - square.imag + copy.real;
		pos.imag = 2 * std::abs(pos.real) * std::abs(pos.imag) + copy.imag;
		pos.real = temp;
		iter++;
		// calculating the square for the check and the next iteration
		square.real = pos.real * pos.real;
		square.imag = pos.imag * pos.imag;
	}
	return iter;
}

// function for calculating the number of iterations for some position pos in julia version of burning ship fractal with some point

uint burning_ship_julia_iter(complex pos, uint max_iterations, complex point)
{
	// variables for slight performance increase
	uint iter = 0;
	complex square;
	square.real = pos.real * pos.real;
	square.imag = pos.imag * pos.imag;
	while (square.real + square.imag < 4 && iter < max_iterations)
	{
		// z = (|a| + |b|i)^2 + z_p
		double temp = square.real - square.imag + point.real;
		pos.imag = 2 * std::abs(pos.real) * std::abs(pos.imag) + point.imag;
		pos.real = temp;
		iter++;
		// calculating squares for the check and the next iteration
		square.real = pos.real * pos.real;
		square.imag = pos.imag * pos.imag;
	}
	return iter;
}

// function to change the number of iterations the functions above return to a color of the pixel

sf::Color colour_palette(uint iterations)
{
	sf::Color pixel;

	uint8_t colour = iterations;

	// simple colour palette

	pixel.r = colour;	  // red
	pixel.g = 2 * colour; // green
	pixel.b = 3 * colour; // blue
	pixel.a = 255;		  // alpha / opacity of this pixel

	return pixel;
}

// a template for passing functions with different number of variables into this function
template <typename IterFunction, typename... Args>

// function generating a texture that is later dispalyed to the user

sf::Texture generate(uint width, uint height, complex top_left, complex bottom_right, uint max_iterations, IterFunction iter_fun, Args... args)
{
	sf::Texture txt;
	txt.create(width, height);

	sf::Uint8* pixels = new sf::Uint8[width * height * 4]; // RGBA ( red green blue alpha ) color model is used by sf::texture

	complex delta;
	delta.real = (bottom_right.real - top_left.real) / (int)width;
	delta.imag = (top_left.imag - bottom_right.imag) / (int)height;

	complex pos = top_left;

	// looping through all the pixels in the application window
	for (uint x = 0; x < width; x++)
	{
		for (uint y = 0; y < height; y++)
		{
			// getting the number of iteration it takes for current point to escape
			uint iter = iter_fun(pos, max_iterations, args...);
			// calculating the index of the texture array of the current pixel
			int arr_pos = 4 * (width * y + x);
			// convering number of iteration to a colour
			sf::Color p = colour_palette(iter);
			// assigning values into the texture array
			pixels[arr_pos] = p.r;
			pixels[arr_pos + 1] = p.g;
			pixels[arr_pos + 2] = p.b;
			pixels[arr_pos + 3] = p.a;
			// moving onto the position of the next pixel
			pos.imag = pos.imag - delta.imag;
		}
		pos.real = pos.real + delta.real;
		pos.imag = top_left.imag;
	}
	// updating our texture with the array
	txt.update(pixels, width, height, 0, 0);

	delete[] pixels;

	return txt;
}

// a function to deretminate which fractal to generate

sf::Texture which(uint which_one, int width, int height, complex top_left, complex bottom_right, uint max_iterations, complex julia_param)
{
	sf::Texture fractal;

	switch (which_one)
	{
		case 0: // mandelbrot fractal
			fractal = generate(width, height, top_left, bottom_right, max_iterations, mendel_iter);
			break;
		case 1: // julia verion of the mendelbrot fractal
			fractal = generate(width, height, top_left, bottom_right, max_iterations, mandelbrot_julia_iter, julia_param);
			break;
		case 2: // burning ship fractal
			fractal = generate(width, height, top_left, bottom_right, max_iterations, burning_ship_iter);
			break;
		case 3: // julia version of the burning ship fractal
			fractal = generate(width, height, top_left, bottom_right, max_iterations, burning_ship_julia_iter, julia_param);
			break;
		default:
			break;
	}

	return fractal;
}

// function for updating the julia parameter used in some fractals upon clicking

void update_julia_param(complex& julia_param, int width, int height, complex top_left, complex bottom_right, sf::Vector2i mouse_pos)
{
	// calculating "lengths" of one pixel
	complex d;
	d.real = (bottom_right.real - top_left.real) / width;
	d.imag = (top_left.imag - bottom_right.imag) / height;

	julia_param.real = top_left.real + d.real * mouse_pos.x;
	julia_param.imag = top_left.imag - d.imag * mouse_pos.y;
}

// function for calculating new parameters upon zooming

void zoom(complex& top_left, complex& bottom_right, int width, int height, sf::Vector2i mouse_pos, sf::Event event, double zoom, double& zoomlvl)
{
	// calculating the size of area dispalyed by the window
	complex size;
	size.real = (top_left.real + bottom_right.real) / 2.;
	size.imag = (top_left.imag + bottom_right.imag) / 2.;

	size.real = size.real - top_left.real;
	size.imag = size.imag - bottom_right.imag;

	// scaling the size of the area depending of the zoom
	if (event.mouseWheelScroll.delta > 0) // zoom in
	{
		size.real = size.real / zoom;
		size.imag = size.imag / zoom;
		zoomlvl = zoomlvl * zoom;
	}
	else // zoom out
	{
		size.real = size.real * zoom;
		size.imag = size.imag * zoom;
		zoomlvl = zoomlvl / zoom;
	}
	// "lengths" of one pixel
	complex d;
	d.real = (bottom_right.real - top_left.real) / width;
	d.imag = (top_left.imag - bottom_right.imag) / height;

	complex new_center;
	new_center.real = top_left.real + mouse_pos.x * d.real;
	new_center.imag = top_left.imag - mouse_pos.y * d.imag;

	top_left.real = new_center.real - size.real;
	top_left.imag = new_center.imag + size.imag;

	bottom_right.real = new_center.real + size.real;
	bottom_right.imag = new_center.imag - size.imag;
}

//  function for writing a zoom lvl in a nice way
// from 0 to 100 a value with a few decimals points is displayed
// from 100 to 1e6 a whole number is displayed
// from 1e6 upwords numbers are displayed in scientific notation

std::string zoom_string(double zoom_lvl)
{
	std::string string_zoom;
	if (0 < zoom_lvl && zoom_lvl < 100)
	{
		string_zoom = std::to_string(zoom_lvl);
	}
	else if (100 < zoom_lvl && zoom_lvl < 1000000)
	{
		string_zoom = std::to_string((int)zoom_lvl);
	}
	else
	{
		int power = log(zoom_lvl) / log(10);
		double zoom_to_write = zoom_lvl / pow(10, power);

		string_zoom = std::to_string(zoom_to_write) + "e" + std::to_string(power);
	}
	return string_zoom;
}

// function changes complex number into a nice string

std::string com_to_nice_str(complex position)
{
	std::string positioninwork = std::to_string(position.real);
	if (position.imag >= 0)
	{
		positioninwork = positioninwork + " + ";
	}
	else
	{
		positioninwork = positioninwork + " - ";
	}
	positioninwork = positioninwork + std::to_string(std::abs(position.imag));
	return positioninwork;
}

// function for calculating new parameters upon resizing the window

void resizing(sf::RenderWindow& window, sf::Event& event, complex& top_left, complex& bottom_right, int& width, int& height, int& window_x, int& window_y)
{
	// "lengths" of one pixel
	complex d;
	d.real = (bottom_right.real - top_left.real) / width;
	d.imag = (top_left.imag - bottom_right.imag) / height;
	// the amount the window was resized by
	int d_x = event.size.width - width;
	int d_y = event.size.height - height;
	// which wall was rezised
	if (window_x == window.getPosition().x) // right wall
	{
		bottom_right.real = bottom_right.real + d.real * d_x;
	}
	else // left wall
	{
		top_left.real = top_left.real - d.real * d_x;
	}
	if (window_y == window.getPosition().y) // down wall
	{
		bottom_right.imag = bottom_right.imag - d.imag * d_y;
	}
	else // top wall
	{
		top_left.imag = top_left.imag + d.imag * d_y;
	}

	width = event.size.width;
	height = event.size.height;
	// window position on the screen
	window_x = window.getPosition().x;
	window_y = window.getPosition().y;

	//creating a rectangle and setting the view of the window to it to update the window
	sf::FloatRect visibleArea(0, 0, width, height);
	window.setView(sf::View(visibleArea));
}

//  function counting how many files are in a directory
// returns the number of files inside a directory or -1 when an error occured

int file_count(std::string path)
{
	const char* directory = path.c_str();
	DIR* directory_pointer;
	int i = 0;
	directory_pointer = opendir(directory);

	if (directory_pointer != NULL)
	{
		while (readdir(directory_pointer))
			i++;

		closedir(directory_pointer);
	}
	else
	{
		i = -1;
	}

	return i;
}

// function for saving currently dispaled fractal as a picture

void save_image(sf::Texture& txt)
{
	std::string path = "./pictures/";
	int count = file_count(path) - 1; // on windows there are 2 hidden(?) files and we don't want to start counting from 2
	if (count != -1)
	{
		std::string name = path + "image_" + std::to_string(count) + ".png";
		sf::Image to_save = txt.copyToImage();
		to_save.saveToFile(name);
	}
}

// function to reset the view back the begining

void reset_view(complex& top_left, complex& bottom_right, double& zoomlvl)
{
	zoomlvl = 1;

	top_left.real = -2;
	top_left.imag = 2;

	bottom_right.real = 2;
	bottom_right.imag = -2;
}

int main()
{
	//loading the font
	sf::Font roboto;
	roboto.loadFromFile("src\\Roboto-Black.ttf");

	//
	// setting up all the buttons and their descriptions
	//

	sf::Text zoomtxt;
	zoomtxt.setFont(roboto);
	zoomtxt.setCharacterSize(30);
	zoomtxt.setStyle(sf::Text::Regular);
	zoomtxt.setString("Zoom: 1");
	zoomtxt.setPosition(7, 0);

	sf::Text position;
	position.setFont(roboto);
	position.setCharacterSize(30);
	position.setStyle(sf::Text::Regular);
	position.setString("Position: \n0 + 0i");
	position.setPosition(7, 35);

	sf::Text julia_parameter;
	julia_parameter.setFont(roboto);
	julia_parameter.setCharacterSize(30);
	julia_parameter.setStyle(sf::Text::Regular);
	julia_parameter.setString("Julia Parameter: \n0 + 0i");
	julia_parameter.setPosition(7, 105);

	button options_panel(-5, -5, 325, 505);
	options_panel.rectangle.setFillColor(sf::Color(69, 69, 69, 255));
	options_panel.rectangle.setOutlineColor(sf::Color(164, 164, 164, 255));

	button menel_buttn(0, 200, 110, 110 / 1.618);
	menel_buttn.rectangle.setFillColor(sf::Color(255, 0, 0, 255));
	menel_buttn.rectangle.setOutlineColor(sf::Color(201, 0, 0, 255));

	sf::Text menel_buttn_text;
	menel_buttn_text.setFont(roboto);
	menel_buttn_text.setCharacterSize(15);
	menel_buttn_text.setStyle(sf::Text::Regular);
	menel_buttn_text.setString("Mendel Brot\nFractal");
	menel_buttn_text.setPosition(10, 215);

	button mendel_julia_button(140, 200, 110, 110 / 1.618);
	mendel_julia_button.rectangle.setFillColor(sf::Color(255, 0, 155, 255));
	mendel_julia_button.rectangle.setOutlineColor(sf::Color(201, 0, 101, 255));

	sf::Text mendel_julia_button_text;
	mendel_julia_button_text.setFont(roboto);
	mendel_julia_button_text.setCharacterSize(15);
	mendel_julia_button_text.setStyle(sf::Text::Regular);
	mendel_julia_button_text.setString("Mendel Brot\nJulia Fractal");
	mendel_julia_button_text.setPosition(149, 215);

	button burning_shop_button(0, 300, 110, 110 / 1.618);
	burning_shop_button.rectangle.setFillColor(sf::Color(0, 0, 255, 255));
	burning_shop_button.rectangle.setOutlineColor(sf::Color(0, 0, 201, 255));

	sf::Text burning_shop_button_text;
	burning_shop_button_text.setFont(roboto);
	burning_shop_button_text.setCharacterSize(15);
	burning_shop_button_text.setStyle(sf::Text::Regular);
	burning_shop_button_text.setString("Burning Ship\nFractal");
	burning_shop_button_text.setPosition(10, 315);

	button burning_ship_julia_button(140, 300, 110, 110 / 1.618);
	burning_ship_julia_button.rectangle.setFillColor(sf::Color(0, 155, 255, 255));
	burning_ship_julia_button.rectangle.setOutlineColor(sf::Color(0, 101, 201, 255));

	sf::Text burning_ship_julia_text;
	burning_ship_julia_text.setFont(roboto);
	burning_ship_julia_text.setCharacterSize(15);
	burning_ship_julia_text.setStyle(sf::Text::Regular);
	burning_ship_julia_text.setString("Burning Ship\nJulia Fractal");
	burning_ship_julia_text.setPosition(150, 315);

	button help_button(0, 400, 110, 100 / 1.618);
	help_button.rectangle.setFillColor(sf::Color(255, 155, 0, 255));
	help_button.rectangle.setOutlineColor(sf::Color(201, 101, 0, 255));

	sf::Text help_button_text;
	help_button_text.setFont(roboto);
	help_button_text.setCharacterSize(15);
	help_button_text.setStyle(sf::Text::Regular);
	help_button_text.setString("Help");
	help_button_text.setPosition(37, 420);

	button help_panel(200, 200, 400, 400);
	help_panel.rectangle.setFillColor(sf::Color(155, 155, 0, 255));
	help_panel.rectangle.setOutlineColor(sf::Color(101, 101, 0, 255));

	sf::Text help_panel_text;
	help_panel_text.setFont(roboto);
	help_panel_text.setCharacterSize(15);
	help_panel_text.setStyle(sf::Text::Regular);
	help_panel_text.setString("Keys:\nh  - hide/enable side panel\nf1 - help(this)\nr - come back to the starting view\n\nMouse:\nleft mouse button - set the position of\n\t\t\t\t\t\t\t\t\tJulia Parameter\nscroll - zoom in/out\n\t\t\t  When zooming the place of the cursor\n\t\t\t  becames the middle of the screen.\n\nPictures are saved into pictures folder\nsaved pictures are named:\nimage_{number of pictures in the folder + 1}.png\n\nTo exit this panel press outside of it");
	help_panel_text.setPosition(210, 210);

	button save_button(140, 400, 110, 100 / 1.618);
	save_button.rectangle.setFillColor(sf::Color(155, 0, 255, 255));
	save_button.rectangle.setOutlineColor(sf::Color(101, 0, 201, 255));

	sf::Text save_button_text;
	save_button_text.setFont(roboto);
	save_button_text.setCharacterSize(15);
	save_button_text.setStyle(sf::Text::Regular);
	save_button_text.setString("Save Image");
	save_button_text.setPosition(153, 420);

	//
	//  initialization of parameters used in the application
	//

	bool ui_visible = true;

	bool help_panel_visible = false;

	// which fractal is being displayed
	uint which_one = 0;

	double zoomlvl = 1;

	int width = 800;
	int height = 800;

	uint max_iterations = 255;

	sf::RenderWindow window(sf::VideoMode(width, height), "Eksplorator fraktali");

	int window_x = window.getPosition().x;
	int window_y = window.getPosition().y;

	complex top_left;
	top_left.real = -2;
	top_left.imag = 2;

	complex bottom_right;
	bottom_right.real = 2;
	bottom_right.imag = -2;

	complex julia_param;
	julia_param.real = 0;
	julia_param.imag = 0;

	// did something happen that needs updating the displayed fractal
	bool update = 1;

	sf::Texture fractal_txt; // texture can be built from an array
	sf::Sprite fractal;		 // sprite can be displayed

	while (window.isOpen())
	{
		//
		// event handling
		//

		sf::Event event;
		while (window.pollEvent(event))
		{
			// X button
			if (event.type == sf::Event::Closed)
				window.close();
			// mouse presses
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

				if (help_panel_visible)
				{
					if (!help_panel.is_pressed(mouse_pos))
					{
						help_panel_visible = 0;
					}
				}
				else
				{
					if (options_panel.is_pressed(mouse_pos) && ui_visible)
					{
						if (menel_buttn.is_pressed(mouse_pos))
						{
							which_one = 0;
							update = 1;
						}
						if (mendel_julia_button.is_pressed(mouse_pos))
						{
							which_one = 1;
							update = 1;
						}
						if (burning_shop_button.is_pressed(mouse_pos))
						{
							which_one = 2;
							update = 1;
						}
						if (burning_ship_julia_button.is_pressed(mouse_pos))
						{
							which_one = 3;
							update = 1;
						}
						if (help_button.is_pressed(mouse_pos))
						{
							help_panel_visible = 1;
						}
						if (save_button.is_pressed(mouse_pos))
						{
							save_image(fractal_txt);
						}
					}
					else
					{
						update_julia_param(julia_param, width, height, top_left, bottom_right, mouse_pos);

						julia_parameter.setString("Julia Parameter: \n" + com_to_nice_str(julia_param));

						if (which_one == 1 || which_one == 3)
						{
							update = 1;
						}
					}
				}
			}

			// Keyboard presses

			if (event.type == sf::Event::KeyPressed)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::H))
				{
					ui_visible = !ui_visible;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
				{
					help_panel_visible = !help_panel_visible;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
				{
					reset_view(top_left, bottom_right, zoomlvl);
					position.setString("Position: \n0 + 0i");
					zoomtxt.setString("Zoom: 1");
					update = 1;
				}
			}

			// mouse scroll wheel

			if (event.type == sf::Event::MouseWheelScrolled)
			{
				double zom = 1.1;
				sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

				zoom(top_left, bottom_right, width, height, mouse_pos, event, zom, zoomlvl);

				zoomtxt.setString("Zoom: " + zoom_string(zoomlvl));

				complex center;
				center.real = (top_left.real + bottom_right.real) / 2;
				center.imag = (top_left.imag + bottom_right.imag) / 2;

				position.setString("Position: \n" + com_to_nice_str(center));

				update = 1;
			}

			// resizing the window

			if (event.type == sf::Event::Resized)
			{
				resizing(window, event, top_left, bottom_right, width, height, window_x, window_y);

				complex center;
				center.real = (top_left.real + bottom_right.real) / 2;
				center.imag = (top_left.imag + bottom_right.imag) / 2;

				position.setString("Position: \n" + com_to_nice_str(center));

				help_panel.update(width / 2 - 200, height / 2 - 200, 400, 400); // help panel doesn't change size and is in the middle of the screen
				help_panel_text.setPosition(width / 2 - 190, height / 2 - 190);

				update = 1;
			}
		}
		// updating the displayed fractal
		if (update)
		{
			fractal_txt = which(which_one, width, height, top_left, bottom_right, max_iterations, julia_param);
			fractal.setTexture(fractal_txt);
			fractal.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(width, height)));
			update = 0;
		}

		//
		// drawing all the necessary stuff in the window
		//

		window.clear(sf::Color::Blue);

		window.draw(fractal);
		if (ui_visible)
		{
			window.draw(options_panel.rectangle);
			window.draw(zoomtxt);
			window.draw(position);
			window.draw(julia_parameter);
			window.draw(menel_buttn.rectangle);
			window.draw(menel_buttn_text);
			window.draw(mendel_julia_button.rectangle);
			window.draw(mendel_julia_button_text);
			window.draw(burning_shop_button.rectangle);
			window.draw(burning_shop_button_text);
			window.draw(burning_ship_julia_button.rectangle);
			window.draw(burning_ship_julia_text);
			window.draw(help_button.rectangle);
			window.draw(help_button_text);
			window.draw(save_button.rectangle);
			window.draw(save_button_text);
		}
		if (help_panel_visible)
		{
			window.draw(help_panel.rectangle);
			window.draw(help_panel_text);
		}
		window.display();
	}

	return 0;
}