# SnapKey Configuration Guide

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
The config.json file contains:
- settings: General application settings including delays
- groups: Key groupings for different movement combinations
- keymap: Human-readable descriptions of key codes

After modifying the configuration, restart SnapKey to apply changes.

## Example Config
```json
{
  "groups": [
    {
      "id": 1,
      "keys": [
        65,
        68
      ],
      "name": "Movement Keys 1"
    },
    {
      "id": 2,
      "keys": [
        83,
        87
      ],
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
    "max_delay_ms": 12,
    "min_delay_ms": 5,
    "version": "2.0.0"
  }
}