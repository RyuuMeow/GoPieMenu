#include <windows.h>

// A separate process verifies real keyboard delivery, independent of Qt focus.
LRESULT CALLBACK targetProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    static int keys = 0;
    if (message == WM_KEYDOWN && wparam == VK_F24 && (GetKeyState(VK_CONTROL) & 0x8000)) ++keys;
    if (message == WM_APP + 1) return keys;
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(window, message, wparam, lparam);
}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    WNDCLASSW klass{}; klass.hInstance = instance; klass.lpfnWndProc = targetProc;
    klass.lpszClassName = L"GoPieMenuInputTestTarget";
    klass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&klass)) return 1;
    const auto window = CreateWindowExW(0, klass.lpszClassName, L"GoPieMenu input test target",
        WS_OVERLAPPEDWINDOW, 50, 50, 600, 400, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, SW_SHOWNORMAL);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    return 0;
}
