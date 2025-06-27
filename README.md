# SnapKey Advanced v3.0.0

SnapKey Advanced is a Windows application that provides intelligent key management for gaming and productivity. It features two distinct modes of operation: **Standard Mode** for advanced key simulation and **Keyboard Mode** for simple delay injection.

## Features

- **Dual Operation Modes**: Standard mode with key simulation or Keyboard mode with delay injection
- **System Tray Integration**: Complete control via system tray with visual status indicators
- **Configurable Key Groups**: Organize keys into logical groups for different movement combinations
- **Nanosecond Precision Delays**: Highly precise timing control with randomization
- **Real-time Configuration**: Toggle modes and settings without restarting
- **Automatic Configuration Management**: Backup and restore functionality
- **Multi-instance Protection**: Prevents multiple instances from running simultaneously

## Operation Modes

### Standard Mode (Default)
- Intercepts configured keys and provides intelligent key simulation
- Prevents multiple keys in the same group from being active simultaneously
- Adds randomized delays between key transitions
- Ideal for applications requiring precise key control

### Keyboard Mode
- Allows normal key presses to reach applications
- Adds configurable delays only when keys are pressed
- Useful for applications that need direct keyboard input with timing control
- Toggle via system tray menu or configuration file

## System Tray Controls

Right-click the system tray icon to access:
- **Rebind Keys**: Opens configuration file for editing
- **Disable SnapKey**: Temporarily disable all functionality (visual indicator changes)
- **Use Keyboard Mode**: Toggle between Standard and Keyboard modes
- **Restart SnapKey**: Restart the application
- **Version Info**: Display current version
- **Key Delay Info**: Show current delay settings
- **Exit**: Close the application

**Quick Toggle**: Double-click the tray icon to quickly enable/disable SnapKey

## Key Codes Reference

### Letter Keys
- A: 65
- B: 66
- C: 67
- D: 68
- E: 69
- F: 70
- G: 71
- H: 72
- I: 73
- J: 74
- K: 75
- L: 76
- M: 77
- N: 78
- O: 79
- P: 80
- Q: 81
- R: 82
- S: 83
- T: 84
- U: 85
- V: 86
- W: 87
- X: 88
- Y: 89
- Z: 90

### Arrow Keys
- Up: 38
- Down: 40
- Left: 37
- Right: 39

### Special Keys
- BACKSPACE: 8
- L SHIFT: 160
- R SHIFT: 161
- L CONTROL: 162
- R CONTROL: 163
- ALT: 164
- ESC: 27
- SPACE: 32
- DEL: 46

### Numpad Keys
- NUM0: 96
- NUM1: 97
- NUM2: 98
- NUM3: 99
- NUM4: 100
- NUM5: 101
- NUM6: 102
- NUM7: 103
- NUM8: 104
- NUM9: 105

## Keyboard Layouts
### QWERTY
Default configuration: A/D + S/W (Keys: 65,68 / 83,87)

### AZERTY
Recommended configuration: Q/D + Z/S (Keys: 81,68 / 90,83)

### QWERTZ
Same as QWERTY: A/D + S/W (Keys: 65,68 / 83,87)

## Configuration File Structure

The `config.json` file contains three main sections:

### Settings Section
- **min_delay_ns**: Minimum delay in nanoseconds (Standard Mode)
- **max_delay_ns**: Maximum delay in nanoseconds (Standard Mode)
- **use_keyboard**: Boolean to enable Keyboard Mode
- **version**: Application version

### Groups Section
- **id**: Unique identifier for the key group
- **name**: Human-readable name for the group
- **keys**: Array of key codes that belong to this group

### Keymap Section
- Maps key codes to human-readable names for reference

**Note**: Most settings can be changed via the system tray menu without editing the configuration file directly. The application will automatically update the config file when settings are changed through the interface.

## Example Configuration
```json
{
  "groups": [
    {
      "id": 1,
      "keys": [65, 68],
      "name": "Movement Keys 1"
    },
    {
      "id": 2,
      "keys": [83, 87],
      "name": "Movement Keys 2"
    }
  ],
  "keymap": {
    "65": "A",
    "68": "D",
    "83": "S",
    "87": "W"
  },
  "settings": {
    "min_delay_ns": 500000,
    "max_delay_ns": 500000,
    "use_keyboard": true,
    "version": "3.0.0"
  }
}
```

## Installation and Usage

1. **Download**: Get the latest `SnapKeyAdvanced.exe` from the releases
2. **Run**: Execute the application (it will create a default config.json if none exists)
3. **Configure**: Right-click the system tray icon to access settings
4. **Customize**: Edit key bindings by opening the configuration file via the tray menu

## Delay Settings

SnapKey uses nanosecond precision for timing:
- **Standard Values**: 500,000 ns (0.5 ms) for general use
- **Range**: Can be set from 0 to several million nanoseconds
- **Randomization**: When min and max differ, delays are randomized within the range
- **Mode-Specific**: Different delay settings can be configured for each mode

## Troubleshooting

- **Multiple Instances**: SnapKey prevents multiple instances automatically
- **Configuration Issues**: Delete config.json to restore defaults
- **Tray Icon Missing**: Check Windows notification area settings
- **Key Not Working**: Verify key codes in the configuration match your keyboard layout