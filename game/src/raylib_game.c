#include "raylib.h"
#include <stdbool.h>
#include <math.h>
#include <curl/curl.h>
#include <jansson.h>

#define MAX_NAME_SIZE 256
#define MAX_TEAM_COUNT 64
#define TEAM_COLORS_LEN 20

#define API_KEY "AIzaSyDdPUw7fL6B3tduJKHV9NyU6wveOl3Re-U" // Please be nice
#define SPREADSHEET_ID "1JXpmjPBp9jPgNV9JzadyE0ZJ1xovF4pDf5BpP50aYlg"
#define SHEET_NAME "Sheet1"
#define SHEET_ID "0"
#define GOOGLE_SHEETS_URL "GET https://sheets.googleapis.com/v4/spreadsheets/1JXpmjPBp9jPgNV9JzadyE0ZJ1xovF4pDf5BpP50aYlg/values/Sheet1!A:B?key=AIzaSyDdPUw7fL6B3tduJKHV9NyU6wveOl3Re-U"

#define JSON_BUF_SIZE 4096

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

void populateTeamListFromGoogleSheet(char* apiKey) {
    // Construct the URL for the Google Sheets API endpoint
    char url[200];
    sprintf(url, "%s/%s/values/Sheet1!A:B?key=%s", GOOGLE_SHEETS_URL, SHEET_ID, apiKey);

    // Create a CURL handle and set the URL and HTTP headers
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Create a buffer to hold the JSON response
        char jsonBuf[JSON_BUF_SIZE];
        memset(jsonBuf, 0, JSON_BUF_SIZE);

        // Set up the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &responseHandler);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, jsonBuf);

        // Perform the HTTP GET request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Error performing HTTP request: %s\n", curl_easy_strerror(res));
        }
        else {
            // Parse the JSON response
            json_error_t error;
            json_t* root = json_loads(jsonBuf, 0, &error);
            if (!root) {
                fprintf(stderr, "Error parsing JSON response: %s\n", error.text);
            }
            else {
                // Extract the team names and scores from the response and populate teamList
                json_t* values = json_object_get(json_object_get(root, "values"), "value");
                int num_teams = json_array_size(values) - 1; // subtract 1 to exclude the header row
                if (num_teams > MAX_TEAM_COUNT) {
                    fprintf(stderr, "Error: too many teams in Google Sheet (max %d)\n", MAX_TEAM_COUNT);
                }
                else {
                    for (int i = 0; i < num_teams; i++) {
                        json_t* row = json_array_get(values, i + 1); // add 1 to skip the header row
                        const char* name = json_string_value(json_array_get(row, 0));
                        int score = json_integer_value(json_array_get(row, 1));
                        if (strlen(name) >= MAX_NAME_SIZE) {
                            fprintf(stderr, "Error: team name is too long (max %d characters)\n", MAX_NAME_SIZE - 1);
                        }
                        else {
                            strcpy(teamList[i].name, name);
                            teamList[i].score = score;
                        }
                    }
                    // Free the JSON root object
                    json_decref(root);
                }
            }

            // Clean up
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
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
