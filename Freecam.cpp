#include <Windows.h>
#include <thread>
#include <MinHook.h>
#include <filesystem>
#include <fstream>
#include <math.h>
#include "mem.hpp"
#define PI 3.14159

void hookKey();
void debugLogF(const std::string message);

bool enabled = false;
bool w, s, a, d, space, shift;
float speed = 0.5f;
uintptr_t moduleBase;
void init(HMODULE hModule){
    moduleBase = (uintptr_t)GetModuleHandle("Minecraft.Windows.exe");
    hookKey();

    while (true){
        auto player = Mem::ResolveMultiLvlPtr(moduleBase + 0x041FC2A8, {0x10, 0x50, 0x138, 0x0});
        auto camTickX = moduleBase + 0x13200F0;
        auto camTickY = moduleBase + 0x13200F8;
        auto camTickZ = moduleBase + 0x1320100;
        if (enabled && player){
            auto camX = Mem::ResolveMultiLvlPtr(moduleBase + 0x04209468, {0x0, 0x18, 0x88, 0xAD8, 0x0, 0xB60, 0x8, 0x510});
            auto camY = camX + 4;
            auto camZ = camY + 4;
            auto rotX = *(float*)(player + 0x13C);

            if (rotX + 90.0 > 180.0) rotX = (float)-180.0 + (float)fmod(rotX, 90.0);
            else rotX += 90;

            float moveX = 0, moveZ = 0;

            if (camTickX){
                Mem::Nop((BYTE*)camTickX, 8);
                Mem::Nop((BYTE*)camTickY, 8);
                Mem::Nop((BYTE*)camTickZ, 8);
            }

            if (space) *(float*) camY += speed;
            if (shift) *(float*) camY -= speed;
            
            if (w){
                moveX = (float)cos(rotX * -(PI / 180.0));
                moveZ = (float)sin(rotX * (PI / 180.0));
            }

            if (s){
                if (rotX + 180.0 > 180.0){
                    float tmpX = (float)(-180.0 + fmod(rotX, 180.0));
                    moveX = (float)cos((tmpX) * PI / 180.0);
                    moveZ = (float)sin((tmpX) * PI / 180.0);
                }
                else{
                    moveX = (float)cos((rotX + 180.0) * PI / 180.0);
                    moveZ = (float)sin((rotX + 180.0) * PI / 180.0);
                }
            }

            if (d){
                if (rotX + 90.0 > 180.0){
                    float tmpX = (float)(-180.0 + fmod(rotX, 90.0));
                    moveX = (float)cos((tmpX) * PI / 180.0);
                    moveZ = (float)sin((tmpX) * PI / 180.0);
                }
                else{
                    moveX = (float)cos((rotX + 90.0) * PI / 180.0);
                    moveZ = (float)sin((rotX + 90.0) * PI / 180.0);
                }
            }

            if (a){
                if (rotX - 90.0 < -180.0){
                    float tmpX = (float)(180.0 + fmod(rotX, 90.0));
                    moveX = (float)cos((tmpX) * PI / 180.0);
                    moveZ = (float)sin((tmpX) * PI / 180.0);
                }
                else{
                    moveX = (float)cos((rotX - 90.0) * PI / 180.0);
                    moveZ = (float)sin((rotX - 90.0) * PI / 180.0);
                }
            }

            *(float*)camX += moveX * speed;
            *(float*)camZ += moveZ * speed;
        }

        else{
            if (camTickX){
                Mem::Patch((BYTE*)camTickX, (BYTE*)"\xF3\x0F\x11\x86\x10\x05\x00\x00", 8);
                Mem::Patch((BYTE*)camTickY, (BYTE*)"\xF3\x0F\x11\x8E\x14\x05\x00\x00", 8);
                Mem::Patch((BYTE*)camTickZ, (BYTE*)"\xF3\x0F\x11\x96\x18\x05\x00\x00", 8);
            }
        }
        Sleep(2);
    }
}

typedef void(__thiscall* key)(uint64_t, bool);
key _key;
void key_Callback(uint64_t key, bool isDown){
    bool cancel = false;
    if (key == 0x43 && isDown) enabled = !enabled;
    if (enabled){
        if (key == 0x57){
            w = isDown;
            cancel = isDown;
        }
        if (key == 0x53){
            s = isDown;
            cancel = isDown;
        }
        if (key == 0x41){
            a = isDown;
            cancel = isDown;
        }
        if (key == 0x44){
            d = isDown;
            cancel = isDown;
        }
        if (key == VK_SPACE){
            space = isDown;
            cancel = isDown;
        }
        if (key == VK_SHIFT){
            shift = isDown;
            cancel = isDown;
        }
    }
    if (!cancel) _key(key, isDown);
}

void hookKey(){
    MH_Initialize();
    uintptr_t addr = moduleBase + 0x7AEF00;
    if(MH_CreateHook((void*)addr, &key_Callback, reinterpret_cast<LPVOID*>(&_key)) == MH_OK){
        MH_EnableHook((void*)addr);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved){
    switch(fdwReason){
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, hInstance, 0, 0);
        break;
    }
    return TRUE;
};
