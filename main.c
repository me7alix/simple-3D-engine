#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "src/renderer.h"

void movement(RenderCamera *c){
  float mvSpeed = 4, rtSpeed = 0.3;

  c->ang.y += GetMouseDelta().x * -rtSpeed * GetFrameTime();
  c->ang.x += GetMouseDelta().y * rtSpeed * GetFrameTime();

  if(IsKeyDown(KEY_W)){
    c->pos.z += mvSpeed * GetFrameTime() * sin(c->ang.y+PI/2.0);
    c->pos.x += mvSpeed * GetFrameTime() * cos(c->ang.y+PI/2.0);
  }
  if(IsKeyDown(KEY_S)){
    c->pos.z += -mvSpeed * GetFrameTime() * sin(c->ang.y+PI/2.0);
    c->pos.x += -mvSpeed * GetFrameTime() * cos(c->ang.y+PI/2.0);
  }
  if(IsKeyDown(KEY_A)){
    c->pos.z += -mvSpeed * GetFrameTime() * sin(c->ang.y);
    c->pos.x += -mvSpeed * GetFrameTime() * cos(c->ang.y);
  }
  if(IsKeyDown(KEY_D)){
    c->pos.z += mvSpeed * GetFrameTime() * sin(c->ang.y);
    c->pos.x += mvSpeed * GetFrameTime() * cos(c->ang.y);
  }
  if(IsKeyDown(KEY_SPACE)){
    c->pos.y += mvSpeed * GetFrameTime();
  }
  if(IsKeyDown(KEY_LEFT_SHIFT)){
    c->pos.y += -mvSpeed * GetFrameTime();
  }
}

int main(void){
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 3D renderer");
  SetTargetFPS(144);
  DisableCursor();

  trs_init();
  set_sun_dir((Vector3){3, 4, 1});
    
  get_current_camera()[0] = (RenderCamera){
    .pos = (Vector3){0, 0, 0},
    .ang = (Vector3){0, 0, 0},
    .fov = 70.0,
  };

  RenderObject car = {
    .rm = parse_obj("models/toyota.obj"),
    .pos = (Vector3){0, -2, 4},
    .sc = (Vector3){0.004, 0.004, 0.004},
    .ang = (Vector3){0, 0, 0},
    .tx = LoadImage("textures/toyota.png"),
    .isTxUsing = 1,
  };

  RenderObject apple = {
    .rm = parse_obj("models/apple.obj"),
    .pos = (Vector3){0, 0.3, -4},
    .sc = (Vector3){0.2, 0.2, 0.2},
    .ang = (Vector3){0, 0, 0},
    .isTxUsing = 0,
  };

  RenderObject cube = {
    .rm = parse_obj("models/cube.obj"),
    .pos = (Vector3){0, -1.1, -4},
    .sc = (Vector3){0.4, 1, 0.4},
    .ang = (Vector3){0, 0, 0},
    .tx = LoadImage("textures/plane.png"),
    .isTxUsing = 1,
  };

  RenderObject plane = {
    .rm = parse_obj("models/plane.obj"),
    .pos = (Vector3){-5, -2.1, 5},
    .sc = (Vector3){3.1, 1, 3.1},
    .ang = (Vector3){0, 0, 0},
    .tx = LoadImage("textures/plane.png"),
    .isTxUsing = 1,
  };

  while(!WindowShouldClose()){
    trs_reset();
    movement(get_current_camera());

    apple.ang.y += GetFrameTime() * PI * 2 * 0.14;
    apple.ang.z += GetFrameTime() * PI * 2 * 0.14;
    apple.ang.x += GetFrameTime() * -PI * 2 * 0.14;

    car.ang.y += GetFrameTime() * 0.7;
    car.pos.z += 5 * GetFrameTime() * sin(car.ang.y+PI/2.0);
    car.pos.x += 5 * GetFrameTime() * cos(car.ang.y+PI/2.0);
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    draw_render_object(&car);
    draw_render_object(&apple);
    draw_render_object(&cube);
    draw_render_object(&plane);

    draw_triangles();

    EndDrawing();
  }
  
  CloseWindow();
  return 0;
}
