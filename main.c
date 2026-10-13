#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_ITEMS 64
#define MAX_LEN 128

static char items[MAX_ITEMS][MAX_LEN];
static int itemCount = 0;

static const Color palette[] = {
    {239, 71, 111, 255}, {255, 209, 102, 255}, {6, 214, 160, 255},
    {17, 138, 178, 255}, {124, 63, 208, 255}, {255, 107, 53, 255},
    {46, 134, 222, 255}, {247, 37, 133, 255}, {87, 204, 153, 255},
    {66, 71, 105, 255},
};
#define PALETTE_COUNT ((int)(sizeof(palette) / sizeof(palette[0])))

static void trim(char *s)
{
    int n = (int)strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' ||
                     s[n - 1] == '\n' || s[n - 1] == '\r'))
        s[--n] = '\0';
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t')
        start++;
    if (start)
        memmove(s, s + start, strlen(s + start) + 1);
}

static int parseItems(const char *arg)
{
    if (!arg)
        return 0;
    if (arg[0] == '-')
        arg++;

    char buf[4096];
    strncpy(buf, arg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *p = buf;
    size_t len = strlen(p);
    if (len >= 2 && p[0] == '[' && p[len - 1] == ']') {
        p[len - 1] = '\0';
        p++;
    } else {
        char *open = strchr(p, '[');
        if (open)
            p = open + 1;
        char *close = strchr(p, ']');
        if (close)
            *close = '\0';
    }

    char *tok = strtok(p, ",");
    while (tok && itemCount < MAX_ITEMS) {
        trim(tok);
        if (*tok) {
            strncpy(items[itemCount], tok, MAX_LEN - 1);
            items[itemCount][MAX_LEN - 1] = '\0';
            itemCount++;
        }
        tok = strtok(NULL, ",");
    }
    return itemCount;
}

int main(int argc, char **argv)
{
    parseItems(argc > 1 ? argv[1] : NULL);

    if (itemCount < 2) {
        fprintf(stderr,
                "Usage: %s -[item1,item2,item3,...]\n"
                "Example: %s -[Pizza,Burger,Sushi,Tacos]\n",
                argv[0], argv[0]);
        return 1;
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(900, 700, "Spin the Wheel");
    SetTargetFPS(60);

    const Vector2 center = {450.0f, 350.0f};
    const float radius = 260.0f;
    const float segAngle = 360.0f / (float)itemCount;

    float rotation = 0.0f;
    float startRotation = 0.0f;
    float targetRotation = 0.0f;
    float spinT = 1.0f;
    const float spinDuration = 4.0f;

    int resultIndex = -1;
    int lastResult = -1;

    while (!WindowShouldClose()) {
        bool spinning = spinT < 1.0f;

        if (!spinning &&
            (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
            startRotation = rotation;
            int fullTurns = 6 + GetRandomValue(0, 3);
            float jitter = (float)GetRandomValue(0, 3599) / 10.0f;
            targetRotation = rotation + fullTurns * 360.0f + jitter;
            spinT = 0.0f;
            resultIndex = -1;
        }

        if (spinT < 1.0f) {
            spinT += GetFrameTime() / spinDuration;
            if (spinT >= 1.0f)
                spinT = 1.0f;
            float eased = 1.0f - powf(1.0f - spinT, 3.0f);
            rotation = startRotation + (targetRotation - startRotation) * eased;

            if (spinT >= 1.0f) {
                float rel = fmodf(270.0f - rotation, 360.0f);
                if (rel < 0.0f)
                    rel += 360.0f;
                resultIndex = (int)(rel / segAngle) % itemCount;
                lastResult = resultIndex;
            }
        }

        BeginDrawing();
        ClearBackground((Color){24, 26, 32, 255});

        for (int i = 0; i < itemCount; i++) {
            float a0 = rotation + i * segAngle;
            float a1 = a0 + segAngle;
            bool highlight = (resultIndex == i);
            Color c = palette[i % PALETTE_COUNT];
            if (highlight)
                c = ColorBrightness(c, 0.35f);
            DrawCircleSector(center, radius, a0, a1, 8, c);
            DrawCircleSectorLines(center, radius, a0, a1, 8, Fade(BLACK, 0.35f));

            float mid = (a0 + a1) * 0.5f * DEG2RAD;
            float tx = center.x + cosf(mid) * radius * 0.62f;
            float ty = center.y + sinf(mid) * radius * 0.62f;
            const char *label = items[i];
            int fs = 24;
            int tw = MeasureText(label, fs);
            DrawText(label, (int)(tx - tw / 2.0f), (int)(ty - fs / 2.0f), fs, RAYWHITE);
        }

        DrawCircleV(center, 34.0f, (Color){24, 26, 32, 255});
        DrawCircleLinesV(center, 34.0f, RAYWHITE);

        DrawTriangle(
            (Vector2){center.x - 22.0f, center.y - radius - 34.0f},
            (Vector2){center.x + 22.0f, center.y - radius - 34.0f},
            (Vector2){center.x, center.y - radius + 6.0f},
            (Color){255, 255, 255, 255});

        if (lastResult >= 0) {
            const char *line = TextFormat("Result: %s", items[lastResult]);
            int fs = 40;
            int tw = MeasureText(line, fs);
            DrawText(line, (int)(center.x - tw / 2.0f), 20, fs, GOLD);
        } else {
            const char *line = "Press SPACE or click to spin";
            int fs = 22;
            int tw = MeasureText(line, fs);
            DrawText(line, (int)(center.x - tw / 2.0f), 28, fs, Fade(RAYWHITE, 0.7f));
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
