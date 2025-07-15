# Game of Life
Conway's Game of life implemented in C using SDL
The game implementation is taken from antirez's implementation done in it's Youtube [channel](https://www.youtube.com/watch?v=c5atNuYdKK8) 

To install SDL you can follow their [guide](https://wiki.libsdl.org/SDL2/Installation)

To compile run
 
`make`

Then run 

`./game_of_life`


The n_rows and n_cols (grid size) are passed dynamically in order to implement window size updates in the future (probably never)
 
To start the game select right clicking the cells you want as ALIVE cells and press the key ENTER

You can also use mouse motion while pressing the right button to select multiple cells faster

To end the game and clear the grid press the SPACE key

To pause the game in the current state press the key P, you can also add more alive cells while the game is paused

To change the animation speed (iterations per second) by SPEED_UNIT, press U to increase and D to decrease

