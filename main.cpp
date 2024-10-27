#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <stack>
#include "chip8.h"
#include <SFML/Graphics.hpp>
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef uint16_t OPCODE;
typedef uint32_t PIXEL;

const unsigned width = 64;
const unsigned height= 32;

int main(int argc, char* argv[]) {
    if(argc < 2){
        std::cout<< "Example usage:"
                << " ./chip8emulator [path-to-.ch8 file]\n";
    }
    Chip8Emulator<64,32> state{};
    std::string bin_path(argv[1]);
    state.load_game(bin_path);
    float frameRate = 60;
    int cpuFrequency = 1000;
    int opPerFrame = cpuFrequency / frameRate;
    float interval = 1.0f/frameRate;

    //Set up graphics
    sf::RenderWindow window(sf::VideoMode(width, height), "Chip8 Emulator");
    window.setFramerateLimit(frameRate);
    window.setKeyRepeatEnabled(false);
    sf::Texture texture;
    texture.create(width, height);
    // run the program as long as the window is open
    sf::Clock cl;
    cl.getElapsedTime().asSeconds();
    sf::Event e;
    while (window.isOpen())
    {

        while(window.pollEvent(e)){
            switch(e.type){
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    state.push_event(e);
                    break;
                case sf::Event::KeyReleased:
                    state.push_event(e);
                    break;
                default:
                    break;
            }
        }
        state.process_event();
        // clear the window with black color
        if(interval < cl.getElapsedTime().asSeconds()) {
            // check all the window's events that were triggered since the last iteration of the loop
            cl.restart();
            state.decrease_timer();
            for(int i = 0;i<opPerFrame;i++){
                if(state.emulate()!=0){
                    std::cerr << "error!" << std::endl;
                    return 0;
                }
            }
            window.clear(sf::Color::Black);
            texture.update(reinterpret_cast<const sf::Uint8 *>(state.get_canvas()));
            sf::Sprite sprite(texture);
            window.draw(sprite);
            window.display();
        }
    }
    return 0;
}
