/**
* Author: Will Roche
* Assignment: Pong Clone
* Date due: 02/28/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include <cmath>

// screen + game constants
constexpr int   SCREEN_WIDTH   = 1600 / 2;
constexpr int   SCREEN_HEIGHT  = 900  / 2;
constexpr int   FPS            = 60;
constexpr char  BG_COLOR[]     = "#0077B6";

constexpr float PADDLE_W       = 30.0f;
constexpr float PADDLE_H       = 115.0f;
constexpr float BALL_SIZE      = 25.0f;
constexpr float PADDLE_SPEED   = 320.0f;
constexpr float BALL_SPEED     = 280.0f;
constexpr float PADDLE_MARGIN  = 40.0f; 
constexpr float WALL_H         = 50.0f;
constexpr int   WIN_SCORE      = 3;
constexpr int   MAX_BALLS      = 3;

// app + delta time state
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;

// paddles and balls (all 3 are always allocated, active ones tracked separately)
Entity *player1 = nullptr;
Entity *player2 = nullptr;
Entity *balls[MAX_BALLS] = { nullptr, nullptr, nullptr };

bool gBallActive[MAX_BALLS] = { true, false, false };
int  gNumBalls = 1;
bool gSinglePlayer = false;

// single player direction, bounces paddle up and down between the wood strips
enum SinglePlayerDirection { SP_UP, SP_DOWN };
SinglePlayerDirection gSPDir = SP_UP;

int gScores[2] = { 0, 0 };  // index 0 p1, index 1 p2
int gWinner = -1; // stays -1 until someone reaches WIN_SCORE

// walls loaded separately since they aren't entities
Texture2D gTopWood;
Texture2D gBottomWood;

// function declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void resetBall(int index);
void setNumBalls(int n);

// sends a ball back to center with a random launch angle from 20-60 degrees (angled)
// trig convert that angle into x and y components of movement vector
void resetBall(int index){
    balls[index]->setPosition({ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f });

    float angle = (float)GetRandomValue(20, 60) * DEG2RAD; //converts degree to radians
    float xDir = 1.0f;
    float yDir = 1.0f;
    if (GetRandomValue(0, 1) == 0) xDir = -1.0f;  // randomly go left or right
    if (GetRandomValue(0, 1) == 0) yDir = -1.0f;  // randomly go up or down

    balls[index]->setMovement({ xDir * cosf(angle), yDir * sinf(angle) });
    balls[index]->setSpeed(BALL_SPEED);
}

// activate the first n balls and deactivate the rest
void setNumBalls(int n){
    gNumBalls = n;
    for (int i = 0; i < MAX_BALLS; i++){
        bool shouldBeActive = (i < gNumBalls);
        if (shouldBeActive && !gBallActive[i]) resetBall(i); // if a ball is being activated reset it from its preloaded state
        gBallActive[i] = shouldBeActive;
    }
}

void initialise(){
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Shrimp Pong");
    SetTargetFPS(FPS);

    // left paddle (shrimp)
    player1 = new Entity(
        { PADDLE_MARGIN, SCREEN_HEIGHT / 2.0f },
        { PADDLE_W, PADDLE_H },
        "assets/game/paddle1.png"
    );
    player1->setSpeed(PADDLE_SPEED);

    // right paddle (shrimp)
    player2 = new Entity(
        { SCREEN_WIDTH - PADDLE_MARGIN, SCREEN_HEIGHT / 2.0f },
        { PADDLE_W, PADDLE_H },
        "assets/game/paddle2.png"
    );
    player2->setSpeed(PADDLE_SPEED);

    // create all 3 balls upfront even if only 1 is active
    for (int i = 0; i < MAX_BALLS; i++){
        balls[i] = new Entity(
            { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
            { BALL_SIZE, BALL_SIZE },
            "assets/game/ball.png"
        );
    }

    gBallActive[0] = true;
    gBallActive[1] = false;
    gBallActive[2] = false;
    resetBall(0);

    // wall textures get stretched to fill the full screen width in render
    gTopWood = LoadTexture("assets/game/topwood.png");
    gBottomWood = LoadTexture("assets/game/bottomwood.png");
}

void processInput(){
    if (WindowShouldClose()) gAppStatus = TERMINATED;

    // once game is over nothing else is interactive, only the game over winner/loser screen
    if (gWinner != -1) return;

    // 123 keys set the number of active balls
    if (IsKeyPressed(KEY_ONE)) setNumBalls(1);
    if (IsKeyPressed(KEY_TWO)) setNumBalls(2);
    if (IsKeyPressed(KEY_THREE)) setNumBalls(3);

    // t toggles between 2 player and 1 player mode
    if (IsKeyPressed(KEY_T)) gSinglePlayer = !gSinglePlayer;

    // reset movement so the last key doesn't carry over
    player1->resetMovement();
    player2->resetMovement();

    // player 1
    if (IsKeyDown(KEY_W)) player1->moveUp();
    if (IsKeyDown(KEY_S)) player1->moveDown();

    if (!gSinglePlayer){
        // player 2
        if (IsKeyDown(KEY_UP))   player2->moveUp();
        if (IsKeyDown(KEY_DOWN)) player2->moveDown();
    }
    else{
        // up and down logic for autoplay
        float paddleY = player2->getPosition().y;
        float half    = PADDLE_H / 2.0f;

        if (gSPDir == SP_UP){
            player2->moveUp();
            if (paddleY - half <= WALL_H) gSPDir = SP_DOWN; // if hitting wood switch direction
        }
        else{
            player2->moveDown();
            if (paddleY + half >= SCREEN_HEIGHT - WALL_H) gSPDir = SP_UP; // if hitting wood switch direction
        }
    }
}

void update(){
    // if game over
    if (gWinner != -1) return; 

    // delta time
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    player1->update(deltaTime);
    player2->update(deltaTime);

    // make sure paddles dont overlap wood by offsetting by half the paddle height on each side
    float paddleHalf = PADDLE_H / 2.0f;

    Vector2 p1pos = player1->getPosition();
    if (p1pos.y < WALL_H + paddleHalf) p1pos.y = WALL_H + paddleHalf;
    if (p1pos.y > SCREEN_HEIGHT - WALL_H - paddleHalf) p1pos.y = SCREEN_HEIGHT - WALL_H - paddleHalf;
    player1->setPosition(p1pos);

    Vector2 p2pos = player2->getPosition();
    if (p2pos.y < WALL_H + paddleHalf) p2pos.y = WALL_H + paddleHalf;
    if (p2pos.y > SCREEN_HEIGHT - WALL_H - paddleHalf) p2pos.y = SCREEN_HEIGHT - WALL_H - paddleHalf;
    player2->setPosition(p2pos);

    for (int i = 0; i < MAX_BALLS; i++){
        if (!gBallActive[i]) continue;

        balls[i]->update(deltaTime);

        Vector2 pos = balls[i]->getPosition();
        Vector2 mov = balls[i]->getMovement();
        float half = BALL_SIZE / 2.0f;

        // balls bounce off top and bottom wood strips
        // absolute value forces the y direction away from the walls so it can't get stuck
        if (pos.y - half <= WALL_H){
            pos.y = WALL_H + half;
            mov.y = abs(mov.y);   // push downward
            balls[i]->setPosition(pos);
            balls[i]->setMovement(mov);
        }
        else if (pos.y + half >= SCREEN_HEIGHT - WALL_H){
            pos.y = SCREEN_HEIGHT - WALL_H - half;
            mov.y = -abs(mov.y);  // push upward
            balls[i]->setPosition(pos);
            balls[i]->setMovement(mov);
        }

        // reorient after wall correction before paddle checks
        pos = balls[i]->getPosition();
        mov = balls[i]->getMovement();

        //actual paddle collision logic
        if (balls[i]->checkCollision(player1)){
            mov.x = abs(mov.x);   // must go right (away from P1)
            pos.x = player1->getPosition().x + PADDLE_W / 2.0f + half + 2.0f;  // push outside paddle
            balls[i]->setPosition(pos);
            balls[i]->setMovement(mov);
        }
        if (balls[i]->checkCollision(player2)){
            mov.x = -abs(mov.x);  // must go left (away from P2)
            pos.x = player2->getPosition().x - PADDLE_W / 2.0f - half - 2.0f;  // push outside paddle
            balls[i]->setPosition(pos);
            balls[i]->setMovement(mov);
        }

        // if ball exits, ball resets to center and goal is scored
        pos = balls[i]->getPosition();
        if (pos.x - half <= 0.0f){
            gScores[1]++;
            resetBall(i);
        }
        else if (pos.x + half >= SCREEN_WIDTH){
            gScores[0]++;
            resetBall(i);
        }
    }

    // check win after all scoring is done
    if (gScores[0] >= WIN_SCORE) gWinner = 0;
    if (gScores[1] >= WIN_SCORE) gWinner = 1;
}

void render(){
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOR));

    // stretch wood texture across the screen
    DrawTexturePro(gTopWood,
        { 0, 0, (float)gTopWood.width, (float)gTopWood.height },
        { 0, 0, (float)SCREEN_WIDTH, WALL_H },
        { 0, 0 }, 0.0f, WHITE);
    DrawTexturePro(gBottomWood,
        { 0, 0, (float)gBottomWood.width, (float)gBottomWood.height },
        { 0, SCREEN_HEIGHT - WALL_H, (float)SCREEN_WIDTH, WALL_H },
        { 0, 0 }, 0.0f, WHITE);

    // paddles always render; balls only if active
    player1->render();
    player2->render();
    for (int i = 0; i < MAX_BALLS; i++)
        if (gBallActive[i]) balls[i]->render();

    // scoreboard with drawtext function
    DrawText(TextFormat("%d", gScores[0]), SCREEN_WIDTH / 4, WALL_H + 20, 50, WHITE);
    DrawText(TextFormat("%d", gScores[1]), 3 * SCREEN_WIDTH / 4, WALL_H + 20, 50, WHITE);

    if (gWinner != -1){
        const char *msg;
        if (gWinner == 0) msg = "Player 1 Wins!";
        else msg = "Player 2 Wins!";
        int fontSize = 60;
        int textW = MeasureText(msg, fontSize);
        DrawText(msg, (SCREEN_WIDTH - textW) / 2, SCREEN_HEIGHT / 2 - fontSize / 2, fontSize, WHITE);
    }

    EndDrawing();
}

void shutdown(){
    //cleanup
    delete player1;
    delete player2;
    for (int i = 0; i < MAX_BALLS; i++) delete balls[i];
    UnloadTexture(gTopWood);
    UnloadTexture(gBottomWood);
    CloseWindow();
}

int main(void){
    initialise();

    while (gAppStatus == RUNNING){
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}
