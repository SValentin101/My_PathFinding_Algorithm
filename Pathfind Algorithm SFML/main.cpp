#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <ostream>
#include <istream>

// The clase Node will be each square (called node because it can be an hexagon or whatever we want). The objects of this class will be the 
// ones used to calculate the path, they will not take part in any drawings. That will be handed by NodeBody class objects, which will be
// the ones that we will draw on the screen and paint as blue (starting node), red (end node) or black (not transitable nodes)
class Node
{
public:
	Node(int x_pos, int y_pos, bool transit, bool start, bool finish) : x(x_pos), y(y_pos), transitable(transit), start_node(start),
		finish_node(finish) {}

	
	friend bool node_ok(const Node& node, std::vector<Node>, std::vector<Node>);
	friend bool node_in_lista(Node& node, std::vector<Node>);
	friend bool operator == (const Node&, const Node&);
	friend bool operator != (const Node& A, const Node& B);
	friend std::ostream& operator << (std::ostream&, const Node&);

	int x, y;
	int parent_x, parent_y;
	double g, h, f;

	bool transitable;
	bool start_node;
	bool finish_node;

};

class NodeBody
{
public:
	NodeBody(int x_ident, int y_ident, bool transit, bool start, bool finish)
	{
		this->x_ident = x_ident;
		this->y_ident = y_ident;
		Body.setSize(sf::Vector2f(75, 75));
		Body.setOrigin(sf::Vector2f(25, 25));
		Body.setPosition(sf::Vector2f(x_ident * 75 + 50, y_ident * 75 + 50));
		Body.setOutlineColor(sf::Color::Black);
		Body.setOutlineThickness(1.25);
		
		if (transit)
		{
			Body.setFillColor(sf::Color::White);
		}
		else
		{
			Body.setFillColor(sf::Color::Black);
		}
		if (start)
		{
			Body.setFillColor(sf::Color::Blue);
		}
		if (finish)
		{
			Body.setFillColor(sf::Color::Red);
		}


	}
	int x_ident, y_ident;
	sf::RectangleShape Body;

};


void set_obstacles(std::vector<NodeBody>&, sf::Vector2f, std::vector<Node>&);
void delete_obstacles(std::vector<NodeBody>&, sf::Vector2f, std::vector<Node>& );
void set_origin(std::vector<NodeBody>&, sf::Vector2f, std::vector<Node>&);
void set_end(std::vector<NodeBody>&, sf::Vector2f, std::vector<Node>&);


int main()
{
	// This vector will be the one where all the nodes used for calculations will be created into.
	std::vector<Node> vector_casillas;

	// In this vector we will create the squares that we will draw on screen. Both vectors are independent from each other.
	// But Node and NodeBody objects will share the same x,y. 
	std::vector<NodeBody> vector_SFML_casillas;

	// SFML Library Stuff for the main window.
	const int WINDOW_HEIGHT = 950;
	const int WINDOW_WIDTH = 1020;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 10;
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "A* Pathfinding", sf::Style::Close | sf::Style::Titlebar, settings);
	//
	
	// SFML Library Stuff for the buttons that we will have (only 2), and also an enum to cycle through what we want to do.
	enum button
	{
		starting_position = 0, end_position = 1, create_obstacles = 2, restart = 3
	};
	button button_signal = starting_position;

	sf::Vector2f button_size(70, 35);
	sf::RectangleShape Start_button;
	Start_button.setSize(button_size);
	Start_button.setOrigin(button_size.x / 2, button_size.y / 2);
	Start_button.setPosition(sf::Vector2f(975, 200));
	Start_button.setFillColor(sf::Color(77, 77, 77));
	Start_button.setOutlineColor(sf::Color::White);
	Start_button.setOutlineThickness(1);

	sf::RectangleShape Restart_button;
	Restart_button.setSize(button_size);
	Restart_button.setOrigin(button_size.x / 2, button_size.y / 2);
	Restart_button.setPosition(sf::Vector2f(975, 235));
	Restart_button.setFillColor(sf::Color(77, 77, 77));
	Restart_button.setOutlineColor(sf::Color::White);
	Restart_button.setOutlineThickness(1);
	//


	// SFML Library Stuff for the text that the buttons will have.
	sf::Text ButtonText;
	sf::Font myfont;
	myfont.loadFromFile("OldNewspaperTypes.ttf");
	ButtonText.setFont(myfont);
	ButtonText.setString("Go!");
	ButtonText.setFillColor(sf::Color::White);
	ButtonText.setCharacterSize(16);
	ButtonText.setPosition(963, 190);

	sf::Text ButtonText2;
	ButtonText2.setFont(myfont);
	ButtonText2.setString("Clear");
	ButtonText2.setFillColor(sf::Color::White);
	ButtonText2.setCharacterSize(16);
	ButtonText2.setPosition(956, 226);
	//

	// Main bools to jump to the different options. Once one is complete the next one will be true, and so on.
	bool start_working = false;
	bool start_path = false;
	bool start_drawing = false;

	// We will need a vector to hold the (x,y) position of the mouse when we click over a node or button.
	sf::Vector2f PosMouse;

	// Create the graphical representation of the nodes directly in vector_SFML_casillas
	// Create the 'real' nodes which we will be using for operations directly in vector_casillas vector.
	// For simplicity sake we will have a total of 144 nodes on the screen.
	for (int i = 0; i != 12; i++)
	{
		for (int j = 0; j != 12; j++)
		{
			vector_casillas.emplace_back(j, i, true, false, false);
			vector_SFML_casillas.emplace_back(j, i, true, false, false);
		}
	}

	std::vector<Node> lista_abierta, lista_cerrada, lista_objetivo;
	std::vector<Node> lista_trayecto;

	std::vector<Node>::iterator iter;

	// SFML Library Suff. The clock is only created because we want to paint a square after an interval (0.3s). If not put the path will be drawn
	// immediately at the drawn.
	sf::Clock myclock;
	sf::Event event;
	
	float time;
			
	
	
	while (window.isOpen())
	{
		if (start_working)
		{
			// We will put the starting position in a list and the end position in the other.
			for (auto it = vector_casillas.begin(); it != vector_casillas.end(); it++)
			{
				if (it->start_node)
				{
					lista_cerrada.push_back(*it);
				}
				if (it->finish_node)
				{
					lista_objetivo.push_back(*it);
				}
			}

			// Iterate over and over again until the end node is in the lista_cerrada list.
			while (!node_in_lista(*lista_objetivo.begin(), lista_cerrada))
			{

				// Find the neighbours of the element added to the lista_cerrada, and add them to lista_abierta for further examination.
				for (auto it = vector_casillas.begin(); it != vector_casillas.end(); it++)
				{
					// If the neighbour candidate is an okey node = is not in lista_abierta or lista cerrada already.
					if (node_ok(*it, lista_cerrada, lista_abierta))  
					{
						// Get it's distance from the present node.
						int dist_x = std::sqrt(std::pow(lista_cerrada.back().x - it->x, 2));
						int dist_y = std::sqrt(std::pow(lista_cerrada.back().y - it->y, 2));

						// Only accept the nodes (squares) that are his neighbours (x and y distance <= 1) and are valid neighbours.
						if (dist_x <= 1 && dist_y <= 1)
						{
							// Le digo quien es su padre.
							it->parent_x = lista_cerrada.back().x;
							it->parent_y = lista_cerrada.back().y;
							// Lo meto en la lista abierta
							lista_abierta.push_back(*it);
						}
					}
				}

				// We will calculte the g, h and f of each node previously added to the lista_abierta.
				for (auto it = lista_abierta.begin(); it != lista_abierta.end(); it++)
				{
					it->g = std::sqrt(std::pow(lista_cerrada.back().x - it->x, 2) + std::pow(lista_cerrada.back().y - it->y, 2));
					it->h = std::sqrt(std::pow(lista_objetivo.back().x - it->x, 2) + std::pow(lista_objetivo.back().y - it->y, 2));
					it->f = it->g + it->h;
				}

				// We will reorder the lista_abierta vector by the value of f. The lowest value will be found at the lista_abierta.begin() position.
				// using a lambda function. 
				std::sort(lista_abierta.begin(), lista_abierta.end(), [](const Node& A, const Node& B) {return A.f < B.f; });

				// Importat step!
				// Check is a solution is possible. We will reach the 'not possible' part when we have added all the elements to the lista_cerrada.
				// remaining the lista_abierta with a size of 0. Then exit the loop because you are trapped and clear everything afterwards.

				if (lista_abierta.size() == 0)
				{
					std::cout << "There is no possible solution! \n";
					start_working = false;
					lista_cerrada.clear();
					lista_abierta.clear();
					lista_objetivo.clear();
					lista_trayecto.clear();

					break;
				}
				//

				// If a solution is possible just continue. Take the element with the lowest f from lista_abierta, erase it, and put it in
				// the lista_cerrada vector.
				auto iter_lowest_f = lista_abierta.begin();

				lista_cerrada.push_back(*iter_lowest_f);
				std::swap(*iter_lowest_f, lista_abierta.back());
				lista_abierta.pop_back();

			
				// Once the end node has been added to the lista_cerrada vector this loop will end and we will start to look for the path
				// to the first node from the last one added (the end node), using the parent stuff described in the variables of the class.
				if (node_in_lista(*lista_objetivo.begin(), lista_cerrada))
				{
					start_working = false;
					start_path = true;
				}

			}
		}

		// If you have found a solution, then you can start looking at how to create the path.	
		if (start_path)
		{
			// We will start by going from the end node to the first node. Looking at the parents that each node has.
			// That's how we are going to find our way

			// Our last node added to lista_cerrada is in fact the end node. Add that node at the first position in lista_trayecto, which is 
			// completly empty at this point.
			lista_trayecto.insert(lista_trayecto.begin(), lista_cerrada.back());

			// The parent stuff. The parent of the end node will have and x, y = end_node->parent_x, end_node->parent_y;
			int x_final_parent = lista_cerrada.back().parent_x;
			int y_final_parent = lista_cerrada.back().parent_y;
			// And so on through the whole chain. Each node has a parent that added him.

			// While the start node hasn't beed added to lista_trayecto, keep going backwards looking for the parents starting from the end node.
			// *lista_cerrada.being() is the first node that we added, the starting node and it's still in the first position of that list.
			while (!node_in_lista(*lista_cerrada.begin(), lista_trayecto))
			{
				// Find parents.
				for (auto iter = lista_cerrada.begin(); iter != lista_cerrada.end(); iter++)
				{
					if (iter->x == x_final_parent && iter->y == y_final_parent)
					{
						// If you found it put it in the lista_trayecto, at the first position. So in the first iteration we will have
						// the end node in the position [1] and the parent of that node in position [0], and so on.
						lista_trayecto.insert(lista_trayecto.begin(), *iter);

						x_final_parent = iter->parent_x;
						y_final_parent = iter->parent_y;
					}
				}
			}
			// If end start node has been added to the lista_trayecto start the drawing phase.
			if (node_in_lista(*lista_cerrada.begin(), lista_trayecto))
			{
				start_path = false;
				start_drawing = true;
				iter = lista_trayecto.begin();
			}
		}

		// This while is to keep the window active, SFML stuff mainly.
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			// If a left click was made.
			if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				/*std::cout << "Left Click" << "\n";*/

				// We have to do the static_cast because getGlobalBounds().contains(PosMouse) only accepts type float.
				PosMouse.x = static_cast<float>(sf::Mouse::getPosition(window).x);
				PosMouse.y = static_cast<float>(sf::Mouse::getPosition(window).y);

				// Depending on what signal is active we will call diferrent functions.
				// Not all signals are buttons. Starting_position is a signal but not a button, and with it we will create the starting node.
				// Start_button and Restart_button are buttons with a signal.
				if (button_signal == create_obstacles)
				{
					set_obstacles(vector_SFML_casillas, PosMouse, vector_casillas);
				}
				if (button_signal == end_position)
				{
					set_end(vector_SFML_casillas, PosMouse, vector_casillas);
					button_signal = create_obstacles;
				}
				if (button_signal == starting_position)
				{
					set_origin(vector_SFML_casillas, PosMouse, vector_casillas);
					button_signal = end_position;
				}
				if (Start_button.getGlobalBounds().contains(PosMouse))
				{
					start_working = true;
				}
				if (Restart_button.getGlobalBounds().contains(PosMouse))
				{
					// Set everything to the default.
					start_working = false;
					start_path = false;
					start_drawing = false;

					button_signal = starting_position;

					lista_cerrada.clear();
					lista_abierta.clear();
					lista_objetivo.clear();
					lista_trayecto.clear();

					// Reset all colors and configurations in both vectors with the nodes.
					for (auto iter = vector_casillas.begin(); iter != vector_casillas.end(); iter++)
					{
						iter->transitable = true;
						iter->start_node = false;
						iter->finish_node = false;
					}
					for (auto iter = vector_SFML_casillas.begin(); iter != vector_SFML_casillas.end(); iter++)
					{
						iter->Body.setFillColor(sf::Color::White);
						iter->Body.setOutlineColor(sf::Color::Black);
					}
				}
			}

			// Something that will always be available (without any signals), but only for the obstacles, is to remove them by making right 
			// click over them.
			if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
			{
				/*std::cout << "Right Click" << "\n";*/

				PosMouse.x = static_cast<float>(sf::Mouse::getPosition(window).x);
				PosMouse.y = static_cast<float>(sf::Mouse::getPosition(window).y);

				delete_obstacles(vector_SFML_casillas, PosMouse, vector_casillas);
			}
		}

		// When start_path has completed it will activate start_drawing.
		if (start_drawing)
		{
			time = myclock.getElapsedTime().asSeconds();  // We get the time from the clock created at the begining.
			// The program is only allowed to draw the path after 0.03s each time, to make it look like it's moving.
			// iter = lista_trayecto.begin() previously defined.
			if (time >= 0.03 && iter < lista_trayecto.end())
			{
				int x_ident = iter->x;
				int y_ident = iter->y;
				// The way that the graphical nodes are connected to the nodes used for calculations is by x,y. They both share the same
				// coordinates, but only the graphical nodes are the ones being drawn. We are looking for the graphical nodes to paint them blue.
				auto iter_casilla = std::find_if(vector_SFML_casillas.begin(), vector_SFML_casillas.end(),
					[x_ident, y_ident](NodeBody& A) {return (A.x_ident == x_ident && A.y_ident == y_ident); });

				iter_casilla->Body.setFillColor(sf::Color::Blue);
				iter++;
				myclock.restart();
			}

		}
		// The background color.
		window.clear(sf::Color(140, 140, 140, 255));

		// Draw all the nodes (graphical ones)
		for (const auto &elements : vector_SFML_casillas)
		{
			window.draw(elements.Body);
		}

		window.draw(Start_button);
		window.draw(Restart_button);
		window.draw(ButtonText);
		window.draw(ButtonText2);

		window.display();

	}
	return 0;
}

// Overloading of ==, != done mainly to fast compare nodes.
bool operator == (const Node& A, const Node& B)
{
	return (A.x == B.x && A.y == B.y);
}
bool operator != (const Node& A, const Node& B)
{
	return !(A == B);
}
//

// Overloaing of << operator mainly done to ease the trouble of std::cout << Node while implementing the algorithm.
std::ostream& operator << (std::ostream& os, const Node& node)
{
	os << "Y: " << node.y << " X: " << node.x << " Trans: " << node.transitable << " Start: " << node.start_node << " Finish: " << node.finish_node
		<< " Parent Y: " << node.parent_y << " Parent X: " << node.parent_x;

	return os;
}
//

// Just to check if a node will be accepted for calculations.
bool node_ok(const Node& node, std::vector<Node> lista_cerrada, std::vector<Node> lista_abierta)
{
	// The node that we pass cannot be in lista_cerrada, abierta or transitable == false or be the starting node.
	auto iter = std::find(lista_cerrada.begin(), lista_cerrada.end(), node);
	auto iter2 = std::find(lista_abierta.begin(), lista_abierta.end(), node);

	if (iter != lista_cerrada.end() || iter2 != lista_abierta.end() || node.transitable == false || node.start_node == true)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool node_in_lista(Node& node, std::vector<Node> lista_abierta)
{
	auto iter2 = std::find(lista_abierta.begin(), lista_abierta.end(), node);
	if (iter2 != lista_abierta.end())
	{
		return true;
	}
	else
	{
		return false;
	}

}

void set_obstacles(std::vector<NodeBody>& vector_SFML_casillas, sf::Vector2f PosMouse, std::vector<Node>& vector_casillas)
{
	int x_ident_vector_nodos = 0;
	int y_ident_vector_nodos = 0;

	// Get the node(square) where we clicked. Change its color tu black and give me his x,y. Those x,y will be shared by one of the nodes in
	// vector_casillas.
	for (auto iter = vector_SFML_casillas.begin(); iter != vector_SFML_casillas.end(); iter++)
	{
		if (iter->Body.getGlobalBounds().contains(PosMouse))
		{
			iter->Body.setFillColor(sf::Color::Black);
			iter->Body.setOutlineColor(sf::Color::White);
			x_ident_vector_nodos = iter->x_ident;
			y_ident_vector_nodos = iter->y_ident;
		}
	}
	// Since we are putting obstacles, uptade the nodes to make them not transitable.
	for (auto iter = vector_casillas.begin(); iter != vector_casillas.end(); iter++)
	{
		if (x_ident_vector_nodos == iter->x && y_ident_vector_nodos == iter->y)
		{
			iter->transitable = false;
		}
	}

}
void delete_obstacles(std::vector<NodeBody>& vector_SFML_casillas, sf::Vector2f PosMouse, std::vector<Node>& vector_casillas)
{
	int x_ident_vector_nodos = 0;
	int y_ident_vector_nodos = 0;

	for (auto iter = vector_SFML_casillas.begin(); iter != vector_SFML_casillas.end(); iter++)
	{
		if (iter->Body.getGlobalBounds().contains(PosMouse))
		{
			iter->Body.setFillColor(sf::Color::White);
			iter->Body.setOutlineColor(sf::Color::Black);
			x_ident_vector_nodos = iter->x_ident;
			y_ident_vector_nodos = iter->y_ident;
		}
	}

	for (auto iter = vector_casillas.begin(); iter != vector_casillas.end(); iter++)
	{
		if (x_ident_vector_nodos == iter->x && y_ident_vector_nodos == iter->y)
		{
			iter->transitable = true;
		}
	}

}

// Both functions, set_origin and set_end word in the same way.
void set_origin(std::vector<NodeBody>& vector_SFML_casillas, sf::Vector2f PosMouse, std::vector<Node>& vector_casillas)
{
	int x_ident_vector_nodos = 0;
	int y_ident_vector_nodos = 0;

	for (auto iter = vector_SFML_casillas.begin(); iter != vector_SFML_casillas.end(); iter++)
	{
		if (iter->Body.getGlobalBounds().contains(PosMouse))
		{
			iter->Body.setFillColor(sf::Color::Blue);
			x_ident_vector_nodos = iter->x_ident;
			y_ident_vector_nodos = iter->y_ident;
		}
	}

	for (auto iter = vector_casillas.begin(); iter != vector_casillas.end(); iter++)
	{
		if (x_ident_vector_nodos == iter->x && y_ident_vector_nodos == iter->y)
		{
			iter->start_node = true;
		}
	}

}
void set_end(std::vector<NodeBody>& vector_SFML_casillas, sf::Vector2f PosMouse, std::vector<Node>& vector_casillas)
{
	int x_ident_vector_nodos = 0;
	int y_ident_vector_nodos = 0;

	for (auto iter = vector_SFML_casillas.begin(); iter != vector_SFML_casillas.end(); iter++)
	{
		if (iter->Body.getGlobalBounds().contains(PosMouse))
		{
			iter->Body.setFillColor(sf::Color::Red);
			x_ident_vector_nodos = iter->x_ident;
			y_ident_vector_nodos = iter->y_ident;
		}
	}

	for (auto iter = vector_casillas.begin(); iter != vector_casillas.end(); iter++)
	{
		if (x_ident_vector_nodos == iter->x && y_ident_vector_nodos == iter->y)
		{
			iter->finish_node = true;
		}
	}
}
