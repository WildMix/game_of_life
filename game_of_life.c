/*
 *	Conway's Game of life implemented in C using SDL as the graphic library
 *	The game implementation is taken from antirez (Salvatore Sanfilippo) version reproduced in it's youtube channel
 *	The application does not implement renderering hence there could be a lot of performance bugs
 *	The n_rows and n_cols (grid size) are passed dynamically in order to implement window size updates in the future
 *
 *	To start the game select right clicking the cells you want as ALIVE cells and press the key ENTER
 *	You can also use mouse motion while pressing the right button to select multiple cells faster
 *	To end the game and clear the grid press the SPACE key
 *	To pause the game in the current state press the key P, you can also add more alive cells while the game is pause
 *	To change the animation speed (iterations per second) by SPEED_UNIT, press U to increase and D to decrease
 * */


#include <SDL.h>
#define INITIAL_WIDTH 1000
#define INITIAL_HEIGHT 1000
#define CELL_SIZE 5	
#define LINE_WIDTH 0	// grid's lines width  
#define MAX_SPEED_VALUE 200
#define MIN_SPEED_VALUE 1
#define SPEED_UNIT 5
const SDL_Color alive_cell_color = {0xFF, 0xFF, 0xFF, 0xFF}; // default white
const SDL_Color dead_cell_color = {0x00, 0x00, 0x00, 0x00}; // default black
const SDL_Color cell_lines_color = {0xFF, 0xFF, 0xFF, 0xFF};
typedef enum
{
	ALIVE,
	DEAD,
	NO_STATE	// used before starting the game or when the game is paused to set the cell as the opposite of the current value
} STATE;


typedef struct
{
	SDL_Rect rect;
	STATE state;	
} CELL;

/* Translate the specified x,y grid point into the index in
 * the linear array. This function implements wrapping, so
 * both negative and positive coordiantes that are out of the
 * grid will wrap around. */
int cell_to_index(int x, int y, int n_rows, int n_cols) {
    if (x < 0) {
        x = (-x) % n_cols;
        x = n_cols - x;
    }
    if (y < 0) {
        y = (-y) % n_rows;
        y = n_rows - y;
    }
    if (x >= n_cols) x = x % n_cols;
    if (y >= n_rows) y = y % n_rows;

    return y*n_cols+x;
}

void set_cell(CELL *grid, int x, int y, CELL new_cell, int n_rows, int n_cols) {
	grid[cell_to_index(x, y, n_rows, n_cols)] = new_cell;
}

void set_state_of_cell(CELL* grid, int x, int y, STATE state, int n_rows, int n_cols)
{
	// if no state is selected, sets ALIVE if it's DEAD or DEAD if it's ALIVE
	if (state == NO_STATE)
	{
		int index = cell_to_index(x, y, n_rows, n_cols);
		STATE old_state = grid[index].state;
		grid[index].state = old_state == ALIVE ? DEAD : ALIVE;
	}
	else grid[cell_to_index(x, y, n_rows, n_cols)].state = state;
}

CELL get_cell(CELL *grid, int x, int y, int n_rows, int n_cols) {
	return grid[cell_to_index(x, y, n_rows, n_cols)];
}

/* Draw grid on the screen */
void draw_grid(SDL_Renderer* renderer, int n_rows, int n_cols)
{
	for (int row = 0; row < n_rows; row++) {
                for (int col = 0; col < n_cols; col++) {
                        int x = col * CELL_SIZE, y = row * CELL_SIZE;

                        SDL_Rect top_line = {x, y, CELL_SIZE, LINE_WIDTH};
                        SDL_Rect bottom_line = {x, y + CELL_SIZE, CELL_SIZE, LINE_WIDTH};
                        SDL_Rect left_line = {x, y, LINE_WIDTH, CELL_SIZE};
                        SDL_Rect right_line = {x + CELL_SIZE, y, LINE_WIDTH, CELL_SIZE};
			
			SDL_SetRenderDrawColor(renderer, cell_lines_color.r, cell_lines_color.g, cell_lines_color.b, cell_lines_color.a);
                        SDL_RenderFillRect(renderer, &top_line);
                        SDL_RenderFillRect(renderer, &bottom_line);
                        SDL_RenderFillRect(renderer, &left_line);
                        SDL_RenderFillRect(renderer, &right_line);

                }
        }
	SDL_RenderPresent(renderer);
}

/* Grid's "constructor" */
void set_grid(CELL *grid, STATE state, int n_rows, int n_cols) {
	for (int row = 0; row < n_rows; row++) {
		for (int col = 0; col < n_cols; col++) {
 			int x = col * CELL_SIZE, y = row * CELL_SIZE;
                        SDL_Rect rect = {x + LINE_WIDTH, y + LINE_WIDTH, CELL_SIZE - (LINE_WIDTH * 2), CELL_SIZE - (LINE_WIDTH * 2)};				
			CELL cell = (CELL) {rect, state};
			set_cell(grid, col, row, cell, n_rows, n_cols);
		}
	}
}

/* Counting living cells neighbours to the selected one using offset values, negative values are contemplated thanks to cell_to_index implementation */
int count_living_neighbors(CELL *grid, int x, int y, int n_rows, int n_cols) {
	int alive = 0;
	for (int yo = -1; yo <= 1; yo++) {
		for (int xo = -1; xo <= 1; xo++) {
			if (xo == 0 && yo == 0) continue;
			if (get_cell(grid, x+xo, y+yo, n_rows, n_cols).state == ALIVE) alive++;
		}
	}
	return alive;
}

/* Color update of the specific cell's color */
void update_cell(SDL_Renderer* renderer, CELL* grid, int x, int y, int n_rows, int n_cols, int update_surface)
{
	CELL cell = get_cell(grid, x, y, n_rows, n_cols); 

	if (cell.state == ALIVE)
		SDL_SetRenderDrawColor(renderer, alive_cell_color.r, alive_cell_color.g, alive_cell_color.b, alive_cell_color.a); 
	if (cell.state == DEAD)
		SDL_SetRenderDrawColor(renderer, dead_cell_color.r, dead_cell_color.g, dead_cell_color.b, dead_cell_color.a);
       	SDL_RenderFillRect(renderer, &cell.rect);	
	if (update_surface) SDL_RenderPresent(renderer);
}

/* Compute grid's new state following game's rules
 * Updates cells only if they changed their state between iterations
 * Updates window surface only when state has changed 
 * Returns 1 if any cell has changed from the old to the new grid 
 * If no cell has changed its state the game has finished */
int compute_new_state(SDL_Renderer* renderer, CELL *old, CELL *new, int n_rows, int n_cols) {
	int state_changed = 0;
	for (int y = 0; y < n_rows; y++) {
		for (int x = 0; x < n_cols; x++) {
			int n_alive = count_living_neighbors(old, x, y, n_rows, n_cols);
			STATE new_state = DEAD;
			STATE old_state = get_cell(old, x, y, n_rows, n_cols).state;  
			if (old_state == ALIVE) {
				if (n_alive == 2 || n_alive == 3)
					new_state = ALIVE;
			} else {
				if (n_alive == 3)
					new_state = ALIVE;
			}
			set_state_of_cell(new,x,y,new_state, n_rows, n_cols);
			if (old_state != new_state)
			{
				state_changed = 1;
				update_cell(renderer, new, x, y, n_rows, n_cols, 0);
			}
		}
	}
	/* Dirty rect rendering is implemented automatically 
	 checking state changes */
	if (state_changed) SDL_RenderPresent(renderer);
	return state_changed;
}

/* Bulk update of the grid's color of cells  */
void update_grid(SDL_Renderer* renderer, CELL* grid, int n_rows, int n_cols)
{
	for (int y = 0; y < n_rows; y++)
	{
		for (int x = 0; x < n_cols; x++)
		{	
			CELL cell = get_cell(grid, x, y, n_rows, n_cols);
			if (cell.state == ALIVE)
				SDL_SetRenderDrawColor(renderer, alive_cell_color.r, alive_cell_color.g, alive_cell_color.b, alive_cell_color.a); 
			if (cell.state == DEAD)
				SDL_SetRenderDrawColor(renderer, dead_cell_color.r, dead_cell_color.g, dead_cell_color.b, dead_cell_color.a); 
			SDL_RenderFillRect(renderer, &cell.rect);
		}
	}
	SDL_RenderPresent(renderer);
}

/* Two grids are used to swap the state into a new grid without changing the state of the old one */
int game_of_life(SDL_Renderer* renderer, CELL* old_grid, CELL* new_grid, int n_rows, int n_cols, int animation_speed)
{
	int wait_time = 1000 / animation_speed;
	compute_new_state(renderer, old_grid, new_grid, n_rows, n_cols);
	SDL_Delay(wait_time);
	int state_has_changed = compute_new_state(renderer, new_grid, old_grid, n_rows, n_cols);
	SDL_Delay(wait_time);
	if (!state_has_changed) SDL_Log("game has finished - press SPACE to clear the board");
	return !state_has_changed;
}

int main ()
{	
	SDL_Log("--- Game Controls ---\n\t- Start: Right-click cells to set as ALIVE, then press ENTER.\n\t- Fast Select: Hold right-click and drag to select multiple cells.\n\t- End/Clear: Press SPACE.\n\t- Pause/Resume: Press P. You can add cells while paused.\n\t- Speed: Press U to increase speed, D to decrease speed.");
	SDL_Window* window = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, INITIAL_WIDTH, INITIAL_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);  
	int n_rows = INITIAL_HEIGHT / CELL_SIZE;
	int n_cols = INITIAL_WIDTH / CELL_SIZE;
	int grid_cells = n_rows * n_cols;	

	CELL* old_grid = malloc(sizeof(CELL) * grid_cells);
	CELL* new_grid = malloc(sizeof(CELL) * grid_cells);

	int simulation_running = 1;
	int game_started = 0;
	int motion_lock = 1;	
	int animation_speed = 60;  // default to 60 iterations per second

	draw_grid(renderer, n_rows, n_cols);
	set_grid(old_grid, DEAD, n_rows, n_cols);
	set_grid(new_grid, DEAD, n_rows, n_cols);
	update_grid(renderer, old_grid, n_rows, n_cols);
	
	while (simulation_running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event) != 0)
		{
			switch(event.type)
			{

				case SDL_QUIT:
					{
						simulation_running = 0;
						break;
					}

				case SDL_MOUSEBUTTONDOWN:
					{
						if (!game_started)
						{
							int x = event.button.x / CELL_SIZE;
							int y = event.button.y / CELL_SIZE;
							set_state_of_cell(old_grid, x, y, NO_STATE, n_rows, n_cols);
							update_cell(renderer, old_grid, x, y, n_rows, n_cols, 1);
							motion_lock = 0;
						}
						break;
					}

				case SDL_MOUSEMOTION:
					{
						if (!game_started && !motion_lock)
						{
						  	int x = event.motion.x / CELL_SIZE;
                                                        int y = event.motion.y / CELL_SIZE;
                                                        set_state_of_cell(old_grid, x, y, ALIVE, n_rows, n_cols);
							update_cell(renderer, old_grid, x, y, n_rows, n_cols, 1);	
						}
						break;
					}

				case SDL_MOUSEBUTTONUP:
					{
						motion_lock = 1;
						break;
					}

				case SDL_KEYDOWN:
					{
						// RETURN key is used to start the game
						if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
						{
							game_started = 1;
							SDL_Log("Game started - press 'P' to pause it");
						}
						// SPACE key is used to end the game and clear the grid
						if (event.key.keysym.sym == SDLK_SPACE)
						{
							game_started = 0;
							set_grid(old_grid, DEAD, n_rows, n_cols);
							set_grid(new_grid, DEAD, n_rows, n_cols);
							update_grid(renderer, old_grid, n_rows, n_cols);
						}
						// U key to increase the animation_speed
						if (event.key.keysym.sym == SDLK_u)
						{
							if ((animation_speed + SPEED_UNIT) > MAX_SPEED_VALUE) break;
							animation_speed += SPEED_UNIT;
							SDL_Log("animation_speed: %d iterations per second", animation_speed);
						}	
						// D key to decrease the animation_speed
						if (event.key.keysym.sym == SDLK_d)
						{
							if ((animation_speed - SPEED_UNIT) < MIN_SPEED_VALUE) break;
							animation_speed -= SPEED_UNIT;
							SDL_Log("animation_speed: %d iterations per second", animation_speed);
						}
						// P key to pause the game
						if (event.key.keysym.sym == SDLK_p)
						{
							game_started = 0;
							SDL_Log("Game paused - press ENTER to restart it");
						}
						break;
					}
			}
		}

		

		if (game_started)
		{
			game_started = !game_of_life(renderer, old_grid, new_grid, n_rows, n_cols, animation_speed);
		}
		
	}
	
	free(old_grid);
	free(new_grid);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

}
