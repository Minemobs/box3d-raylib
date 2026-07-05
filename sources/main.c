#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "raylib.h"
#include <stdio.h>

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Window title"

#define MAX_CUBES_IN_SCENE 256
#define MOV_SPEED 150

#define RV2B3V(v)                                                              \
  (b3Vec3) { v.x, v.y, v.z }

typedef struct {
  Camera3D cam;
  Vector3 velocity;
} Player;

typedef struct {
  Color color;
  Vector3 pos;
  b3Quat rot;
  Vector3 size;
  b3BodyId bodyId;
  Model *model;
} Cube;

typedef struct {
  int currIndex;
  int length;
  Cube cubes[MAX_CUBES_IN_SCENE];
} CircularBoxArray;

const double timestep = 1.0 / 200.0f;

static Cube *addToArray(Cube cube, CircularBoxArray *arr) {
  if (arr->currIndex == MAX_CUBES_IN_SCENE) {
    arr->currIndex = 0;
  }
  arr->cubes[arr->currIndex++] = cube;
  if (arr->length < MAX_CUBES_IN_SCENE)
    arr->length++;
  return arr->cubes + (arr->currIndex - 1);
}

static Cube *createCube(Vector3 pos, Vector3 size, b3WorldId worldId,
                        bool isDynamic, Color color, CircularBoxArray *arr) {
  b3BodyDef groundBodyDef = b3DefaultBodyDef();
  groundBodyDef.position = RV2B3V(pos);
  if (isDynamic)
    groundBodyDef.type = b3_dynamicBody;
  b3BodyId cubeId = b3CreateBody(worldId, &groundBodyDef);
  b3BoxHull boxHull = b3MakeBoxHull(size.x / 2, size.y / 2, size.z / 2);
  b3ShapeDef cubeShapeDef = b3DefaultShapeDef();
  if (isDynamic) {
    cubeShapeDef.density = 1.0f;
    cubeShapeDef.baseMaterial.friction = 0.3f;
  }
  b3CreateHullShape(cubeId, &cubeShapeDef, &boxHull.base);
  Cube cube = {color, pos, {}, size, cubeId};
  return addToArray(cube, arr);
}

static void drawCube3D(Camera3D cam, Cube cube) {
  float angleRad;
  b3Vec3 axis = b3GetAxisAngle(&angleRad, cube.rot);
  float angleDeg = angleRad * RAD2DEG;
  Vector3 axisRay = (Vector3){axis.x, axis.y, axis.z};
  if (cube.model == NULL)
    DrawCubeV(cube.pos, cube.size, cube.color);
  else
    DrawModelEx(*cube.model, cube.pos, axisRay, angleDeg, cube.size,
                cube.color);
  DrawCubeWiresV(cube.pos, cube.size, BLACK);
}

static void updateCube(Cube *cube, b3WorldId worldId) {
  b3Vec3 position = b3Body_GetPosition(cube->bodyId);
  b3Quat rotation = b3Body_GetRotation(cube->bodyId);
  cube->pos = (Vector3){position.x, position.y, position.z};
  cube->rot = rotation;
}

static void physicsUpdate(Player *player, b3WorldId worldId,
                          CircularBoxArray *arr) {
  static bool time_once = false;
  static double t0 = 0.0;
  static double simulation_time = 0.0;
  static double subStepCount = 4.0;

  if (time_once)
    t0 = GetTime();
  double t1 = GetTime();
  double elapsed = t1 - t0;

  if (elapsed > .25) {
    elapsed = .25;
  }

  t0 = t1;
  simulation_time += elapsed;
  while (simulation_time >= timestep) {
    b3World_Step(worldId, timestep, subStepCount);

    for (int i = 0; i < arr->length; i++) {
      updateCube(arr->cubes + i, worldId);
    }
    simulation_time -= timestep;
  }
}

int main(void) {
  CircularBoxArray arr = {};
  Camera3D cam = {
      {.0f, 2.f, 4.f}, {.0, .0, .0}, {.0, 1.f, .0}, 90.f, CAMERA_PERSPECTIVE};
  Player player = {cam};
  b3WorldDef worldDef = b3DefaultWorldDef();
  b3WorldId worldId = b3CreateWorld(&worldDef);
  createCube((Vector3){}, (Vector3){5.0, 1.0, 5.0}, worldId, false, GRAY, &arr);
  Cube *cube = createCube((Vector3){0, 5, 0}, (Vector3){1.0, 1.0, 1.0}, worldId,
                          true, RED, &arr);
  Cube *cube2 = createCube((Vector3){0, 6, 0}, (Vector3){1.0, .5, 1.0}, worldId,
                           true, BLUE, &arr);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  DisableCursor();
  SetTargetFPS(240);

  Model cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
  cube->model = &cubeModel;
  cube2->model = &cubeModel;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_T)) {
      for (int i = 0; i < arr.length; i++) {
        b3Body_ApplyLinearImpulseToCenter(arr.cubes[i].bodyId,
                                          (b3Vec3){0, 10, 0}, true);
      }
    }
    if (IsKeyPressed(KEY_R)) {
      Cube *cube = createCube(
          (Vector3){0, 5, 0}, (Vector3){1.0, 1.0, 1.0}, worldId, true,
          (Color){GetRandomValue(0, 255), GetRandomValue(50, 125),
                  GetRandomValue(100, 200), 255},
          &arr);
      cube->model = &cubeModel;
    }
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(cam);
    for (int i = 0; i < arr.length; i++) {
      drawCube3D(cam, arr.cubes[i]);
    }
    DrawGrid(10, 1.0f);
    physicsUpdate(&player, worldId, &arr);
    UpdateCameraPro(
        &cam,
        (Vector3){
            ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) *
                 0.1f - // Move forward-backward
             (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f) *
                dt * MOV_SPEED,
            ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) *
                 0.1f - // Move right-left
             (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f) *
                dt * MOV_SPEED,
            ((IsKeyDown(KEY_SPACE) * .1f) - (IsKeyDown(KEY_LEFT_SHIFT) * .1f)) *
                dt * MOV_SPEED // Move up-down
        },
        (Vector3){
            GetMouseDelta().x * 0.05f, // Rotation: yaw
            GetMouseDelta().y * 0.05f, // Rotation: pitch
            0.0f                       // Rotation: roll
        },
        GetMouseWheelMove() * 2.0f); // Move to target (zoom)
    EndMode3D();
    DrawFPS(10, 10);
    EndDrawing();
  }

  UnloadModel(cubeModel);
  b3DestroyWorld(worldId);
  CloseWindow();
  return 0;
}
