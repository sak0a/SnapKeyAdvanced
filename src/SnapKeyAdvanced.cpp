#include <windows.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <random>
#include <thread>
#include <chrono>
#include "../include/nlohmann/json.hpp"
#include "../include/resource.h"

using json = nlohmann::json;
using namespace std;

// Constants and Paths
constexpr auto CONFIG_FILE = "./config.json";
constexpr auto CONFIG_BACKUP = "./meta/backup.json";

// Window Message IDs
constexpr auto ID_TRAY_APP_ICON = 1001;
constexpr auto WM_TRAYICON = (WM_USER + 1);

// Menu Item IDs
enum MenuItems {
    ID_TRAY_EXIT = 3000,
    ID_TRAY_VERSION_INFO,
    ID_TRAY_DELAY_INFO,
    ID_TRAY_REBIND_KEYS,
    ID_TRAY_LOCK_FUNCTION,
    ID_TRAY_RESTART
};

// Application State
struct AppState {
    int minDelay = 5;
    int maxDelay = 12;
    bool isLocked = false;
    HHOOK hHook = nullptr;
    HANDLE hMutex = nullptr;
    NOTIFYICONDATA nid = {};

    struct KeyState {
        bool registered = false;
        bool keyDown = false;
        int group{};
        bool simulated = false;
    };

    struct GroupState {
        int previousKey = 0;
        int activeKey = 0;
    };

    unordered_map<int, GroupState> groupInfo;
    unordered_map<int, KeyState> keyInfo;
};

// Global state (could be refactored into a singleton class)
AppState appState;

// Function Declarations
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ConfigManager {
public:
    static json getDefaultConfig() {
        return {
            {"settings", {
                {"min_delay_ms", 5},
                {"max_delay_ms", 12},
                {"version", "2.0.0"}
            }},
            {"groups", {{
                {"id", 1},
                {"name", "Movement Keys 1"},
                {"keys", {65, 68}}  // A, D
            }, {
                {"id", 2},
                {"name", "Movement Keys 2"},
                {"keys", {83, 87}}  // S, W
            }}},
            {"keymap", {
                {"65", "A"},
                {"68", "D"},
                {"83", "S"},
                {"87", "W"}
            }}
        };
    }

    static void createDefaultConfig() {
        try {
            const json defaultConfig = getDefaultConfig();
            ofstream configFile(CONFIG_FILE);
            configFile << defaultConfig.dump(2);

            MessageBox(nullptr, TEXT("Default config.json has been created."),
                      TEXT("SnapKey"), MB_ICONINFORMATION);
        }
        catch (const exception& e) {
            string error = "Failed to create default config: " + string(e.what());
            MessageBox(nullptr, error.c_str(), TEXT("SnapKey Error"), MB_ICONERROR);
        }
    }

    static bool loadConfig() {
        try {
            // Check if config file exists
            ifstream configFile(CONFIG_FILE);
            if (!configFile.is_open()) {
                createDefaultConfig();
                configFile.open(CONFIG_FILE);
                if (!configFile.is_open()) {
                    MessageBox(nullptr, TEXT("Could not create or open config.json"),
                             TEXT("SnapKey Error"), MB_ICONERROR);
                    return false;
                }
            }

            json config = json::parse(configFile);

            // Load delay settings
            if (config.contains("settings")) {
                auto& settings = config["settings"];
                appState.minDelay = settings["min_delay_ms"];
                appState.maxDelay = settings["max_delay_ms"];
            }

            // Load key groups
            if (config.contains("groups")) {
                for (const auto& group : config["groups"]) {
                    int groupId = group["id"];
                    for (const auto& key : group["keys"]) {
                        int keyCode = key.get<int>();
                        if (!appState.keyInfo[keyCode].registered) {
                            appState.keyInfo[keyCode].registered = true;
                            appState.keyInfo[keyCode].group = groupId;
                        } else {
                            MessageBox(nullptr, TEXT("Duplicate key detected in config!"),
                                     TEXT("SnapKey Error"), MB_ICONERROR);
                            return false;
                        }
                    }
                }
            }
            return true;
        }
        catch (const json::exception& e) {
            string error = "JSON parsing error: " + string(e.what());
            MessageBox(nullptr, error.c_str(), TEXT("SnapKey Error"), MB_ICONERROR);
            return false;
        }
    }

    static void restoreDefaultConfig() {
        createDefaultConfig();
    }
};

class KeyHandler {
public:
    static void handleKeyDown(int keyCode) {
        auto& currentKeyInfo = appState.keyInfo[keyCode];
        auto& currentGroupInfo = appState.groupInfo[currentKeyInfo.group];

        if (!currentKeyInfo.keyDown) {
            currentKeyInfo.keyDown = true;
            sendKey(keyCode, true);

            if (currentGroupInfo.activeKey == 0 || currentGroupInfo.activeKey == keyCode) {
                currentGroupInfo.activeKey = keyCode;
            } else {
                currentGroupInfo.previousKey = currentGroupInfo.activeKey;
                currentGroupInfo.activeKey = keyCode;
                addRandomDelay();
                sendKey(currentGroupInfo.previousKey, false);
            }
        }
    }

    static void handleKeyUp(int keyCode) {
        auto& currentKeyInfo = appState.keyInfo[keyCode];
        auto& currentGroupInfo = appState.groupInfo[currentKeyInfo.group];

        if (currentGroupInfo.previousKey == keyCode && !currentKeyInfo.keyDown) {
            currentGroupInfo.previousKey = 0;
        }

        if (currentKeyInfo.keyDown) {
            currentKeyInfo.keyDown = false;
            if (currentGroupInfo.activeKey == keyCode && currentGroupInfo.previousKey != 0) {
                sendKey(keyCode, false);
                currentGroupInfo.activeKey = currentGroupInfo.previousKey;
                currentGroupInfo.previousKey = 0;
                sendKey(currentGroupInfo.activeKey, true);
            } else {
                currentGroupInfo.previousKey = 0;
                if (currentGroupInfo.activeKey == keyCode) currentGroupInfo.activeKey = 0;
                sendKey(keyCode, false);
            }
        }
    }

private:
    static void sendKey(const int targetKey, const bool keyDown) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = targetKey;
        input.ki.wScan = MapVirtualKey(targetKey, 0);

        DWORD flags = KEYEVENTF_SCANCODE;
        input.ki.dwFlags = keyDown ? flags : flags | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    static void addRandomDelay() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(appState.minDelay, appState.maxDelay);
        this_thread::sleep_for(chrono::milliseconds(dis(gen)));
    }
};

class SystemTray {
public:

    static void initialize(HWND hwnd) {
        memset(&appState.nid, 0, sizeof(NOTIFYICONDATA));

        appState.nid.cbSize = sizeof(NOTIFYICONDATA);
        appState.nid.hWnd = hwnd;
        appState.nid.uID = ID_TRAY_APP_ICON;
        appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        appState.nid.uCallbackMessage = WM_TRAYICON;

        // Load icon from resources
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_ON));
        if (hIcon)
            appState.nid.hIcon = hIcon;
        else
            appState.nid.hIcon = LoadIcon(NULL, IDI_ERROR);

        lstrcpy(appState.nid.szTip, TEXT("SnapKeyAdvanced"));

        Shell_NotifyIcon(NIM_ADD, &appState.nid);
    }

    static void updateIcon(const bool isLocked) {
        // Update the tray icon
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HICON hIconLocal = LoadIcon(hInstance, MAKEINTRESOURCE(isLocked ? IDI_ICON_OFF : IDI_ICON_ON));
        if (hIconLocal) {
            appState.nid.hIcon = hIconLocal;
            Shell_NotifyIcon(NIM_MODIFY, &appState.nid);
            DestroyIcon(hIconLocal);
        }
    }
};

int main() {
    // Load configuration
    if (!ConfigManager::loadConfig()) return 1;

    // Ensure single instance
    appState.hMutex = CreateMutex(NULL, TRUE, TEXT("SnapKeyMutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, TEXT("SnapKey is already running!"),
                  TEXT("SnapKey"), MB_ICONINFORMATION);
        return 1;
    }


    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("SnapKeyClass");

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Window Registration Failed!"),
                  TEXT("Error"), MB_ICONEXCLAMATION);
        return 1;
    }

    // Create window
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, TEXT("SnapKey"),
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              240, 120, NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, TEXT("Window Creation Failed!"),
                  TEXT("Error"), MB_ICONEXCLAMATION);
        return 1;
    }

    // Initialize system tray
    SystemTray::initialize(hwnd);

    // Set keyboard hook
    appState.hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!appState.hHook) {
        MessageBox(NULL, TEXT("Failed to install hook!"),
                  TEXT("Error"), MB_ICONEXCLAMATION);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    UnhookWindowsHookEx(appState.hHook);
    Shell_NotifyIcon(NIM_DELETE, &appState.nid);
    ReleaseMutex(appState.hMutex);
    CloseHandle(appState.hMutex);

    return 0;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (!appState.isLocked && nCode >= 0) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;

        if (!(pKeyBoard->flags & 0x10) &&
            appState.keyInfo[pKeyBoard->vkCode].registered) {

            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
                KeyHandler::handleKeyDown(pKeyBoard->vkCode);
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
                KeyHandler::handleKeyUp(pKeyBoard->vkCode);
            return 1;
        }
    }
    return CallNextHookEx(appState.hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONDOWN) {
                POINT curPoint;
                GetCursorPos(&curPoint);
                SetForegroundWindow(hwnd);

                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, ID_TRAY_REBIND_KEYS, TEXT("Rebind Keys"));
                AppendMenu(hMenu, MF_STRING | (appState.isLocked ? MF_CHECKED : MF_UNCHECKED),
                          ID_TRAY_LOCK_FUNCTION, TEXT("Disable SnapKey"));
                AppendMenu(hMenu, MF_STRING, ID_TRAY_RESTART, TEXT("Restart SnapKey"));
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, ID_TRAY_VERSION_INFO, TEXT("Version Info"));
                AppendMenu(hMenu, MF_STRING, ID_TRAY_DELAY_INFO, TEXT("Key Delay Info"));
                AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));

                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                             curPoint.x, curPoint.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            else if (lParam == WM_LBUTTONDBLCLK) {
                appState.isLocked = !appState.isLocked;
                SystemTray::updateIcon(appState.isLocked);
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    break;

                case ID_TRAY_VERSION_INFO:
                    MessageBox(hwnd, "SnapKey 2.0.0", TEXT("Version Info"), MB_OK);
                    break;

                case ID_TRAY_DELAY_INFO: {
                    string info = "Delay between Keys: " +
                                to_string(appState.minDelay) + "ms - " +
                                to_string(appState.maxDelay) + "ms";
                    MessageBox(hwnd, info.c_str(), TEXT("Key Delay Info"), MB_OK);
                    break;
                }

                case ID_TRAY_REBIND_KEYS:
                    ShellExecute(NULL, TEXT("open"), CONFIG_FILE, NULL, NULL, SW_SHOWNORMAL);
                    break;

                case ID_TRAY_RESTART: {
                    TCHAR szExeFileName[MAX_PATH];
                    GetModuleFileName(NULL, szExeFileName, MAX_PATH);
                    ShellExecute(NULL, NULL, szExeFileName, NULL, NULL, SW_SHOWNORMAL);
                    PostQuitMessage(0);
                    break;
                }

                case ID_TRAY_LOCK_FUNCTION:
                    appState.isLocked = !appState.isLocked;
                    SystemTray::updateIcon(appState.isLocked);
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
