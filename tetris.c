/* 2017/11/17
  A non-lesson in global variables by Donatas Pekelis.
*/
#include "tetris.h"
//
typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef unsigned char bool;
enum{false,true};

typedef struct{
  Sint8 x, y;
  Uint8 shape, rotation;
} Piece;

typedef struct{
  Uint8 r, g, b;
} Color;

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 384
#define SQUARE_SIZE 16
#define FRAMERATE 1000/60
#define DEFAULT_FALL_SPEED 1000/2
#define SOFT_DROP_SPEED 1000/10
#define STRAFERATE_1 1000/4
#define STRAFERATE_2 1000/10

// board dimensions
#define BOARD_W 10
#define BOARD_W_PX BOARD_W*SQUARE_SIZE + 4
#define BOARD_H 20
#define BOARD_H_PX BOARD_H*SQUARE_SIZE + 4
#define BOARD_X SCREEN_WIDTH/2 - BOARD_W/2 * SQUARE_SIZE - 2
#define BOARD_Y SCREEN_HEIGHT/2 - BOARD_H/2 * SQUARE_SIZE - 2

// queue dimensions
#define QUEUE_X BOARD_X + BOARD_W_PX + SQUARE_SIZE
#define QUEUE_Y BOARD_Y
#define QUEUE_W SQUARE_SIZE*4
#define QUEUE_H SQUARE_SIZE*20

// holding space dimensions
#define HOLD_X BOARD_X - SQUARE_SIZE*5
#define HOLD_Y BOARD_Y + SQUARE_SIZE*2
#define HOLD_W SQUARE_SIZE*4
#define HOLD_H HOLD_W

void logSDLError(FILE *fp, char *msg){
  fprintf(fp,"[ERROR] %s: %s\n",msg,SDL_GetError());
}

// used as the background
void fill_screen_with_tiles(SDL_Texture *t, SDL_Renderer *r){
  int t_width, t_height, x, y;
  SDL_QueryTexture(t,NULL,NULL,&t_width,&t_height);
  for(y=0;y<SCREEN_HEIGHT;y+=t_height){
    for(x=0;x<SCREEN_WIDTH;x+=t_width){
      render_whole_texture(t,r,x,y);
    }
  }
}

Piece generate_piece(){
  Piece p;
  p.rotation = 0;
  p.shape = rand() % 7;
  p.y = 0;
  return p;
}

bool collision(Piece p, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4]){
  int i,j;
  for(j=0;j<4;j++){
    for(i=0;i<4;i++){
      if(p_db[p.shape][p.rotation] & (0x8000 >> (i + 4*j))){
        Sint8 x = p.x + i, y = p.y + j;
        if(x < 0 || x >= BOARD_W || y >= BOARD_H) return true;
        if(y >= 0 && board[x][y] > 0) return true;
      }
    }
  }
  return false;
}

void shift_queue(Piece queue[5]){
  int i;
  for(i=0;i<4;i++){
    queue[i] = queue[i+1];
  }
  queue[4] = generate_piece();
}

void lock_piece(Piece *p, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4],
Piece queue[5], bool *game_running, bool *already_held_once){
  // inscribe old piece into game board
  int i,j;
  for(j=0;j<4;j++){
    for(i=0;i<4;i++){
      if(p_db[p->shape][p->rotation] & (0x8000 >> (i + j*4))){
        board[p->x+i][p->y+j] = p->shape+1;
      }
    }
  }
  // check for complete rows
  int j2, k; j=0;
  while(j<BOARD_H){
    j2 = j;
    for(i=0;i<BOARD_W;i++){
      if(board[i][j] == 0){j++;break;}
    }
    // clear row
    if(j2 == j){
      for(i=j;i>0;i--){
        for(k=0;k<BOARD_W;k++){
          board[k][i] = board[k][i-1];
        }
      }
      for(i=0;i<BOARD_W;i++){
        board[i][0] = 0;
      }
      j++;
    }
  }
  // replace current (dynamic) piece
  *already_held_once = false;
  *p = queue[0];
  p->x = 3; p->y = -1;
  shift_queue(queue);
  // check for game over
  if(collision(*p,board,p_db)){
    *game_running = false;
  }
}

void move_piece_down(Piece *p, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4],
Piece queue[5], bool *game_running, bool *already_held_once){
  Piece dp = *p;
  dp.y++;
  if(collision(dp,board,p_db)){
    lock_piece(p,board,p_db,queue,game_running,already_held_once);
  }else{
    *p = dp;
  }
}

void move_piece_left(Piece *p, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4]){
  Piece dp = *p;
  dp.x--;
  if(!collision(dp,board,p_db)){
    Piece dp2 = dp;
    dp2.y++;
    if(!collision(dp2,board,p_db)){
      *p = dp;
    }
  }
}

void move_piece_right(Piece *p, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4]){
  Piece dp = *p;
  dp.x++;
  if(!collision(dp,board,p_db)){
    Piece dp2 = dp;
    dp2.y++;
    if(!collision(dp2,board,p_db)){
      *p = dp;
    }
  }
}

bool nudge_piece(Piece *p, Uint8 board[BOARD_W][BOARD_H],
Uint16 p_db[7][4]){
  p->x++;
  if(!collision(*p,board,p_db)){return true;}
  p->x -= 2;
  if(!collision(*p,board,p_db)){return true;}
  p->x++;
  p->y--;
  if(!collision(*p,board,p_db)){return true;}
  p->y++;
  return false;
}

void rotate_piece_left(Piece *p, Uint8 board[BOARD_W][BOARD_H],
Uint16 p_db[7][4]){
  Piece dp = *p;
  if(dp.rotation == 0){dp.rotation = 3;}else{dp.rotation--;}
  if(!collision(dp,board,p_db) || nudge_piece(&dp,board,p_db)){
    *p = dp;
  }
}

void hard_drop(Piece *p, Uint8 board[BOARD_W][BOARD_H],
Uint16 p_db[7][4], Piece queue[5], bool *game_running,
bool *already_held_once){
  Piece dp = *p;
  do{
    dp.y++;
  }while(!collision(dp,board,p_db));
  dp.y--;
  *p = dp;
  lock_piece(p,board,p_db,queue,game_running,already_held_once);
}

void update(Piece *curr, Uint8 board[BOARD_W][BOARD_H], Uint16 p_db[7][4],
Piece queue[5], Uint32 *prev_fall, bool *game_running,
Uint16 *fall_speed, Sint8 *strafe_direction, Uint32 *strafe_timer,
Piece *held, bool *is_holding, bool *already_held_once, Uint32 curr_time,
Uint32 *prev_strafe){
  if(key.left && !prev_key.left){
    move_piece_left(curr,board,p_db);
    *strafe_direction = -1;
    *prev_strafe = curr_time;
    *strafe_timer = STRAFERATE_1;
  }else if(key.right && !prev_key.right){
    move_piece_right(curr,board,p_db);
    *strafe_direction = 1;
    *prev_strafe = curr_time;
    *strafe_timer = STRAFERATE_1;
  }
  // auto-strafing
  if(key.right - key.left == *strafe_direction &&
  curr_time > *prev_strafe + *strafe_timer){
    *prev_strafe = curr_time;
    *strafe_timer = STRAFERATE_2;
    if(key.right){move_piece_right(curr,board,p_db);}
    else if(key.left){move_piece_left(curr,board,p_db);}
  }
  if(key.up && !prev_key.up){
    rotate_piece_left(curr,board,p_db);
  }
  // soft drop
  if(key.down && !prev_key.down){
    *fall_speed = SOFT_DROP_SPEED;
  }else if(!key.down && prev_key.down){
    *fall_speed = DEFAULT_FALL_SPEED;
  }
  // hard drop
  if(key.space && !prev_key.space){
    hard_drop(curr,board,p_db,queue,game_running,already_held_once);
  }
  // hold a piece
  if(key.lshift && !prev_key.lshift || key.rshift && !prev_key.rshift){
    if(!(*already_held_once)){
      *already_held_once = true;
      if(!(*is_holding)){
        *is_holding = true;
        *held = *curr;
        queue[0].x = 3;
        queue[0].y = 0;
        *curr = queue[0];
        shift_queue(queue);
      }else{
        Piece p = *curr;
        *curr = *held;
        curr->x = 3;
        curr->y = 0;
        p.y = 0;
        *held = p;
      }
    }
  }
  if(curr_time > *prev_fall + *fall_speed){
    *prev_fall = curr_time;
    move_piece_down(curr,board,p_db,queue,game_running,already_held_once);
  }
}

void draw_piece(Piece p, Uint32 x_px, Uint32 y_px, SDL_Texture *t, SDL_Renderer *r,
Uint16 p_db[7][4], Color colors[7]){
  int i,j;
  for(j=0;j<4;j++){
    for(i=0;i<4;i++){
      if(p_db[p.shape][p.rotation] & (0x8000 >> (i + 4*j)) && p.y+j >= 0){
        Color c = colors[p.shape];
        SDL_SetTextureColorMod(t,c.r,c.g,c.b);
        render_whole_texture(t,r,x_px+i*SQUARE_SIZE,y_px+j*SQUARE_SIZE);
        SDL_SetTextureColorMod(t,0xFF,0xFF,0xFF);
      }
    }
  }
}

void draw(SDL_Renderer *r, Uint8 board[BOARD_W][BOARD_H], Piece curr, Piece queue[5],
SDL_Texture *brick, Uint16 p_db[7][4], Piece held, bool is_holding, Color colors[7]
){
  fill_screen_with_tiles(brick,r); // background
  draw_rect(r,BOARD_X,BOARD_Y,BOARD_W_PX,BOARD_H_PX); // game board area
  draw_rect(r,QUEUE_X,QUEUE_Y,QUEUE_W,QUEUE_H); // piece queue area
  draw_rect(r,HOLD_X,HOLD_Y,HOLD_W,HOLD_H); // hold area
  // draw game board data
  int i,j;
  for(j=0;j<BOARD_H;j++){
    for(i=0;i<BOARD_W;i++){
      if(board[i][j] > 0){
        Color c = colors[board[i][j]-1];
        SDL_SetTextureColorMod(brick,c.r,c.g,c.b);
        render_whole_texture(brick,r,BOARD_X+1+i*SQUARE_SIZE,BOARD_Y+1+j*SQUARE_SIZE);
        SDL_SetTextureColorMod(brick,0xFF,0xFF,0xFF);
      }
    }
  }
  // draw queue
  for(i=0;i<5;i++){
    draw_piece(queue[i],QUEUE_X+1,QUEUE_Y+4*SQUARE_SIZE*i+1,brick,r,p_db,
      colors);
  }
  // draw held piece
  if(is_holding){
    draw_piece(held,HOLD_X+1,HOLD_Y+1,brick,r,p_db,colors);
  }
  // draw current piece
  draw_piece(curr,curr.x*SQUARE_SIZE+BOARD_X+1,curr.y*SQUARE_SIZE+BOARD_Y+1,
    brick,r,p_db,colors);
  SDL_RenderPresent(r);
}

int main(int argc, char *argv[]){
  // piece database [shape][rotation]
  Uint16 piece_database[7][4] = {
/*O*/{0b0000011001100000,0b0000011001100000,0b0000011001100000,0b0000011001100000},
/*L*/{0b1100010001000000,0b0010111000000000,0b0100010001100000,0b0000111010000000},
/*I*/{0b0100010001000100,0b0000111100000000,0b0010001000100010,0b0000000011110000},
/*J*/{0b0110010001000000,0b0000111000100000,0b0100010011000000,0b1000111000000000},
/*T*/{0b0100111000000000,0b0100011001000000,0b0000111001000000,0b0100110001000000},
/*Z*/{0b0100110010000000,0b1100011000000000,0b0010011001000000,0b0000110001100000},
/*S*/{0b1000110001000000,0b0110110000000000,0b0100011000100000,0b0000011011000000}
  };

  srand(time(NULL)); // random init

  // game objects
  Uint8 board[BOARD_W][BOARD_H]; // game board (10 blocks wide and 20 high)
  Piece current = {3,-1,rand()%7,0}; // current piece
  Piece queue[5]; // piece queue
  Piece held; // piece being held

  // mechanic variables
  Uint16 fall_speed = DEFAULT_FALL_SPEED;
  Sint8 strafe_direction;
  Uint32 strafe_timer;
  bool is_holding = false, already_held_once = false;;

  // init queue
  int i;
  for(i=0;i<5;i++){queue[i] = generate_piece();}

  // init board
  int j;
  for(j=0;j<BOARD_H;j++){
    for(i=0;i<BOARD_W;i++){
      board[i][j] = 0;
    }
  }

  // used for timing
  Uint32 curr_time;
  Uint32 prev_frame=0, prev_fall=0, prev_strafe=0;
  // logs out any errors
  FILE *log = fopen("LOG","w");

  bool game_running = true, paused = false;

  SDL_Window *win; SDL_Renderer *ren;
  graphics_init(SCREEN_WIDTH,SCREEN_HEIGHT,&ren,&win,log);

  // textures
  SDL_Texture *brick = load_texture("brick.bmp",ren,log);

  // colors
  Color colors[7] = {
    {255,255,0},{255,128,0},{0,255,255},{0,0,255},
    {255,0,255},{255,0,0},{0,255,0}
  };

  while(game_running == true ){
    curr_time = SDL_GetTicks();
    if(input_update() || key.esc){game_running = false;}
    if(key.enter && !prev_key.enter){paused = !paused;}
    if(!paused){
      update(&current,board,piece_database,queue,
        &prev_fall,&game_running,&fall_speed,&strafe_direction,
        &strafe_timer,&held,&is_holding,&already_held_once,
        curr_time,&prev_strafe
      );
    }
    if(curr_time > prev_frame + FRAMERATE){
      prev_frame = curr_time;
      if(!paused){
        draw(ren,board,current,queue,brick,piece_database,held,
          is_holding,colors);
      }
    }
  }
  SDL_DestroyTexture(brick);
  graphics_quit(ren,win);
  fclose(log);
  return 0;
}
int WinMain(int argc, char *argv[]){
  // piece database [shape][rotation]
  Uint16 piece_database[7][4] = {
/*O*/{0b0000011001100000,0b0000011001100000,0b0000011001100000,0b0000011001100000},
/*L*/{0b1100010001000000,0b0010111000000000,0b0100010001100000,0b0000111010000000},
/*I*/{0b0100010001000100,0b0000111100000000,0b0010001000100010,0b0000000011110000},
/*J*/{0b0110010001000000,0b0000111000100000,0b0100010011000000,0b1000111000000000},
/*T*/{0b0100111000000000,0b0100011001000000,0b0000111001000000,0b0100110001000000},
/*Z*/{0b0100110010000000,0b1100011000000000,0b0010011001000000,0b0000110001100000},
/*S*/{0b1000110001000000,0b0110110000000000,0b0100011000100000,0b0000011011000000}
  };

  srand(time(NULL)); // random init

  // game objects
  Uint8 board[BOARD_W][BOARD_H]; // game board (10 blocks wide and 20 high)
  Piece current = {3,-1,rand()%7,0}; // current piece
  Piece queue[5]; // piece queue
  Piece held; // piece being held

  // mechanic variables
  Uint16 fall_speed = DEFAULT_FALL_SPEED;
  Sint8 strafe_direction;
  Uint32 strafe_timer;
  bool is_holding = false, already_held_once = false;;

  // init queue
  int i;
  for(i=0;i<5;i++){queue[i] = generate_piece();}

  // init board
  int j;
  for(j=0;j<BOARD_H;j++){
    for(i=0;i<BOARD_W;i++){
      board[i][j] = 0;
    }
  }

  // used for timing
  Uint32 curr_time;
  Uint32 prev_frame=0, prev_fall=0, prev_strafe=0;
  // logs out any errors
  FILE *log = fopen("LOG","w");

  bool game_running = true, paused = false;

  SDL_Window *win; SDL_Renderer *ren;
  graphics_init(SCREEN_WIDTH,SCREEN_HEIGHT,&ren,&win,log);

  // textures
  SDL_Texture *brick = load_texture("brick.bmp",ren,log);

  // colors
  Color colors[7] = {
    {255,255,0},{255,128,0},{0,255,255},{0,0,255},
    {255,0,255},{255,0,0},{0,255,0}
  };

  while(game_running == true ){
    curr_time = SDL_GetTicks();
    if(input_update() || key.esc){game_running = false;}
    if(key.enter && !prev_key.enter){paused = !paused;}
    if(!paused){
      update(&current,board,piece_database,queue,
        &prev_fall,&game_running,&fall_speed,&strafe_direction,
        &strafe_timer,&held,&is_holding,&already_held_once,
        curr_time,&prev_strafe
      );
    }
    if(curr_time > prev_frame + FRAMERATE){
      prev_frame = curr_time;
      if(!paused){
        draw(ren,board,current,queue,brick,piece_database,held,
          is_holding,colors);
      }
    }
  }
  SDL_DestroyTexture(brick);
  graphics_quit(ren,win);
  fclose(log);
  return 0;
}
