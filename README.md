# PS3 Grapher

A real-time graph plotting utility for **PlayStation 3** using **tiny3d** and the **PSL1GHT SDK**.

## Features

- **Graph rendering**: Plots lines and dots with a circular buffer for dynamic data display
- **Axis configuration**: Customizable X/Y ranges, offsets, and window size (default 800×500 at 1920×1080 resolution)
- **Axis labels**: Automatic numerical tick labels on both X and Y axes using TTF fonts
- **Dynamic updates**: Sliding mode for real-time data visualization with circular buffer overwrite
- **Color highlighting**: Current segment is highlighted in green

## Technical Details

| Aspect | Description |
|--------|-------------|
| Platform | PlayStation 3 (PPU/SPU) |
| SDK | PSL1GHT + tiny3d |
| Language | C |
| Rendering | Polygon quads & lines via tiny3d_VertexPos/Color |
| Fonts | TTF via `ttf_render.h` (`display_ttf_string`) |
| Dot buffer | Circular linked-list (`Dot` with `prev_dot` pointer) |

## Usage

```c
Grapher_Init(-10.0f, 10.0f, -1.0f, 1.0f, 200);  // start_x, end_x, min_y, max_y, dots_count
Grapher_set_labels("X", 2, "Y", 2, 5, 5);       // axis labels, tick count
Grapher_show_axes();

// In render loop:
Grapher_append_Dot(sin(curX));                  // add a point
Grapher_show(3.0f, RGBA(255, 255, 255, 255));   // render (dot_size, color)

// On cleanup:
Grapher_DeInit();
```

## Dependencies

- [PSL1GHT SDK](https://github.com/ps3homebrew/PSL1GHT)
- `tiny3d.h` (PS3 rendering library)
- `ttf_render.h` — TTF font rendering module

## Purpose

Designed for **homebrew applications** requiring data visualization: algorithm testing, signal display, or mathematical function demonstration on original PS3 hardware or the **rpcs3** emulator.

## API Reference

### Initialization

```c
void Grapher_Init(f32 start_x, f32 end_x, f32 min_y, f32 max_y, u32 dots_count);
```

### Label Setup

```c
void Grapher_set_labels(const char* axis_x_name, u8 x_name_len, 
                        const char* axis_y_name, u8 y_name_len, 
                        u32 count_x, u32 count_y);
```

### Rendering

```c
void Grapher_show_axes();        // Draw X and Y axes
void Grapher_append_Dot(f32 val_y);  // Add next data point
void Grapher_show(f32 dot_size, u32 color);  // Render full graph
void Grapher_clear();            // Reset all data
void Grapher_DeInit();           // Free all allocated memory
```

### Color Macro

```c
#define RGBA(r, g, b, alpha) (r << 24) | (g << 16) | (b << 8) | alpha
```

## License

This project is for PS3 homebrew development. Use freely for non-commercial purposes.
