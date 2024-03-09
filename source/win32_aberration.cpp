#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define global_variable static
#define local_persist   static
#define internal        static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 b32;

typedef float  f32;
typedef double f64;

global_variable int game_width_pixels = 960;
global_variable int game_height_pixels = 540;

#include "aberration.cpp"

#include <windows.h>
#include <Windowsx.h>

struct win32_offscreen_buffer{ 
    BITMAPINFO Info;
    void *Memory;
    int ScreenWidth; int ScreenHeight;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
    
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

Input input;

struct win32_window_dimension{
    int Width;
    int Height;
};

win32_window_dimension 
Win32GetWindowDimension(HWND Window){
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;  
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){
    if (Buffer->Memory){
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width  = Width;
    Buffer->Height = Height;
    
    Buffer->Info.bmiHeader.biSize        = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth       = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight      = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes      = 1;
    Buffer->Info.bmiHeader.biBitCount    = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
} 

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer *Buffer)
                          
{
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message){
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = WParam;
            bool WasDown = (LParam & (1 << 30)) != 0;
            bool IsDown  = ((LParam & (1 << 31)) == 0);
            if (WasDown && IsDown) break;
            
            switch (VKCode){
                case 'W':
                case VK_UP:{
                    input.up_key = IsDown;
                } break;
                
                case 'S':
                case VK_DOWN:{
                    input.down_key = IsDown;
                } break;
                
                case 'D':
                case VK_RIGHT:{
                    input.right_key = IsDown;
                } break;
                
                case 'A':
                case VK_LEFT:{
                    input.left_key = IsDown;
                } break;
                
                case 'G':{
                    input.g_key = IsDown;
                } break;
                
                case VK_SPACE:{
                    input.jump_key = IsDown;  
                } break;
            }
            
            /*
            if (VKCode == 'W'){
                if (!WasDown && IsDown){
                    printf("WPRESSED");
                } else if (WasDown && IsDown){
                    printf(" WDOWN");
                } else if (WasDown && !IsDown){
                    printf("WUP");
                }
                printf("\n");
            }
            */
            
            bool AltKeyWasDown = (LParam & (1 << 29)) != 0;
            if (VKCode == VK_F4 && AltKeyWasDown){
                GlobalRunning = false;
            }
        } break;
        
        case WM_LBUTTONDOWN:{
            input.mouse_left_key = 1;
        } break; 
        
        case WM_LBUTTONUP:{
            input.mouse_left_key = 0;
        } break;
        
        case WM_RBUTTONDOWN:{
            input.mouse_right_key = 1;
        } break; 
        
        case WM_RBUTTONUP:{
            input.mouse_right_key = 0;
        } break;
        
        case WM_MOUSEMOVE:{
            i32 xPos = GET_X_LPARAM(LParam); 
            i32 yPos = GET_Y_LPARAM(LParam);
            
            //printf("%d\n", xPos);
            
            input.mouse_screen_position = {(f32)xPos, (f32)(GlobalBackBuffer.ScreenHeight - yPos)};
        } break;
    
        case WM_SIZE:{
        } break;
        
        case WM_CLOSE:{
            GlobalRunning = false;
        } break;
        
        case WM_DESTROY:{
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:{
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            
            GlobalBackBuffer.ScreenWidth = Dimension.Width;
            GlobalBackBuffer.ScreenHeight = Dimension.Height;
            
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);
            
            //PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
  
            EndPaint(Window, &Paint);

        } break;
        
        default:{
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }

    return (Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
                     HINSTANCE PrevInstance,
                     LPSTR CommandLine,
                     int ShowCode)
 {
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    i64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    WNDCLASS WindowClass = {0};    
    
    Win32ResizeDIBSection(&GlobalBackBuffer, game_width_pixels, game_height_pixels);
    
    WindowClass.style       = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance   = Instance;
    
    WindowClass.lpszClassName = "ClassTrulyAberration";
    
#if DEBUG
    AttachConsole(ATTACH_PARENT_PROCESS);
    //for printf to work
    freopen("CONOUT$", "w", stdout);
#endif
    if (!RegisterClass(&WindowClass)){
        //printf("oshiblis");
        //@TODO: Handle error in class registration
    }
    HWND Window = CreateWindowExA(
                             0,
                             WindowClass.lpszClassName,
                             "And the world turns red",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             0,
                             0,
                             Instance,
                             0);
    if (!Window){
        //printf("oshibkaa");
        //@TODO: Handle error in window allocation
    }
    
    LARGE_INTEGER LastCounter;
    QueryPerformanceCounter(&LastCounter);

    i64 LastCycleCount = __rdtsc();
    
    f32 delta = 0;
    
    InitGame();
    
    GlobalRunning = true;
    while(GlobalRunning){
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
            if (Message.message == WM_QUIT){
                GlobalRunning = false;
            }
        
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
        screen_buffer GameBuffer = {};
        GameBuffer.Memory       = GlobalBackBuffer.Memory;
        GameBuffer.Width        = GlobalBackBuffer.Width;
        GameBuffer.Height       = GlobalBackBuffer.Height;
        GameBuffer.Pitch        = GlobalBackBuffer.Pitch;
        GameBuffer.ScreenWidth  = GlobalBackBuffer.ScreenWidth;
        GameBuffer.ScreenHeight = GlobalBackBuffer.ScreenHeight;
                
        GameUpdateAndRender(delta, input, &GameBuffer);
        
        HDC DeviceContext = GetDC(Window);
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);
        ReleaseDC(Window, DeviceContext);
        
        i64 EndCycleCount = __rdtsc();

        LARGE_INTEGER EndCounter;
        QueryPerformanceCounter(&EndCounter);

        i64 CyclesElapsed = EndCycleCount - LastCycleCount;
        i64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
        f32 MSPerFrame = (f32)(1000.0f * CounterElapsed) / (f32)PerfCountFrequency;
        delta = (f32)(1.0f * CounterElapsed) / (f32)PerfCountFrequency;
        f32 FPS = (f32)PerfCountFrequency / (f32)CounterElapsed;
        //Mega-cycles per frame
        f32 MCPF = (f32)CyclesElapsed / 1000.0f / 1000.0f;
        //printf("MS: %f, Fps: %f, MCpf: %f\n", MSPerFrame, FPS, MCPF);
        
        LastCycleCount = EndCycleCount;
        LastCounter = EndCounter;
    }                                 
    return (0);
}

