# Game of Life
Conway's Game of life implemented in C using SDL
The game implementation is taken from antirez's implementation done in it's Youtube [channel](https://www.youtube.com/watch?v=c5atNuYdKK8) 

To install SDL you can follow their [guide](https://wiki.libsdl.org/SDL2/Installation)

To compile run 
`make`
Then run 
`./game_of_life`



The application does not implement renderering hence there could be a lot of performance bugs
The n_rows and n_cols (grid size) are passed dynamically in order to implement window size updates in the future
 
To start the game select right clicking the cells you want as ALIVE cells and press the key ENTER
You can also use mouse motion while pressing the right button to select multiple cells faster
To end the game press the SPACE key
