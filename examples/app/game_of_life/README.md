# Game of Life

<!-- aui:example app -->
Game of Life implementation that uses advanced large dynamic data rendering techniques such as [ITexture],
[AImage] to be GPU friendly. The computation is performed in [AThreadPool].

![](imgs/eb3f5607-5dfb-4f58-958c-ed513cf3161e.jpeg)

Conway's Game of Life, a cellular automaton devised by the mathematician John Conway. The game consists of a grid where
each cell can be either alive or dead, and its state evolves over time according to simple rules based on the states of
adjacent cells.

> Every cell interacts with its eight neighbours, which are the cells that are horizontally, vertically, or diagonally adjacent. At each step in time, the following transitions occur:
>
> - Any live cell with fewer than two live neighbours dies, as if by underpopulation.
> - Any live cell with two or three live neighbours lives on to the next generation.
> - Any live cell with more than three live neighbours dies, as if by overpopulation.
> - Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
> 
> [Wikipedia](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

## Cells

Represents the grid of cells with their states (`CellState::ALIVE` or `CellState::DEAD`). It has methods for
initialization and randomization, as well as accessors to get the size and state of individual cells.

Computations are handled asynchronously by an AThreadPool, improving performance during state transitions or rule
computations in large grids.

@dontinclude examples/app/game_of_life/src/main.cpp
@skip class Cells
@until /// end

## CellsView

Visualizes the grid using a texture. It updates the texture when cells change their states. When the pointer is pressed
on the view, it toggles the state of the cell under the cursor.

The grid is composed by lines in `drawGrid` lambda. These lines are then passed to the rendering, making a single draw
call to draw the grid.

@skip class CellsView
@until /// end

## GameOfLifeWindow

This is the main window for the Game of Life application. It sets up the UI with buttons to randomize the grid and
start/pause the game, as well as a central area where the `CellsView` is displayed.

@skip class GameOfLifeWindow
@until /// end
