/*******************************************************************************************
*
*   raylib [models] example - full solar system
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019 Aldrin Martoq (@aldrinmartoq)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#define MAX_BODY_CHILDREN 10

float rotationSpeed = 0.2;

// A celestial body that has children bodies orbiting around
typedef struct Body {
    const char *label;                          // label of the body, for ex: moon
    float radius;                               // object radius
    float orbitRadius;                          // orbit average radius
    float orbitPeriod;                          // time the body takes to do a full orbit loop
    float rotationPeriod;                       // time the body takes to do a full rotation on itself
    Texture2D texture;                          // texture of the body
    Model model;                                // model of the body
    float orbitPosition;                        // current orbit position
    float rotationPosition;                     // current rotation position
    Vector2 labelPosition;                      // label position in screen
    struct Body *children[MAX_BODY_CHILDREN];   // children array
    int childrenCount;                          // children count
} Body;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
Body CreateBody(float radius, float orbitRadius, float orbitPeriod, const char *label, const char *texture); // Initializes a new Body with the given parameters
void AddBodyChildren(Body *parent, Body *children); // Add a children body to the parent body
void DrawBody(Body *body, Camera *camera);          // Draw body and its children, updating labelPosition
void DrawBodyLabel(Body *body);                        // Draw body label and its children labels

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - solar system");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 20.0f, 12.0f, 20.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FREE);

    // Create Bodies
    Body sun        = CreateBody(0.2,     0.0,     0, "sun",      "2k_sun");
    Body moon       = CreateBody(0.02,  0.200,    24, "moon",     "2k_moon");
    Body mercury    = CreateBody(0.05,  0.396,    90, "mercury",  "2k_mercury");
    Body venus      = CreateBody(0.05,  0.723,   210, "venus",    "2k_venus_atmosphere");
    Body earth      = CreateBody(0.05,  1.000,   365, "earth",    "2k_earth_daymap");
    Body mars       = CreateBody(0.05,  1.523,   690, "mars",     "2k_mars");
    Body jupiter    = CreateBody(0.05,  5.200,  4260, "jupiter",  "2k_jupiter");
    Body saturn     = CreateBody(0.05,  9.532, 10620, "saturn",   "2k_saturn");
    Body uranus     = CreateBody(0.05, 19.180, 30270, "uranus",   "2k_uranus");
    Body neptune    = CreateBody(0.05, 30.056, 59370, "neptune",  "2k_neptune");
    Body pluto      = CreateBody(0.05, 39.463, 89310, "pluto",    "2k_eris_fictional");

    AddBodyChildren(&sun, &mercury);
    AddBodyChildren(&sun, &venus);
    AddBodyChildren(&sun, &earth);
    AddBodyChildren(&sun, &mars);
    // AddBodyChildren(&sun, &jupiter);
    // AddBodyChildren(&sun, &saturn);
    // AddBodyChildren(&sun, &uranus);
    // AddBodyChildren(&sun, &neptune);
    // AddBodyChildren(&sun, &pluto);

    AddBodyChildren(&earth, &moon);

    bool showHelpMenu = false;
    bool showBodyLabels = true;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);

        if (IsKeyPressed(KEY_H)) showHelpMenu = !showHelpMenu;
        if (IsKeyPressed(KEY_L)) showBodyLabels = !showBodyLabels;

        if (IsKeyPressed(KEY_LEFT)) rotationSpeed -= 0.1;
        if (IsKeyPressed(KEY_RIGHT)) rotationSpeed += 0.1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(camera);

                // NOTE: DrawBody calls recusively body childrens
                DrawBody(&sun, &camera);

            EndMode3D();

            if (showBodyLabels) DrawBodyLabel(&sun);

            DrawText(FormatText("FULL SOLAR SYSTEM - SPEED: %2.2f", rotationSpeed), 120, 10, 20, LIME);

            if (showHelpMenu) DrawText("- [h] Toggle help\n- [l] Toggle labels\n- [left/right arrows] Increase/decrease rotation speed", 10, 40, 10, GREEN);
            else DrawText("Press [h] for help", 10, 40, 10, GREEN);
            
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//--------------------------------------------------------------------------------------------

// Creates a new body
Body CreateBody(float radius, float orbitRadius, float orbitPeriod, const char *label, const char *texture_name)
{
    Body body = { 0 };
    Texture2D texture = LoadTexture(FormatText("resources/solar_system/%s.png", texture_name)); // GenImageCellular()

    GenTextureMipmaps(&texture);

    body.label = label;
    body.radius = radius * 10;
    body.orbitRadius = orbitRadius * 10;
    body.orbitPeriod = orbitPeriod;
    body.model = LoadModel("resources/solar_system/sphere.obj");    // GenMeshSphere()
    body.model.materials[0].maps[MAP_DIFFUSE].texture = texture;
    body.childrenCount = 0;
    body.orbitPosition = 0.0;
    
    return body;
}

void AddBodyChildren(Body *parent, Body *children) 
{
    if (parent->childrenCount >= MAX_BODY_CHILDREN) TraceLog(LOG_ERROR, "ERROR: Body has too many children!");
    else
    {
        parent->children[parent->childrenCount] = children;
        parent->childrenCount++;
    }
}

// Draw body and its children
void DrawBody(Body *body, Camera *camera)
{
    DrawModel(body->model, (Vector3){ 0.0f, 0.0f, 0.0f}, body->radius, WHITE);

    // TODO: labelPosition is not transformed to drawing position
    body->labelPosition = GetWorldToScreen((Vector3){ body->orbitRadius, body->radius, 0.0 }, *camera);

    for (int i = 0; i < body->childrenCount; i++) 
    {
        Body *child = body->children[i];
        child->orbitPosition += rotationSpeed*360/child->orbitPeriod;
        
        rlPushMatrix();
            rlRotatef(child->orbitPosition, 0.0, 1.0, 0.0);
            rlTranslatef(child->orbitRadius, 0.0, 0.0);
            rlRotatef(-child->orbitPosition, 0.0, 1.0, 0.0);

            DrawBody(child, camera);
        rlPopMatrix();

        DrawCircle3D((Vector3){ 0.0f, 0.0f, 0.0f }, child->orbitRadius, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, GRAY);
    }
}

// Draw body label and its children labels
void DrawBodyLabel(Body *body)
{
    DrawText(body->label, body->labelPosition.x - MeasureText(body->label, 20)/2, body->labelPosition.y, 20, WHITE);

    for (int i = 0; i < body->childrenCount; i++)
    {
        Body *child = body->children[i];
        DrawBodyLabel(child);
    }
}
