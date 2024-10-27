//
// Created by Yile Hu on 10/24/24.
//
#include "chip8.h"
#include <exception>
RGBA::RGBA(BYTE r,BYTE g,BYTE b,BYTE a){
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}
RGBA::RGBA(PIXEL p){
memcpy(&rgba, &p, sizeof(PIXEL));
}
bool operator==(RGBA c1, RGBA c2){
    return c1.rgba[0] == c2.rgba[0] && c1.rgba[1] == c2.rgba[1] && c1.rgba[2] == c2.rgba[2]
           && c1.rgba[3] == c2.rgba[3];
}
BYTE* RGBA::byte_rep(){
    return rgba;
}
KeyboardInput::KeyboardInput(std::unordered_map<int, int>& map) {
    key_map = map;
    key_pressed.resize(map.size(), false);
    recent = -1;
}

bool KeyboardInput::is_pressed(int key) {
    return key_pressed[key];
}

bool KeyboardInput::press_key(int key) {
    recent = key;
    std::cerr << "Pressed: " << key << std::endl;
    return key_pressed[key_map[key]] = true;
}

bool KeyboardInput::release_key(int key) {
    std::cerr << "Released: " << key << std::endl;
    return key_pressed[key_map[key]] = false;
}

int KeyboardInput::get_recent() {
    return recent;
}

void KeyboardInput::clear_recent(){
    recent = -1;
}
Chip8CPU::Chip8CPU():memory(new BYTE[0xFFF]) {}
Chip8CPU::~Chip8CPU(){delete[] memory;}
void Chip8CPU::init(std::shared_ptr<BasicPixelCanvas<RGBA>> c, std::shared_ptr<KeyboardInput> i){
    canvas = c;
    input = i;
    pc = 0x200;
    I = 0x0;
    delay_timer = 0;
    sound_timer = 0;
    memset(&V, 0x0, 16);
    memcpy(memory + 0x50, &fontset, sizeof(fontset));
}
void Chip8CPU::load_game(std::string& path){
    std::ifstream game_bin(path, std::ios::in | std::ios::binary | std::ios::ate);
    if(game_bin.is_open()){
        long file_size = game_bin.tellg();
        game_bin.seekg(0, std::ios::beg);
        game_bin.read((char*) &memory[0x200], file_size);
    }else{
        throw std::runtime_error("Error: Failed to open " + path);
    }
}

void Chip8CPU::decrease_timer() {
    if(delay_timer > 0){
        --delay_timer;
    }
    if(sound_timer > 0){
        --sound_timer;
    }
}

int Chip8CPU::execute(){
    BYTE op1 = memory[pc];
    BYTE op2 = memory[pc+1];
    OPCODE instruction = (op1 << 8) | op2;
    std::cout << "EMULATING:" << std::hex << instruction << std::endl;
    WORD X = (instruction & 0x0F00) >> 8;
    WORD Y = (instruction & 0x00F0) >> 4;
    WORD N = instruction & 0x000F;
    WORD NN = instruction & 0x00FF;
    WORD NNN = instruction & 0x0FFF;
    pc+=2;
    int ret_code = 0;
    switch(instruction & 0xF000){
        case 0x0000:
            switch(N){
                case 0x0:
                    for(size_t i = 0;i<2048;i++) {
                        canvas->draw(i, Black);
                    }
                    break;
                case 0xE:
                    pc = stack.top();
                    stack.pop();
                    break;
                default:
                    ret_code = 1;
                    break;
            }
            break;
        case 0x1000:
            pc = NNN;
            break;
        case 0x2000:
            stack.push(pc);
            pc = NNN;
            break;
        case 0x3000:
        {
            pc += (V[X] == NN) << 1;
            break;
        }
        case 0x4000:
        {
            pc += (V[X] != NN) << 1;
            break;
        }
        case 0x5000:
        {
            pc += (V[X] == V[Y]) << 1;
            break;
        }
        case 0x6000:
        {
            V[X] = NN;
            break;
        }
        case 0x7000:
        {
            V[X]+=NN;
            break;
        }
        case 0x8000:
        {
            switch(N){
                case 0x0:
                    V[X] = V[Y];
                    break;
                case 0x1:
                    V[X] |= V[Y];
                    break;
                case 0x2:
                    V[X] &= V[Y];
                    break;
                case 0x3:
                    V[X] ^= V[Y];
                    break;
                case 0x4: {
                    BYTE tmp = V[X];
                    V[X] += V[Y];
                    V[0XF] = V[X] < tmp;
                    break;
                }
                case 0x5: {
                    BYTE tmp = V[X] >= V[Y];
                    V[X] -= V[Y];
                    V[0xF] = tmp;
                    break;
                }
                case 0x6: {
                    BYTE tmp = V[X] & 0x1;
                    V[X] >>= 1;
                    V[0xF] = tmp;
                    break;
                }
                case 0x7: {
                    BYTE tmp = V[Y] >= V[X];
                    V[X] = V[Y] - V[X];
                    V[0xF] = tmp;
                    break;
                }
                case 0xE: {
                    BYTE tmp = (V[X] & 0x8000) != 0;
                    V[X] <<= 1;
                    V[0xF] = tmp;
                    break;
                }
                default:
                    ret_code = 1;
                    break;
            }
            break;
        }
        case 0x9000:
            pc += (V[X] != V[Y]) << 1;
            break;
        case 0xA000:
            I = NNN;
            break;
        case 0xB000:
            pc = V[0] + NNN;
            break;
        case 0xC000:
            V[X] = random_WORD() & NN;
            break;
        case 0xD000:
        {
            WORD x = V[X];
            WORD y = V[Y];
            V[0xF] = 0;
            for(WORD i = 0;i<N;i++){
                for(WORD j = 0;j<8;j++){
                    if(memory[I + i] & (0x80 >> j)){
                        if(canvas->get_color(x+j,y+i) == White){
                            canvas->draw(x+j,y+i, Black);
                            V[0xF] = 1;
                        }else {
                            canvas->draw(x+j,y+i, White);
                        }
                    }
                }
            }
            break;
        }
        case 0xE000: {
            switch(N){
                case 0xE:
                   if(input->is_pressed(V[X])){
                        pc+=2;
                    }
                    //pc += (input->is_pressed(V[X]) << 1);
                    break;
                case 0x1:
                    if(!input->is_pressed(V[X])){
                        pc+=2;
                    }
                    //pc += (!input->is_pressed(V[X]) << 1);
                    break;
                default:
                    ret_code = 1;
                    break;
            }
            break;
        }
        case 0xF000:{
            switch(NN){
                case 0x07:
                    V[X] = delay_timer;
                    break;
                case 0x0A:
                    if(!waiting){
                        waiting = true;
                        input->clear_recent();
                    }
                    if(input->get_recent() == -1)
                        pc -= 2;
                    else
                        V[X] = input->get_recent();
                        waiting = false;
                    break;
                case 0x15:
                    delay_timer = V[X];
                    break;
                case 0x18:
                    sound_timer = V[X];
                    break;
                case 0x1E:
                    I += V[X];
                    break;
                case 0x29:
                    I = fontset[V[X] * 5];
                    break;
                case 0x33:
                    memory[I] = (BYTE)(V[X] / 100);
                    memory[I+1] = (BYTE)((V[X] % 100) / 10);
                    memory[I+2] = (BYTE)(V[X] % 10);
                    break;
                case 0x55:
                    for(int i = 0;i<=X;i++){
                        memory[I+i] = V[i];
                    }
                    break;
                case 0x65:
                    for(int i = 0;i<=X;i++){
                        V[i] = memory[I+i];
                    }
                    break;
                default:
                    ret_code = 1;
                    break;
            }
            break;
        }
        default:
            ret_code = 1;
            break;
    }
    return ret_code;
}
