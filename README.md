# Microwave Oven Controller using PIC16F877A

A menu-driven microwave oven controller developed using the **PIC16F877A microcontroller**, a 16×4 character LCD, and a matrix keypad. The project simulates microwave cooking operations, including Micro, Grill, Convection, and Quick Start modes.

## Features

- **Startup Animation:** Displays a power-on animation before showing the main menu.
- **Micro Mode:** Set cooking time in MM:SS format and start, pause, resume, or stop the countdown.
- **Grill Mode:** Provides the same time-setting and countdown controls as Micro mode.
- **Convection Mode:** Set a temperature between 25°C and 180°C, simulate a 60-second preheating cycle, and then enter the cooking time.
- **Quick Start Mode:** Starts a 30-second countdown immediately. Pressing the Quick Start key while running adds another 30 seconds.
- **LCD Interface:** Displays menus, temperature, cooking time, preheating status, and completion messages.
- **Timer-Based Operation:** Uses Timer2 to generate timing events for countdown and display-related functions.
- **Keypad Controls:** Supports numeric input, clearing, confirmation, start, pause, resume, and stop operations.

## Hardware and Software Requirements

### Hardware

- PIC16F877A microcontroller
- 16×4 character LCD
- 4×3 matrix keypad
- 20 MHz oscillator
- PIC development/simulation board

### Software and Tools

- MPLAB X IDE
- XC8 C Compiler
- PICSimLab (for simulation)

## Operating Modes

| Mode | Description |
|---|---|
| Micro | Enter cooking time and control the countdown |
| Grill | Enter cooking time and control the countdown |
| Convection | Enter temperature, complete simulated preheating, then set cooking time |
| Quick Start | Start at 30 seconds and add 30 seconds by pressing the Quick Start key while running |

## Keypad Controls

| Key | Function |
|---|---|
| `0–9` | Enter temperature or cooking-time digits |
| `*` | Clear the current input |
| `#` | Confirm temperature or cooking time |
| `4` | Start/resume; in Quick Start, add 30 seconds while running |
| `5` | Pause the countdown |
| `6` | Stop and return to the main menu |

*Key behavior depends on the active mode and screen.*

## Project Structure

The project uses separate driver files for peripheral control and a main application file for the operating logic.

```text
Microwave-Oven/
├── main.c
├── clcd.c
├── clcd.h
├── matrix_keypad.c
├── matrix_keypad.h
├── timer.c
├── timer.h
└── README.md
```

*The exact folder structure may vary depending on your MPLAB X project setup.*

## How It Works

1. The microcontroller initializes the LCD, matrix keypad, and Timer2.
2. A startup animation is displayed.
3. The user selects an operating mode from the main menu.
4. Depending on the selected mode, the controller accepts cooking time or temperature input.
5. Timer events update the preheating or cooking countdown.
6. When the countdown reaches zero, a completion message is displayed and the controller returns to the main menu.

## Implementation Details

- **Programming language:** Embedded C
- **Microcontroller:** PIC16F877A
- **LCD interface:** 16×4 character LCD
- **Input interface:** Matrix keypad
- **Timekeeping:** Timer2 interrupt-based timing events
- **Input validation:** Validates the seconds field and Convection temperature range
- **State management:** Uses mode and state variables to control the cooking workflow

## How to Run

1. Clone or download this repository.
2. Open the project in MPLAB X IDE.
3. Ensure the required source files and header files are included in the project.
4. Select the PIC16F877A device and configure the XC8 compiler.
5. Build the project and generate the HEX file.
6. Load the HEX file into PICSimLab or program a compatible PIC16F877A development board.
7. Run the simulation and test each operating mode.

## Learning Outcomes

Through this project, I gained practical experience with:

- PIC16F877A peripheral interfacing
- LCD and matrix keypad drivers
- Timer2-based timing and countdown implementation
- Interrupt-driven programming concepts
- State-machine-based application design
- Input validation and keypad event handling
- Embedded C programming and debugging
- Simulation and testing using PICSimLab

## Future Improvements

- Implement actual temperature sensing and closed-loop temperature control.
- Integrate heating-element control through appropriate driver circuitry.
- Add door-open detection and safety interlocks.
- Improve timer accuracy and input handling.
- Add buzzer alerts for cooking completion.

## Disclaimer

This is an educational microcontroller project that simulates microwave oven functionality. The temperature and preheating behavior are simulated; the project does not implement actual microwave heating or certified appliance safety mechanisms.

## Author

Developed as an embedded systems project to explore microcontroller programming, peripheral interfacing, and timer-based control.
