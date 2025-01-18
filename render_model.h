typedef struct {
  int *vi, *vti, *vni;
  int cnt;
} Face;

typedef struct {
  Vector3 *vs, *vns;
  Vector2 *vts;
  Face *fs;
  int vn, vnn, vtn, fn;
} RenderModel;

RenderModel parse_obj(const char *filename);
