#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "render_model.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define FOV 70.0
#define TM 10000

#define MAX(x, y) ((x) > (y) ? (x) : (y))

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

typedef struct {
  Vector2 p1, p2, p3;
  Vector3 n;
  float distToCam;
} Triangle;

Cam currentCam;

Vector3 sunDir = {3, 4, 1};

Triangle **trs;
int trs_cnt;

Vector3 CalculateTriangleNormal(Vector3 v1, Vector3 v2, Vector3 v3) {
  Vector3 edge1 = Vector3Subtract(v2, v1);
  Vector3 edge2 = Vector3Subtract(v3, v1);
  Vector3 normal = Vector3CrossProduct(edge1, edge2);
  normal = Vector3Normalize(normal);
  return normal;
}

Vector3 FindTriangleCentroid(Vector3 v1, Vector3 v2, Vector3 v3) {
  Vector3 centroid;
  centroid.x = (v1.x + v2.x + v3.x) / 3.0f;
  centroid.y = (v1.y + v2.y + v3.y) / 3.0f;
  centroid.z = (v1.z + v2.z + v3.z) / 3.0f;
  return centroid;
}

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

Vector3 calc(Vector3 v, RenderObject *r, Vector3 *nv){
  v.x *= r->sc.x;
  v.y *= r->sc.y;
  v.z *= r->sc.z;
  v = rotateY(v, r->ang.y);
  v = rotateX(v, r->ang.x);
  v = rotateZ(v, r->ang.z);
  v = Vector3Subtract(v, currentCam.pos);
  v = Vector3Add(v, r->pos);
  *nv = v;
  v = rotateY(v, -currentCam.ang.y);
  v = rotateX(v, -currentCam.ang.x);
  v = rotateZ(v, -currentCam.ang.z);
  return v;
}

void calc_triangle(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 n) {
  if (v1.z < 0 || v2.z < 0 || v3.z < 0) return;

  float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
  float fovScale = tanf(FOV * 0.5f * (PI / 180.0f));

  Vector2 p1 = { (v1.x / (v1.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v1.y / (v1.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
  Vector2 p2 = { (v2.x / (v2.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v2.y / (v2.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
  Vector2 p3 = { (v3.x / (v3.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v3.y / (v3.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };

  Vector3 center = FindTriangleCentroid(v1, v2, v3);
  trs[trs_cnt++][0] = (Triangle){
    .p1 = p1, .p2 = p2, .p3 = p3, .n = n,
    .distToCam = Vector3Distance((Vector3){0, 0, 0}, center),
  };
}

void draw_render_object(RenderObject *r){
  for(int fi = 0; fi < r->rm.fn; fi++){
    Vector3 nv1, nv2, nv3;
    int p1 = r->rm.fs[fi].vi[0] - 1;
    Vector3 v1 = calc(r->rm.vs[p1], r, &nv1);
    for(int vi = 1; vi < r->rm.fs[fi].cnt-1; vi++){
      int p2 = r->rm.fs[fi].vi[vi] - 1;
      int p3 = r->rm.fs[fi].vi[vi+1] - 1;
      
      Vector3 v2 = calc(r->rm.vs[p2], r, &nv2);
      Vector3 v3 = calc(r->rm.vs[p3], r, &nv3);
      
      Vector3 triangleNormal = CalculateTriangleNormal(nv1, nv2, nv3);

      calc_triangle(v1, v2, v3, triangleNormal);
    }
  }
}

void movement(Cam *c){
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

void SwapTriangles(Triangle **a, Triangle **b) {
  Triangle *temp = *a;
  *a = *b;
  *b = temp;
}

int Partition(Triangle **arr, int low, int high) {
  float pivot = arr[high]->distToCam;
  int i = low - 1;

  for (int j = low; j <= high - 1; j++) {
    if (arr[j]->distToCam > pivot) {
      i++;
      SwapTriangles(&arr[i], &arr[j]);
    }
  }
  SwapTriangles(&arr[i + 1], &arr[high]);
  return (i + 1);
}

void QuickSortTriangles(Triangle **arr, int low, int high) {
  if (low < high) {
    int pi = Partition(arr, low, high);
    QuickSortTriangles(arr, low, pi - 1);
    QuickSortTriangles(arr, pi + 1, high);
  }
}

void draw_triangles(){
  QuickSortTriangles(trs, 0, trs_cnt-1);

  for(int i = 0; i < trs_cnt; i++){
    float dotProduct = Vector3DotProduct(trs[i]->n, Vector3Normalize(sunDir));
    dotProduct = MAX(dotProduct, 0);
    Color col = {(int)(dotProduct*255.0), (int)(dotProduct*255.0), (int)(dotProduct*255.0), 255};
    DrawTriangle(trs[i]->p2, trs[i]->p1, trs[i]->p3, col);
  }
}


int main(void){
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 3D engine");
  SetTargetFPS(144);
  DisableCursor();

  trs = malloc(sizeof(Triangle*)*TM);
  for(int i = 0; i < TM; i++){
    trs[i] = malloc(sizeof(Triangle));
  }
    
  currentCam = (Cam){
    .pos = (Vector3){0, 0, 0},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject car = {
    .rm = parse_obj("models/car.obj"),
    .pos = (Vector3){0, -2.1, 4},
    .sc = (Vector3){1, 1, 1},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject apple = {
    .rm = parse_obj("models/apple.obj"),
    .pos = (Vector3){0, 0.3, -4},
    .sc = (Vector3){0.2, 0.2, 0.2},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject cube = {
    .rm = parse_obj("models/cube.obj"),
    .pos = (Vector3){0, -1.1, -4},
    .sc = (Vector3){0.4, 1, 0.4},
    .ang = (Vector3){0, 0, 0},
  };

  RenderObject plane = {
    .rm = parse_obj("models/plane.obj"),
    .pos = (Vector3){0, -2.1, 0},
    .sc = (Vector3){3.1, 1, 3.1},
    .ang = (Vector3){0, 0, 0},
  };

  while(!WindowShouldClose()){
    trs_cnt = 0;
    movement(&currentCam);

    apple.ang.y += GetFrameTime() * PI * 2 * 0.14;
    apple.ang.z += GetFrameTime() * PI * 2 * 0.14;
    apple.ang.x += GetFrameTime() * -PI * 2 * 0.14;

    car.ang.y += GetFrameTime();
    car.pos.z += 6 * GetFrameTime() * sin(car.ang.y+PI/2.0);
    car.pos.x += 6 * GetFrameTime() * cos(car.ang.y+PI/2.0);
    
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

