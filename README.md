# OOP Paint Application

A simple object-oriented paint program for Windows, written in C++ using the Win32 API.  
This application demonstrates encapsulation, polymorphism, and modern C++ practices for GUI and drawing.

---

## Features

- **Draw Shapes:** Rectangle, Circle, and freehand line (Pen)
- **Text Tool:** Add text anywhere on the canvas with custom font and color
- **Eraser Tool:** Erase parts of your drawing with adjustable size
- **Color Picker:** Choose any drawing color
- **Pen/Eraser Size:** Set pen and eraser thickness
- **Canvas Resize:** Change canvas size (up to 4096x4096)
- **Save/Load:** Save your drawing as a BMP file or load an existing BMP
- **Status Bar:** Shows current tool and settings
- **Keyboard Shortcuts:** Quick tool switching

---

## Keyboard Shortcuts

| Key         | Action                |
|-------------|-----------------------|
| `R`         | Rectangle Tool        |
| `C`         | Circle Tool           |
| `L`         | Pen (Line) Tool       |
| `E`         | Toggle Eraser         |
| `T`         | Text Tool             |
| `F5`        | Reset Canvas (800x600)|
| `ESC`       | Cancel Drawing        |

---

## Usage

1. **Build the project** in Visual Studio 2022 (open `Paint.sln` and build).
2. **Run the application.**
3. **Select a tool** from the menu or use keyboard shortcuts.
4. **Draw** on the canvas by clicking and dragging.
5. **Add text** by selecting the text tool and clicking on the canvas.
6. **Change color, pen/eraser size, or font** from the menu.
7. **Save or load** your work using the File menu.

---

## File Structure

- `main.cpp` — Application entry point and main window logic
- `IShape.h` / `Shape.h` — Shape interface and base class
- `RectangleShape.*` / `CircleShape.*` — Rectangle and circle shape implementations
- `TextShape.*` — Text drawing implementation
- `FileManager.*` — Save/load BMP file logic
- `Utils.h` — Utility functions (e.g., Clamp)
- `Resource.h` / `Paint.rc` — Resource definitions (dialogs, icons, menus)

---

## Building

- **Requirements:** Visual Studio 2022, Windows SDK
- Open the solution (`Paint.sln`)
- Build the project (`Build > Build Solution`)
- Run (`Debug > Start Without Debugging`)

---

## Notes

- The app uses Win32 API and GDI for drawing.
- All shapes are managed with C++ polymorphism (`std::unique_ptr<IShape>`).
- The code is organized for clarity and extensibility.

---

## License

This project is provided for educational purposes.  
Feel free to use and modify for your own learning or projects.

