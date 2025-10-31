/// SIDerurgy - Forge your favorite SID.
/// An ASID to MIDI/CV converter.
/// 
/// This is the main app entry point.
/// Author: DDT
/// 
/// https://github.com/0x444454/SIDerurgy
/// 
/// LICENSE
/// Creative Commons, CC BY
///
/// https ://creativecommons.org/licenses/by/4.0/deed.en
///

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdio.h>
#include <devpkey.h>
#include <setupapi.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

#include "SIDerurgy.h"


// int main(int argc, char** argv);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void print_usage(wstring& app_name) {
  wprintf(L"Usage:\n"
    L"  %ls [-h] [-d <level>] [-i <input_MIDI_device>] [-o <output_MIDI_device>]\n"
    L"\n"
    L"  Options:\n"
    L"  -h        : Prints usage info.\n"
    L"  -d <level>: Set debug level.\n"
    L"  -i <name> : The MIDI (ASID) input device name to match (case sensitive search).\n"
    L"  -o <name> : The MIDI output device name to match (case sensitive search).\n"
    L"\n"
    , app_name.c_str()
  );
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

  // Register the window class.
  const wchar_t CLASS_NAME[] = L"SID to MIDI";

  WNDCLASS wc = { };

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  RegisterClass(&wc);

  // Create the window.

  HWND hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    L"SIDerurgy",                   // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,

    NULL,       // Parent window    
    NULL,       // Menu
    hInstance,  // Instance handle
    NULL        // Additional application data
  );

  if (hwnd == NULL) {
    return 0;
  }

  // Don't show window (we don't have cool stuff to show in it yet).
  //ShowWindow(hwnd, nCmdShow);

  // Get or create a console (we love consoles :-).
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    // Launched from console.
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
  }
  else {
    // No parent console ? No problem !
    AllocConsole();
    HANDLE stdHandle;
    int hConsole;
    FILE* fp;
    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    hConsole = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
    fp = _fdopen(hConsole, "w");
    freopen_s(&fp, "CONOUT$", "w", stdout);
  }

  printf("\n");
  printf("SIDerurgy v0.2 build 20251028 - Forge your favorite SID.\n");
  printf("An ASID to MIDI/CV converter created by DDT.\n");
  printf("https://github.com/0x444454/SIDerurgy\n\n");

  // Debug level.
  int debug_level = 0;

  // App name (as launched).
  wstring app_name;

  // Input device.
  wstring name_prefix_in; // Match the first one.

  // Output device.
  wstring name_prefix_out;  // Match the first one.
  //string name_prefix_out = "Microsoft";
  //string name_prefix_out = "loopMIDI";



  // Parse params.
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argv) return 1; // 1 = generic error

  wstring cur_option;
  wstring cur_option_arg;
  for (int i = 1; i < argc; ++i) {
    std::wstring param = argv[i];
    if (!param.empty() && param[0] == L'-') {
      // Param specifies an option.
      if (param.length() < 2) {
        // Invalid parameter.
        wprintf(L"WARNING: Skipping invalid option \"%ls\"\n", param.c_str());
        cur_option.clear();
        continue;
      }
      cur_option = param.substr(1, param.length() - 1);
      // Check for options without arguments.
      if (cur_option == L"h") {
        print_usage(app_name);
      }
      else if (cur_option == L"i") {
        name_prefix_in.clear(); // Clear default.
      }
      else if (cur_option == L"o") {
        name_prefix_out.clear(); // Clear default.
      }
      else if (cur_option == L"ddt") {
        // Hardwire my setup.
        name_prefix_in = L"loopMIDI";
        name_prefix_out = L"POLY2";
      }
    }
    else {
      // Param does not start with '-', so it's an option argument.
      if (cur_option.empty()) {
        wprintf(L"ERROR: No option specified for argument \"%ls\"\n", param.c_str());
        continue;
      }
      // Found option argument.
      cur_option_arg = param;
      // Apply option.
      if (cur_option == L"d") {
        size_t pos = 0;
        debug_level = std::stoi(cur_option_arg, &pos, 10);
        wprintf(L"Debug level = %d\n\n", debug_level);
      }
      if (cur_option == L"i") {
        name_prefix_in = cur_option_arg;
      }
      if (cur_option == L"o") {
        name_prefix_out = cur_option_arg;
      }
    }
  }
  LocalFree(argv);


  // Create SIDerurgy core listener.
  SIDerurgy* siderurgy = new SIDerurgy();
  siderurgy->debug_level_ = debug_level;

  // Create MIDI INPUT device.
  MIDI_Device* device_in = MIDI_Device::create(name_prefix_in, MIDI_Device::Mode::MIDI_IN, siderurgy);
  if (device_in == nullptr) {
    wprintf(L"ERROR: Failed to open MIDI IN device.\n");
    print_usage(app_name);
    return 3; // 3 = file / path not found
  }
  else {
    wprintf(L"Opened MIDI IN device \"%ls\"\n\n", device_in->get_device_name_in().c_str());
    if (device_in->get_error() != MMSYSERR_NOERROR) {
      wprintf(L"WARNING: Opened MIDI IN device error: %d\n", device_in->get_error());
    }
  }

  // Create MIDI OUTPUT device.
  MIDI_Device* device_out = MIDI_Device::create(name_prefix_out, MIDI_Device::Mode::MIDI_OUT);
  if (device_out == nullptr) {
    wprintf(L"ERROR: Failed to open MIDI OUT device.\n");
    return 3; // 3 = file / path not found
  }
  else {
    wprintf(L"Opened MIDI OUT device \"%ls\"\n", device_out->get_device_name_out().c_str());
    if (device_out->get_error() != MMSYSERR_NOERROR) {
      wprintf(L"WARNING: Opened MIDI OUT device error: %d\n", device_out->get_error());
    }
    siderurgy->midi_out_ = device_out;
  }


  // Run the message loop.

  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  delete device_in;

  return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      // All painting occurs here, between BeginPaint and EndPaint.
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
      EndPaint(hwnd, &ps);
      return 0;
    }

    case WM_QUIT: {
      //break;
    }
    
    //default: {
    //  break;
    //}

    //return 0;
  }
  
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}




