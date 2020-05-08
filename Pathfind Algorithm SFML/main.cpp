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
		Body.setSize(sf::Vector2f(30, 30));
		Body.setOrigin(sf::Vector2f(15, 15));
		Body.setPosition(sf::Vector2f(x_ident * 31.80 + 30, y_ident * 31.80 + 30));
		Body.setOutlineColor(sf::Color::Black);
		Body.setOutlineThickness(1);
		
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
	// Here we have 30 x 31 nodes, 930 nodes in total. If you want less just change this settings and remember to adjust the size
	// that the nodes have in the class NodeBody (also their origin and the position they will be into)
	for (int i = 0; i != 30; i++)
	{
		for (int j = 0; j != 31; j++)
		{
			vector_casillas.emplace_back(j, i, true, false, false);
			vector_SFML_casillas.emplace_back(j, i, true, false, false);
		}
	}
	
	// Vectors used for storing the nodes in the calculating process.
	std::vector<Node> lista_abierta, lista_cerrada, lista_objetivo;
	
	// This vector is going to have the final path when everything has been calculated
	// from the starting node to the end node we want to go to.
	std::vector<Node> lista_trayecto;

	std::vector<Node>::iterator iter;

	// SFML Library Suff. The clock is only created because we want to paint a square after an interval (0.3s). If not put the path 
	// will be drawn immediately at the drawn, destroying the "movement" effect because it's instantaneous.
	sf::Clock myclock;
	sf::Event event;
	
	float time;
			
	
	
	while (window.isOpen())
	{
		if (start_working)
		{
			// We will put the starting position node in a vector and the end position node in another.
			// All nodes are still in the vector vector_casillas previously defined.
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

			// Iterate over and over again until the end node is in the lista_cerrada vector.
			while (!node_in_lista(*lista_objetivo.begin(), lista_cerrada))
			{

				// Find the neighbours of the element added to the lista_cerrada, and add them to lista_abierta for further examination.
				for (auto it = vector_casillas.begin(); it != vector_casillas.end(); it++)
				{
					// Performance boost. This if will only consider nodes that are direct neighbours of our first node.
					// How? The neighbours are at a distance less than 1 in both x and y.
					if (abs(it->x - lista_cerrada.back().x) <= 1 && abs(it->y - lista_cerrada.back().y) <= 1)
					{
						// If the neighbour candidate is an okey node = is not in lista_abierta or lista_cerrada vectors already.
						if (node_ok(*it, lista_cerrada, lista_abierta))
						{
							// Get it's distance from the present node, just its x and y, nothing more.
							int dist_x = std::sqrt(std::pow(lista_cerrada.back().x - it->x, 2));
							int dist_y = std::sqrt(std::pow(lista_cerrada.back().y - it->y, 2));

							// Only accept the nodes (squares) that are his neighbours (x and y distance <= 1) and are valid neighbours.
							
							// We don't want to accept diagonal movement. If we wanted to allow it, it would be enought to change next if to: 
							// if (dist_x <= 1 && dist_y <= 1)
							if ((dist_x <= 1 && dist_y == 0) || (dist_x == 0 && dist_y <= 1))
							{
								// Since this neighbour has been generated and accepted we must write into him who is his father.
								it->parent_x = lista_cerrada.back().x;
								it->parent_y = lista_cerrada.back().y;
								// Now I add this neighbour to lista_abierta. The calculations of g, h and f will take place with the
								// objects in lista abierta vector.
								lista_abierta.push_back(*it);
							}
							
						}
				}

				// We will calculte the g, h and f of each node previously added to the lista_abierta.
				for (auto it = lista_abierta.begin(); it != lista_abierta.end(); it++)
				{
					// h^2 = dist_x ^2 + dist_y ^2. Remember, in lista_cerrada we have only 1 item, the starting node right now. But we will add
					// candidates after that, then lista_cerrada.back will have the last one added.
					it->g = std::sqrt(std::pow(lista_cerrada.back().x - it->x, 2) + std::pow(lista_cerrada.back().y - it->y, 2));
					it->h = std::sqrt(std::pow(lista_objetivo.back().x - it->x, 2) + std::pow(lista_objetivo.back().y - it->y, 2));
					it->f = it->g + it->h;
				}

				// We will reorder the lista_abierta vector by the value of f. The lowest value will be found after using the lamdba function
				// will be in the lista_abierta.begin() position.
				std::sort(lista_abierta.begin(), lista_abierta.end(), [](const Node& A, const Node& B) {return A.f < B.f; });

				// Importat step!
				// Check if a solution is possible. We will reach the 'not possible' part when we have added all the elements to the lista_cerrada.
				// remaining the lista_abierta with a size of 0. Then we will exit the loop because it means we are trapped and clear everything afterwards.

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
				// We switch the first element (lowest f) for the last one (biggest f) just so we can use pop_back and take the element of the
				// lowest f out. 
				std::swap(*iter_lowest_f, lista_abierta.back()); 
				lista_abierta.pop_back(); 

			
				// Once the end node has been added to the lista_cerrada vector this loop will end because we have a solution. 
				// And we will start to look for the path to the first node from the last one added (the end node), using the parent 
				// stuff described in the variables of the class.
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
			// That's how we are going to find our way, sounds complex but it's pretty lame.

			// Our last node added to lista_cerrada is in fact the end node. Add that node into the first position in lista_trayecto, which is 
			// completly empty at this point.
			lista_trayecto.insert(lista_trayecto.begin(), lista_cerrada.back());

			// The parent stuff. The parent of the end node will have and x, y = end_node->parent_x, end_node->parent_y;
			int x_final_parent = lista_cerrada.back().parent_x;
			int y_final_parent = lista_cerrada.back().parent_y;
			// And so on through the whole chain. Each node has a parent that added him, we go backwards.

			// While the start node hasn't beed added to lista_trayecto, keep going backwards looking for the parents starting from the end node.
			// *lista_cerrada.being() is the first node that we added (the starting node) and it's still in the first position of that list.
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
			// If you closed the window
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
			// We want to hold space in order to also draw without clicking everytime on the nodes. You can perfectly comment out this 
			// if without any issue
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
			{
				if (button_signal == create_obstacles)
				{
					set_obstacles(vector_SFML_casillas, PosMouse2, vector_casillas);
				}
			}

			// Something that will always be available (without any signals), but only for the obstacles, is to remove them by making right 
			// click over them. (left click to add them, right to remove them)
			if (event.type == event.MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
			{
				PosMouse.x = static_cast<float>(sf::Mouse::getPosition(window).x);
				PosMouse.y = static_cast<float>(sf::Mouse::getPosition(window).y);

				delete_obstacles(vector_SFML_casillas, PosMouse, vector_casillas);
			}
		}

		// When start_path has completed it will activate start_drawing.
		if (start_drawing)
		{
			time = myclock.getElapsedTime().asSeconds();  // We get the time from the clock created at the begining.
			// The program is only allowed to draw the path after 0.03s each time, to make it look like it's moving and finding
			// it's way to the ending node. 
			// iter = lista_trayecto.begin() previously defined, so iterate until iter reaches the end of lista_trayecto vector.
			if (time >= 0.03 && iter < lista_trayecto.end())
			{
				// Now comes the part of connecting the nodes from the calculation to the nodes in the screen (remember they are not the same)
				// but they share the same x_identificator and y_identificator (0, 0), (1, 1), (7,3) just like in a matrix.
				int x_ident = iter->x;
				int y_ident = iter->y;
				// We are looking for the graphical node that corresponds to the node used in the calculations. iter_casilla will give us
				// an iterator to the corresponding graphical node, that we will easily paint. vecot_SFML_casillas contains our graphical nodes,
				// the ones from (NodeBody class) and lista_trayecto contains the path of nodes used for the calculation (the ones from Node class)
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
		// Buttons and text.
		window.draw(Start_button);
		window.draw(Restart_button);
		window.draw(ButtonText);
		window.draw(ButtonText2);

		window.display();

	}
	return 0;
}

// Overloading of ==, != done mainly to compare nodes and look if they are present in a vector or no. Easier to overload == and !=
// than to do it constantly in in and if (...)
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
	// If it has been found in lista_cerrada or lista_abierta that iterator will point to != vector.end() position
	// so return that the node_ok is not okey and shall not be used.
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
	// vector_casillas. vector_SFML_casillas is the one with the graphical nodes, we paint them black.
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
	// Since we are putting obstacles, uptade the nodes of the calculations to make them not transitable.
	// vector_casillas is the vector with the nodes that are going to be used for calculations.
	// If this is not done the algorithm won't know that the black graphical nodes are not transitable.
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

// Both functions, set_origin and set_end work in the same way. Check over which node (graphical one) the click has been made.
// change its color to blue or red and take its x_identificator and y_identificator, used afterwards to update the nodes with
// which the calculation will be made (the ones in vector_casillas). Each node in vector_SFML_casillas is linked to the nodes in
// vector_casillas by the x_ident and y_ident.
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
