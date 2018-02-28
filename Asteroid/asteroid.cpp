#include <cstdio>
#include <ncurses.h>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "ship_class.h"
#include "asteroids.h"
#include "projectile_class.h"
#include <sys/ioctl.h>

using namespace std;

int key_pressed = 0;
int frame_delay = 0;

int large_asteroids = 2;
int med_asteroids = 2;
int small_asteroids = 2;
int num_asteroids = 3*large_asteroids+2*med_asteroids+small_asteroids;
int max_projectiles = 15;
int Y_MAX, X_MAX;
int invincible = 0;
int game_over = 0;





ship_class* ship;
asteroid_class** asteroid;
projectile_class** projectile;

void init_window(){
  int i;
  srand(time(NULL));

  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);

  X_MAX = w.ws_col;
  Y_MAX = w.ws_row;

  initscr();
  //raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  set_escdelay(25);

  for(i = 0; i < X_MAX; i++){
    mvprintw(0,i,"%c",'=');
    mvprintw(Y_MAX-1,i,"%c",'=');
  }

  for(i = 1; i < Y_MAX-1; i++){
    mvprintw(i,0,"%c",'|');
    mvprintw(i,X_MAX-1,"%c",'|');
  }
}

void print_screen(){
  int i;

  for(i = 0; i < max_projectiles; i++){
    if(projectile[i]->active){
      projectile[i]->render();
    }
  }
  for(i = 0; i < num_asteroids; i++){
    if(asteroid[i]->active == 1){
      asteroid[i]->render();
    }
  }
	ship->render_ship();
  refresh();
}



void check_collisions(){
  int i, j, k, x = 0, y = 0;
  int i2, j2;
  int x2,y2, dx, dy, x3, y3;
  int collided = 0;
  int VX[3], VY[3];

  for(i = 0; i < num_asteroids; i++){
    for(j = 0; j < max_projectiles; j++){
      if(asteroid[i]->active == 1 && projectile[j]->active == 1){
        x = (projectile[j]->x_pos+projectile[j]->last_depic_x)/2;
        y = (projectile[j]->y_pos+projectile[j]->last_depic_y)/2;
        x2 = (projectile[j]->x_pos-projectile[j]->last_depic_x)/4;
        y2 = (projectile[j]->y_pos-projectile[j]->last_depic_y)/4;

          if(asteroid[i]->in_asteroid(projectile[j]->x_pos,projectile[j]->y_pos)){
              collided = 1;
          } else if(asteroid[i]->in_asteroid(projectile[j]->last_depic_x,projectile[j]->last_depic_y)){
              collided = 1;
          } else if(asteroid[i]->in_asteroid(x,y)){
              collided = 1;
          } else if(asteroid[i]->in_asteroid(projectile[j]->x_pos+x2,projectile[j]->y_pos+y2)){
              collided = 1;
          } else if(asteroid[i]->in_asteroid(projectile[j]->x_pos+x2*3,projectile[j]->y_pos+y2*3)){
              collided = 1;
          } else {
            collided = 0;
          }

          if(collided){

              asteroid[i]->active = 0;
              projectile[j]->active = 0;
              dx = projectile[j]->dx;
              dy = projectile[j]->dy;
              projectile[j]->clear();
              asteroid[i]->clear();
              x = asteroid[i]->x_pos;
              y = asteroid[i]->y_pos;

              if(asteroid[i]->size == 2){
                while(1){
                  VX[0] = -2+rand() % 5;
                  VX[1] = -2+rand() % 5;
                  VX[2] = -2+rand() % 5;
                  if(VX[0] != VX[1] && VX[1] != VX[2] && VX[2] != VX[0]){break;}
                }

                while(1){
                  VY[0] = -2+rand() % 5;
                  VY[1] = -2+rand() % 5;
                  VY[2] = -2+rand() % 5;
                  if(VY[0] != VY[1] && VY[1] != VY[2] && VY[2] != VY[0]){break;}
                }
                for(i2 = 0; i2 < 3; i2++){
                  for(j2 = 0; j2 < num_asteroids; j2++){
                    if(asteroid[j2]->active == 0){
                      asteroid[j2]->size = 0;
                      asteroid[j2]->x_pos = x;
                      asteroid[j2]->y_pos = y;
                      asteroid[j2]->x_size = 2;
                      asteroid[j2]->y_size = 2;
                      asteroid[j2]->reset_size();
                      asteroid[j2]->vx = VX[i2];
                      asteroid[j2]->vy = VY[i2];
                      asteroid[j2]->active = 1;
                      asteroid[j2]->set_depic();
                      asteroid[j2]->reset_last_depic();
                      break;
                    }
                  }
                }
              } else if(asteroid[i]->size == 1){
                for(i2 = 0; i2 < 2; i2++){
                  for(j2 = 0; j2 < num_asteroids; j2++){
                    if(asteroid[j2]->active == 0){
                      asteroid[j2]->size = 0;
                      asteroid[j2]->x_pos = x;
                      asteroid[j2]->y_pos = y;
                      asteroid[j2]->x_size = 2;
                      asteroid[j2]->y_size = 2;
                      asteroid[j2]->reset_size();
                      asteroid[j2]->vx = -2+rand() % 5;
                      asteroid[j2]->vy = -2+rand() % 5;
                      asteroid[j2]->active = 1;
                      asteroid[j2]->set_depic();
                      asteroid[j2]->reset_last_depic();
                      break;
                    }
                  }
                }
              }

              break;
          }
      }
    }
  }


  if(!invincible){
    for(i = 0; i < num_asteroids; i++){
      if(asteroid[i]->active){
        for(i2 = -1; i2 < 2; i2++){
          for(j2 = -1; j2 < 2; j2++){
            x = ship->x_pos + i2;
            y = ship->y_pos + j2;
            x3 = (x+ship->last_depic_x[1]+i2)/2;
            y3 = (y+ship->last_depic_y[1]+j2)/2;
            x2 = (x-ship->last_depic_x[1]+i2)/4;
            y2 = (y-ship->last_depic_y[1]+j2)/4;

            if(asteroid[i]->in_asteroid(x,y)){
                collided = 1;
            } else if(asteroid[i]->in_asteroid(ship->last_depic_x[1]+i2,ship->last_depic_y[1]+j2)){
                collided = 1;
            } else if(asteroid[i]->in_asteroid(x3,y3)){
                collided = 1;
            } else if(asteroid[i]->in_asteroid(x+x2,y+y2)){
                collided = 1;
            } else if(asteroid[i]->in_asteroid(x+x2*3,y+y2*3)){
                collided = 1;
            } else {
              collided = 0;
            }

            if(collided){
              game_over = 1;
              break;
            }
          }
        }
      }
    }
  }






}

void game_loop(){
  int i;
  clock_t t;
  clock_t t_last = 0;
  int you_win = 0;

  while(1){
    t = clock();
    key_pressed = getch();

    switch(key_pressed){
      case 0405: // Right arrow
      case 066: // 6
        ship->direction--;
        if(ship->direction < 0){
        	ship->direction = 7;
        }
        break;
      case 0402: // Down arrow
      	ship->change_vel(0);
      	break;
      case 062: // 2
        break;
      case 0403: // Up arrow
      case 070: // 8
        ship->change_vel(1);
        break;
      case 0404: // Left arrow
      case 064: // 4
      	ship->direction++;
        if(ship->direction > 7){
        	ship->direction = 0;
        }
        break;
      case 0141: // (a)
        for(i = 0; i < max_projectiles; i++){
          if(projectile[i]->active == 0){      
            projectile[i]->activate(ship->x_pos+2*ship->dx,ship->y_pos+2*ship->dy,ship->vx+4*ship->dx,ship->vy+4*ship->dy);
            break;
          }
        }
        
        break;
      case 0145: // e
        game_over = 1;
        break;
      case 0160: // p
        mvprintw(Y_MAX/2,X_MAX/2 - 4,"PAUSED");
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
        mvprintw(Y_MAX/2,X_MAX/2 - 4,"      ");
		break;
      default:
        break;
    }


    if(t-t_last >= (clock_t) frame_delay){

      for(i = 0; i < num_asteroids; i++){
        if(asteroid[i]->active == 1){
          asteroid[i]->move();
        }
      }
      for(i = 0; i < max_projectiles; i++){
        if(projectile[i]->active){
          projectile[i]->move();
        }
      }
      check_collisions();
      ship->move_ship();
      t_last = clock();
    }

    if(game_over){
      mvprintw(Y_MAX/2,X_MAX/2 - 5,"GAME OVER");
      refresh();
      nodelay(stdscr, FALSE);
      getch();
      break;
    }

    you_win = 1;

    for(i = 0; i < num_asteroids; i++){
      if(asteroid[i]->active){
        you_win = 0;
        break;
      }
    }

    if(you_win){
      mvprintw(Y_MAX/2,X_MAX/2 - 4,"YOU WIN");
      refresh();
      nodelay(stdscr, FALSE);
      getch();
      break;
    }

    print_screen();
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
  int i;
  init_window();

	ship = new ship_class(X_MAX,Y_MAX);


  asteroid = (asteroid_class**) malloc(num_asteroids*sizeof(asteroid_class**));
  projectile = (projectile_class**) malloc(max_projectiles*sizeof(projectile_class**));

  for(i = 0; i < max_projectiles; i++){
    projectile[i] = new projectile_class(X_MAX,Y_MAX,0,0);
  }

  for(i = 0; i < large_asteroids; i++){
    asteroid[i] = new asteroid_class(X_MAX,Y_MAX,6, 6,-2 + rand() % 4,-2 + rand() % 4);
    asteroid[i]->size = 2;
  }

  for(i = 0; i < med_asteroids; i++){
    asteroid[i+large_asteroids] = new asteroid_class(X_MAX,Y_MAX,4, 4,-2 + rand() % 4,-2 + rand() % 4);
    asteroid[i+large_asteroids]->size = 1;
  }

  for(i = 0; i < small_asteroids; i++){
    asteroid[i+large_asteroids+med_asteroids] = new asteroid_class(X_MAX,Y_MAX,2, 2,-2 + rand() % 4,-2 + rand() % 4);
    asteroid[i+large_asteroids+med_asteroids]->size = 0;
  }

  for(i = large_asteroids+med_asteroids+small_asteroids; i < num_asteroids; i++){
    asteroid[i] = new asteroid_class(X_MAX,Y_MAX,1, 1,-2 + rand() % 4,-2 + rand() % 4);
    asteroid[i]->active = 0;
  }




	set_frame_delay();


	game_loop();

	delete ship;
  for(i = 0; i < max_projectiles; i++){
    delete projectile[i];
  }
  for(i = 0; i < num_asteroids; i++){
    delete asteroid[i];
  }
  free(projectile);
  free(asteroid);
  //key_pressed = getch();
	endwin();


  //printf("%#o\n",key_pressed);
  printf("%d\n",frame_delay);
  return 0;
}