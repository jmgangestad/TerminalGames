#include <cstdio>
#include <curses.h>
//#include <unistd.h>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

int key_pressed = 0;
int key = 0;
int direction_x = -1;
int direction_y = 0;
int last_direction_x = direction_x;
int last_direction_y = direction_y;
int frame_delay = 100;
int last_x = -1;
int last_y = -1;
int ball_x = -1;
int ball_y = -1;


void print_screen(vector<int> snake_y, vector<int> snake_x){
  int i,j;

  mvprintw(last_y,last_x,"%c",' ');

  mvprintw(snake_y[0], snake_x[0],"%c",'#');
  mvprintw(ball_y,ball_x,"%c",'O');

  refresh();
}


void init_window(){
  int i;
  srand(time(NULL));
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  //set_escdelay(25);

  for(i = 0; i < 80; i++){
    mvprintw(0,i,"%c",'=');
    mvprintw(23,i,"%c",'=');
  }

  for(i = 1; i < 23; i++){
    mvprintw(i,0,"%c",'|');
    mvprintw(i,79,"%c",'|');
  }

}


void place_ball(vector<int> snake_y, vector<int> snake_x){
  int i, valid_placement = 0;

  while(!valid_placement){

    valid_placement = 1;
    
    ball_x = 1 + rand() % 78;
    ball_y = 1 + rand() % 22;

    for(i = 0; i < snake_x.size(); i++){
      if(ball_x == snake_x[i] && ball_y == snake_y[i]){
          valid_placement = 0;
      }
    }
  }

}


void game_loop(vector<int> snake_y, vector<int> snake_x){
  int i, add = 0, game_over = 0;
  clock_t t;
  clock_t last_t = 0;

  while(1){
    t = clock();
    key_pressed = getch();

    switch(key_pressed){
      case 0405: // Right arrow
      case 066: // 6
        if(last_direction_x != -1){
          direction_x = 1;
          direction_y = 0;
        }
        break;
      case 0402: // Down arrow
      case 062: // 2
        if(last_direction_y != -1){
          direction_x = 0;
          direction_y = 1;
        }
        break;
      case 0403: // Up arrow
      case 070: // 8
        if(last_direction_y != 1){
          direction_x = 0;
          direction_y = -1;
        }
        break;
      case 0404: // Left arrow
      case 064: // 4
        if(last_direction_x != 1){
          direction_x = -1;
          direction_y = 0;
        }
        break;
      case 0141: // a
        add = 1;
        break;
      case 0145: // e
        game_over = 1;
        break;
      case 0160: // p
        mvprintw(11,35,"PAUSED");
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
        mvprintw(11,35,"      ");
		for(i = 0; i < snake_x.size(); i++){
		  mvprintw(snake_y[i],snake_x[i],"%c",'#');
		}
        break;
      default:
        break;
    }


    if(t-last_t >= (clock_t) frame_delay){

      if(add){
        snake_x.push_back(snake_x[snake_x.size()-1]);
        snake_y.push_back(snake_y[snake_y.size()-1]);
        add = 0;
      }

      last_x = snake_x[snake_x.size()-1];
      last_y = snake_y[snake_y.size()-1];

      for(i = snake_x.size()-1; i > 0; i--){
        snake_x[i] = snake_x[i-1];
        snake_y[i] = snake_y[i-1];
      }

      snake_x[0] += direction_x;
      snake_y[0] += direction_y;

      if(snake_x[0] == ball_x && snake_y[0] == ball_y){
        add = 1;
        place_ball(snake_y, snake_x);
      }

      if(snake_x[0] < 1 || snake_x[0] > 78 || snake_y[0] < 1 || snake_y[0] > 22){
        game_over  = 1;
      }

      for(i = 1; i < snake_x.size(); i++){
        if(snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]){
          game_over = 1;
        }
      }

      last_direction_x = direction_x;
      last_direction_y = direction_y;
      last_t = clock();
    }

    if(game_over){
      mvprintw(11,34,"GAME OVER");
      refresh();
      nodelay(stdscr, FALSE);
      getch();
      break;
    }

    print_screen(snake_y, snake_x);
  }

}


void set_frame_delay(){

  time_t t1 = 0, t2 = 0;
  clock_t c1 = 0, c2 = 0;

  t1 = time(NULL);
  c1 = clock();

  while(1){
    t2 = time(NULL);

    if((t2 - t1) >= 3){
      c2 = clock();
      break;
    }
  }

  frame_delay = (c2-c1)/30;
}


int main(int argc, char* argv[]){

  set_frame_delay();

  init_window();

  vector<int> snake_x;
  vector<int> snake_y;

  snake_x.push_back(10 + rand() % 65);
  snake_y.push_back(4 + rand() % 18);

  place_ball(snake_y, snake_x);

  if(snake_x[0] < 40){
    direction_x = 1;
  }

  game_loop(snake_y, snake_x);
  
  endwin();

  printf("%d\n",frame_delay);
  return 0;
}
