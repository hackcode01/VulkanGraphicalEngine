# Game

## Definitions

Launches the engine's graphical window and utilizes all of its functionality. It serves as the system's launch point and client component.

## Structures

- GameState

### Fields

- f32 deltaTime

## Functions

- b8 gameInitialize(Game* gameInstance)

- b8 gameUpdate(Game* gameInstance, f32 deltaTime)

- b8 gameRender(Game* gameInstance, f32 deltaTime)

- void gameOnResize(Game* gameInstance, u32 width, u32 height)

- b8 createGame(Game* outGame)

- int main(void)
