#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "render_model.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define FOV 70.0

Vector3 rotateZ(Vector3 v, float t){
  return (Vector3){
    .x = v.x*cos(t) + -v.y*sin(t),
    .y = v.x*sin(t) + v.y*cos(t),
    .z = v.z,
  }; 
}

Vector3 rotateY(Vector3 v, float t){
  return (Vector3){
    .x = v.x*cos(t) + -v.z*sin(t),
    .y = v.y,
    .z = v.x*sin(t) + v.z*cos(t),
  }; 
}

Vector3 rotateX(Vector3 v, float t){
  return (Vector3){
    .x = v.x,
    .y = v.y*cos(t) + -v.z*sin(t),
    .z = v.y*sin(t) + v.z*cos(t),
  }; 
}

typedef struct {
  Vector3 pos;
  Vector3 ang;
} Cam;

typedef struct {
  RenderModel rm;
  Vector3 pos;
  Vector3 sc;
  Vector3 ang;
} RenderObject;

Vector3 calc(Vector3 v, RenderObject *r, Cam c){
  v.x *= r->sc.x;
  v.y *= r->sc.y;
  v.z *= r->sc.z;
  v = rotateY(v, r->ang.y);
  v = rotateX(v, r->ang.x);
  v = rotateZ(v, r->ang.z);
  v = Vector3Subtract(v, c.pos);
  v = Vector3Add(v, r->pos);
  v = rotateY(v, -c.ang.y);
  v = rotateX(v, -c.ang.x);
  v = rotateZ(v, -c.ang.z);
  return v;
}

void draw_triangle(Vector3 v1, Vector3 v2, Vector3 v3) {
    if (v1.z < 0 || v2.z < 0 || v3.z < 0) return;

    float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    float fovScale = tanf(FOV * 0.5f * (PI / 180.0f));

    Vector2 p1 = { (v1.x / (v1.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
                   -(v1.y / (v1.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
    Vector2 p2 = { (v2.x / (v2.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
                   -(v2.y / (v2.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
    Vector2 p3 = { (v3.x / (v3.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
                   -(v3.y / (v3.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };

    DrawLineV(p1, p2, WHITE);
    DrawLineV(p1, p3, WHITE);
    DrawLineV(p3, p2, WHITE);
    // DrawTriangle(p1, p2, p3, WHITE);
}

void draw_render_object(RenderObject *r, Cam c){
  for(int fi = 0; fi < r->rm.fn; fi++){
    int p1 = r->rm.fs[fi].vi[0] - 1;
    Vector3 v1 = calc(r->rm.vs[p1], r, c);
    for(int vi = 1; vi < r->rm.fs[fi].cnt-1; vi++){
      int p2 = r->rm.fs[fi].vi[vi] - 1;
      int p3 = r->rm.fs[fi].vi[vi+1] - 1;
      
      Vector3 v2 = calc(r->rm.vs[p2], r, c);
      Vector3 v3 = calc(r->rm.vs[p3], r, c);

      draw_triangle(v1, v2, v3);
    }
  }
}

void movement(Cam *c){
  float mvSpeed = 4, rtSpeed = 0.23;

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
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 3D engine");
  SetTargetFPS(144);
  DisableCursor();
    
  Cam cam = {
    .pos = (Vector3){0, 0, 0},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject robj = {
    .rm = parse_obj("models/car.obj"),
    .pos = (Vector3){0, -2.1, 4},
    .sc = (Vector3){1, 1, 1},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject robj2 = {
    .rm = parse_obj("models/apple.obj"),
    .pos = (Vector3){0, 0.3, -4},
    .sc = (Vector3){0.2, 0.2, 0.2},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject robj3 = {
    .rm = parse_obj("models/cube.obj"),
    .pos = (Vector3){0, -1.1, -4},
    .sc = (Vector3){0.4, 1, 0.4},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject robj4 = {
    .rm = parse_obj("models/plane.obj"),
    .pos = (Vector3){0, -2.1, 0},
    .sc = (Vector3){8, 1, 8},
    .ang = (Vector3){0, 0, 0},
  };

  while(!WindowShouldClose()){
    movement(&cam);

    //robj2.sc.x = 1+sin(GetTime());
    //robj2.sc.y = 1+sin(GetTime()+PI*2*0.33);
    //robj2.sc.z = 1+sin(GetTime()+PI*2*0.66);
    robj2.ang.y += GetFrameTime() * PI * 2 * 0.14;
    robj2.ang.z += GetFrameTime() * PI * 2 * 0.14;
    robj2.ang.x += GetFrameTime() * -PI * 2 * 0.14;

    robj.ang.y += GetFrameTime();
    robj.pos.z += 6 * GetFrameTime() * sin(robj.ang.y+PI/2.0);
    robj.pos.x += 6 * GetFrameTime() * cos(robj.ang.y+PI/2.0);
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    draw_render_object(&robj, cam);
    draw_render_object(&robj2, cam);
    draw_render_object(&robj3, cam);
    draw_render_object(&robj4, cam);
    
    EndDrawing();
  }
  
  CloseWindow();
  return 0;
}

