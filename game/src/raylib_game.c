#include "raylib.h"
#include <stdbool.h>
#include <math.h>

#define MAX_NAME_SIZE 256
#define MAX_TEAM_COUNT 64
#define TEAM_COLORS_LEN 20

static const int screenWidth = 1920 - 64;
static const int screenHeight = 1080 - 128;

static const int teamNameSize = 80;

static Color teamColors[20] = {
    { 200, 200, 200, 255 },
    { 130, 130, 130, 255 },
    { 80, 80, 80, 255 },
    { 255, 203, 0, 255 },
    { 255, 161, 0, 255 },
    { 255, 109, 194, 255 },
    { 230, 41, 55, 255 },
    { 190, 33, 55, 255 },
    { 0, 228, 48, 255 },
    { 0, 158, 47, 255 },
    { 0, 117, 44, 255 },
    { 102, 191, 255, 255 },
    { 0, 121, 241, 255 },
    { 0, 82, 172, 255 },
    { 200, 122, 255, 255 },
    { 135, 60, 190, 255 },
    { 112, 31, 126, 255 },
    { 211, 176, 131, 255 },
    { 127, 106, 79, 255 },
    { 76, 63, 47, 255 },
};

typedef struct Team {
    char name[MAX_NAME_SIZE];
    int score;
    Color color;
    bool hidden;
} Team;

Team teamList[MAX_TEAM_COUNT];

static void UpdateDrawFrame(void);          // Update and draw one frame

void InitTeams() {
    for (int i = 0; i < MAX_TEAM_COUNT; i++) {
        Team* team = &teamList[i];
        strcpy(team->name, "N/A");
        team->score = 0;
        team->color = BLACK;
        team->hidden = true;
    }

    for (int i = 0; i < 32; i++) {
        Team* team = &teamList[i];
        sprintf(team->name, "Goobers #%d", i);
        team->color = teamColors[rand() % TEAM_COLORS_LEN];
        team->hidden = false;
    }
}

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Web Scoreboard");

    SetTargetFPS(60);

    InitTeams();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        UpdateDrawFrame();
        EndDrawing();
    }

    CloseWindow();          // Close window and OpenGL context

    return 0;
}

static void UpdateDrawFrame(void)
{
    
    ClearBackground(ColorFromHSV(GetTime() * 10.0, 0.8, 0.8));
    for (int i = 0; i < MAX_TEAM_COUNT; i++) {
        Team* team = &teamList[i];
        if (team->hidden) continue;
        const int maxTeamsInColumn = 8;
        const int columnWidth = screenWidth / 2 - 64;
        const int lineSpacing = 8;
        const int margin = 16;
        const int padding = 4;
        const int lineYPos = (lineSpacing + teamNameSize) * (i % maxTeamsInColumn);
        const int lineXPos = (i / maxTeamsInColumn) * columnWidth;
        const Vector2 offset = { sin(GetTime() + i) * 16 + 32, cos(GetTime()) * 16 + 32 };

        // Shadow
        const int shadowLength = 8;
        DrawRectangle(
            lineXPos + margin - padding + offset.x + shadowLength,
            lineYPos - padding + offset.y + shadowLength,
            columnWidth,
            teamNameSize + padding,
            (Color){ 0, 0, 0, 100 }
        );

        DrawRectangle(
            lineXPos + margin - padding + offset.x,
            lineYPos - padding + offset.y,
            columnWidth,
            teamNameSize + padding,
            WHITE
        );

        DrawText(
            team->name,
            lineXPos + margin + offset.x,
            lineYPos + offset.y,
            teamNameSize,
            team->color
        );

        const int scoreMargin = 700;
        char scoreAsStr[32];
        sprintf(scoreAsStr, "%d", team->score);

        DrawText(
            scoreAsStr,
            lineXPos + scoreMargin + margin + offset.x,
            lineYPos + offset.y,
            teamNameSize,
            team->color
        );
    }
    
}
