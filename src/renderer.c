#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

Vector3 sun_dir;
RenderCamera current_camera;

void set_sun_dir(Vector3 _sun_dir){
  sun_dir = _sun_dir;
}

void set_current_camera(RenderCamera new){
  current_camera = new;
}

RenderCamera *get_current_camera(){
  return &current_camera;
}

Triangle **trs;
int trs_cnt;

void trs_init(){
  trs = malloc(sizeof(Triangle*)*MAX_TRIANGLES);
  for(int i = 0; i < MAX_TRIANGLES; i++){
    trs[i] = malloc(sizeof(Triangle));
  }
}

void trs_reset(){
  trs_cnt = 0;
}

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

Color getPixel(Image img, Vector2 uv){
  uv.x = MAX(MIN(uv.x, 1.0f), 0.0f);
  uv.y = MAX(MIN(uv.y, 1.0f), 0.0f);

  int x = (int)(uv.x * (img.width - 1) + 0.5f);
  int y = (int)(uv.y * (img.height - 1) + 0.5f);
  
  y = img.height - 1 - y;

  return GetImageColor(img, x, y);
}

Vector3 calc(Vector3 v, RenderObject *r, Vector3 *nv){
  v.x *= r->sc.x;
  v.y *= r->sc.y;
  v.z *= r->sc.z;
  v = rotateY(v, r->ang.y);
  v = rotateX(v, r->ang.x);
  v = rotateZ(v, r->ang.z);
  v = Vector3Subtract(v, current_camera.pos);
  v = Vector3Add(v, r->pos);
  *nv = v;
  v = rotateY(v, -current_camera.ang.y);
  v = rotateX(v, -current_camera.ang.x);
  v = rotateZ(v, -current_camera.ang.z);
  return v;
}

void calc_triangle(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 n, Color clr) {
  if (v1.z < 0 && v2.z < 0 && v3.z < 0) return;
  if (v1.z < 0) v1.z = 0.01;
  if (v2.z < 0) v2.z = 0.01;
  if (v3.z < 0) v3.z = 0.01;

  float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
  float fovScale = tanf(current_camera.fov * 0.5f * (PI / 180.0f));

  Vector2 p1 = { (v1.x / (v1.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v1.y / (v1.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
  Vector2 p2 = { (v2.x / (v2.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v2.y / (v2.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };
  Vector2 p3 = { (v3.x / (v3.z * fovScale * aspectRatio)) * SCREEN_WIDTH * 0.5f + 0.5f * SCREEN_WIDTH,
		 -(v3.y / (v3.z * fovScale)) * SCREEN_HEIGHT * 0.5f + 0.5f * SCREEN_HEIGHT };

  Vector3 center = FindTriangleCentroid(v1, v2, v3);
  trs[trs_cnt++][0] = (Triangle){
    .distToCam = Vector3Distance((Vector3){0, 0, 0}, center),
    .p1 = p1, .p2 = p2, .p3 = p3, .n = n,
    .clr = clr,
  };
}

void draw_render_object(RenderObject *r){
  for(int fi = 0; fi < r->rm.fn; fi++){
    Vector2 clr_pos = {0, 0};
    Color clr = WHITE;
    if(r->isTxUsing){
      for(int vti = 0; vti < r->rm.fs[fi].cnt; vti++){
	int ind = r->rm.fs[fi].vti[vti] - 1;
	clr_pos = Vector2Add(clr_pos, r->rm.vts[ind]);
      }
    
      clr_pos.x /= r->rm.fs[fi].cnt;
      clr_pos.y /= r->rm.fs[fi].cnt;
      clr = getPixel(r->tx, clr_pos);
    }
    
    Vector3 nv1, nv2, nv3;
    int p1 = r->rm.fs[fi].vi[0] - 1;
    Vector3 v1 = calc(r->rm.vs[p1], r, &nv1);
    for(int vi = 1; vi < r->rm.fs[fi].cnt-1; vi++){
      int p2 = r->rm.fs[fi].vi[vi] - 1;
      int p3 = r->rm.fs[fi].vi[vi+1] - 1;
      
      Vector3 v2 = calc(r->rm.vs[p2], r, &nv2);
      Vector3 v3 = calc(r->rm.vs[p3], r, &nv3);
      
      Vector3 triangleNormal = CalculateTriangleNormal(nv1, nv2, nv3);

      calc_triangle(v1, v2, v3, triangleNormal, clr);
    }
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
    float dotProduct = Vector3DotProduct(trs[i]->n, Vector3Normalize(sun_dir));
    dotProduct = MAX(dotProduct, 0.15);
    Color col = {(int)(dotProduct*255.0), (int)(dotProduct*255.0), (int)(dotProduct*255.0), 255};
    col.r = (unsigned char)((float)(col.r)/255 * (float)(trs[i]->clr.r)/255 * 255);
    col.g = (unsigned char)((float)(col.g)/255 * (float)(trs[i]->clr.g)/255 * 255);
    col.b = (unsigned char)((float)(col.b)/255 * (float)(trs[i]->clr.b)/255 * 255);
    DrawTriangle(trs[i]->p2, trs[i]->p1, trs[i]->p3, col);
  }
}
