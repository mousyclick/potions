#include <raylib.h>

int main()
{
  InitWindow(800, 600, "Hello Raylib");
  while(!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RED);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
