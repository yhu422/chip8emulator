//
// Created by Yile Hu on 10/24/24.
//

#ifndef CHIP8EMULATOR_CHIP8_H
#define CHIP8EMULATOR_CHIP8_H
#include <iostream>
#include <stack>
#include <fstream>
#include <string>
#include <cassert>
#include <queue>
#include <SFML/Graphics.hpp>
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef uint16_t OPCODE;
typedef uint32_t PIXEL;

static WORD random_WORD(){
    std::srand(std::time(nullptr)); // seed the random number generator
    WORD number = std::rand() % 256;
    return number;
}

template<class Color, unsigned int WIDTH=64, unsigned int HEIGHT=32>
class BasicPixelCanvas;
class KeyboardInput;
static constexpr uint8_t fontset[80] =
        {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

struct RGBA{
    static const unsigned int Size = 4;
    BYTE rgba[4];
    RGBA(BYTE r,BYTE g,BYTE b,BYTE a=0xFF);
    RGBA(PIXEL p);
    BYTE* byte_rep();
};
bool operator==(RGBA c1, RGBA c2);
static RGBA Black(0x00,0x00,0x00);
static RGBA White(0xFF,0xFF,0xFF);

class Chip8CPU{
public:
    Chip8CPU();
    ~Chip8CPU();
    void init(std::shared_ptr<BasicPixelCanvas<RGBA>>, std::shared_ptr<KeyboardInput>);
    void load_game(std::string&);
    int execute();
    void decrease_timer();
private:
    BYTE* memory;
    BYTE V[16];
    WORD I;
    WORD pc;
    std::stack<WORD> stack;
    WORD delay_timer;
    WORD sound_timer;
    std::shared_ptr<BasicPixelCanvas<RGBA>> canvas;
    std::shared_ptr<KeyboardInput> input;
    bool waiting = false;
};

class KeyboardInput{
public:
    explicit KeyboardInput(std::unordered_map<int, int>&);
    bool press_key(int);
    bool release_key(int);
    bool is_pressed(int);
    int get_recent();
    void clear_recent();
private:
    std::vector<bool> key_pressed;
    std::unordered_map<int, int> key_map;
    int recent;
};

template<class Color, unsigned int WIDTH, unsigned int HEIGHT>
class BasicPixelCanvas{
public:
    BasicPixelCanvas(){
        memset(canvas, 0, WIDTH * HEIGHT * Color::Size);
    }
    void draw(size_t x, size_t y, Color color){
        memcpy(&canvas[(y*WIDTH+x)*Color::Size], color.byte_rep(), Color::Size);
    }
    void draw(size_t pos, Color color){
        memcpy(&canvas[pos*Color::Size], color.byte_rep(), Color::Size);
    }
    Color get_color(size_t x, size_t y) const{
        return Color(*(reinterpret_cast<const PIXEL*>(&canvas[(y*WIDTH+x) * Color::Size])));
    }
    BYTE* get_canvas(){
        return &canvas[0];
    }
private:
    BYTE canvas[WIDTH * HEIGHT * Color::Size];
};

template<short WIDTH, short HEIGHT>
class Chip8Emulator{
private:
    std::shared_ptr<BasicPixelCanvas<RGBA, WIDTH, HEIGHT>> canvas;
    std::shared_ptr<KeyboardInput> keyboard;
    std::queue<sf::Event> event_queue;
    Chip8CPU cpu_state;
public:
    Chip8Emulator(){
        canvas = std::make_shared<BasicPixelCanvas<RGBA, WIDTH, HEIGHT>>();
        std::unordered_map<int, int> keyMap;
        keyMap.insert({sf::Keyboard::Key::X, 0});
        keyMap.insert({sf::Keyboard::Key::Num1, 1});
        keyMap.insert({sf::Keyboard::Key::Num2, 2});
        keyMap.insert({sf::Keyboard::Key::Num3, 3});
        keyMap.insert({sf::Keyboard::Key::Q, 4});
        keyMap.insert({sf::Keyboard::Key::W, 5});
        keyMap.insert({sf::Keyboard::Key::E, 6});
        keyMap.insert({sf::Keyboard::Key::A, 7});
        keyMap.insert({sf::Keyboard::Key::S, 8});
        keyMap.insert({sf::Keyboard::Key::D, 9});
        keyMap.insert({sf::Keyboard::Key::Z, 0xA});
        keyMap.insert({sf::Keyboard::Key::C, 0xB});
        keyMap.insert({sf::Keyboard::Key::Num4, 0xC});
        keyMap.insert({sf::Keyboard::Key::R, 0xD});
        keyMap.insert({sf::Keyboard::Key::F, 0xE});
        keyMap.insert({sf::Keyboard::Key::V, 0xF});
        keyboard = std::make_shared<KeyboardInput>(keyMap);
        cpu_state.init(canvas, keyboard);
    }
    //Emulate 1 cycle
    int emulate(){
        return cpu_state.execute();
    }
    BYTE* get_canvas(){
        return canvas->get_canvas();
    }
    void load_game(std::string& path){
        cpu_state.load_game(path);
    }

    void push_event(sf::Event& event){
        event_queue.push(event);
    }
    void process_event(){
        while(!event_queue.empty()) {
            auto event = event_queue.front();
            event_queue.pop();
            switch (event.type) {
                case sf::Event::KeyPressed:
                    keyboard->press_key(event.key.code);
                    break;
                case sf::Event::KeyReleased:
                    keyboard->release_key(event.key.code);
                    break;
                default:
                    break;
            }
        }
    }
    void decrease_timer(){
        cpu_state.decrease_timer();
    }
};
#endif //CHIP8EMULATOR_CHIP8_H
