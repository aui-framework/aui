# Minesweeper Game

<!-- aui:example app -->
Minesweeper game implementation driven by [ass].

![](../imgs/Screenshot_20250211_052511.png)

## Game Logic

The entire game logic is located in MinesweeperWindow.cpp.

### Initialization

The game starts with `beginGame` function. `beginGame` Initializes a new Minesweeper game with specified dimensions and
bomb count. Clears any previous game state, sets up the grid layout, initializes cell views, connects click handlers for
opening cells and flagging, and finally packs UI elements to be displayed.

- Generates a grid with `AGridLayout`
- Dynamically creates `CellView`s based on game data
- Handles user events (left clicks and right clicks)

<!-- aui:snippet examples/app/minesweeper/src/MinesweeperWindow.cpp beginGame -->

### Event handling

As was mentioned, the game handles events by setting signal handlers on `CellView`. This is done inside
`setupEventHandlers`. Here's a little breakdown:

#### Toggle flag (right click)

In a minesweeper game, right click toggles the flag. First, we check that game is not finished (`mReveal`), then we
check if cell is open. Finally, we toggle the flag:

<!-- aui:snippet examples/app/minesweeper/src/MinesweeperWindow.cpp clickedRight -->

As you can see, game data is represented by `FieldCell` enum:

<!-- aui:snippet examples/app/minesweeper/src/FieldCell.h FieldCell -->

`FieldCell` is just a bitfield implemented thanks to [AUI_ENUM_FLAG].

#### Open cell (left click)

Left click delegates cell opening logic to `openCell` which contain game-specific logic.

<!-- aui:snippet examples/app/minesweeper/src/MinesweeperWindow.cpp clicked -->

## Cell style

In response to user's actions, the game updates its internal state. To visualize the state, `updateCellViewStyle`
function is called. This method updates the style of a cell at coordinates (x, y) based on its state or any other
conditions (e.g., game over, win/lose states). It triggers an event to let the framework know that custom styles need to
be re-evaluated and applied accordingly.

In game style sheets, a custom selector tied to `CellView` is used to display various cell states:

<!-- aui:snippet examples/app/minesweeper/src/Style.cpp CellSelector -->

Such selector is used in style sheets accordingly:

<!-- aui:snippet examples/app/minesweeper/src/Style.cpp open -->

Also, for reveal game state (on win/lose) there's an additional selector:

<!-- aui:snippet examples/app/minesweeper/src/Style.cpp RevealSelector -->

`RevealSelector` and `CellSelector` are used together to show game results:

![](imgs/Screenshot_20250211_051126.png)

<!-- aui:snippet examples/app/minesweeper/src/Style.cpp reveal -->
