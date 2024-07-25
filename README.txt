Level Creation
--------------

Levels will be specified in text files.

Level Specification
-------------------

- One entity specification per line.

- Coordinates are given in grid coordinates.

- Each grid cell is 64 by 64 pixels.

- (0,0) is the at the bottom left.

- Tile & decoration entities should be positioned such that
    the bottom left of grid cell is aligned with the bottom left 
    of the entity.

- Velocity is in units of pixels/frame.

- Acceleration (gravity) is in units of pixels/frame^2.

- Time is based on frames.

Tile Entity Specification:
Tile N GX GY
    Animation Name:     N (string, asset name)
    GX Grid X Pos:      GX (float)
    GY Grid Y Pos:      GY (float)

Decoration Entity Specification:
Dec N GX GY
    Animation Name      N (string, asset name)
    GX Position         X (float)
    GY Position         Y (float)

Player Specification:
Player GX GY CW CH SX SY SM GY B
    GX, GY Grid Pos     X, Y (float, float)
    BoundingBox W/H     CW,CH (float, float)
    Left/Right Speed    SX (float)
    Jump Speed          SY (float)
    Max Speed           SM (float)
    Gravity             GY (float)
    Bullet Animation    B (string)
