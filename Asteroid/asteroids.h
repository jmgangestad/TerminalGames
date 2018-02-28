class asteroid_class{
private:
    void get_dx_dy(){
    switch(direction){
      case 0:
        dx = 1;
        dy = 0;
        break;
      case 1:
        dx = 1;
        dy = -1;
        break;
      case 2:
        dx = 0;
        dy = -1;
        break;
      case 3:
        dx = -1;
        dy = -1;
        break;
      case 4:
        dx = -1;
        dy = 0;
        break;
      case 5:
        dx = -1;
        dy = 1;
        break;
      case 6:
        dx = 0;
        dy = 1;
        break;
      case 7:
        dx = 1;
        dy = 1;
        break;
      default:
        mvprintw(0,0,"ERROR SETTING ASTEROID");
        nodelay(stdscr, FALSE);
        refresh();
        getch();
        nodelay(stdscr, TRUE);
        break;
    }
  }

public:
  int x_pos;
  int y_pos;
  int sc_x_max;
  int sc_y_max;
  int dx;
  int dy;
  int direction;
  int vel;
  int vx;
  int vy;
  int x_size;
  int y_size;
  int active;
  int size;

  int* depic_x;
  int* depic_y;
  int* last_depic_x;
  int* last_depic_y;


  void set_depic(){
    int i;
    get_dx_dy();

    for(i = 0; i < y_size; i++){
      depic_y[i] = y_pos+i;
    }

    for(i = 0; i < x_size; i++){
      depic_x[i] = x_pos+i;
    }

    for(i = 0; i < x_size; i++){
      if(depic_x[i] > sc_x_max-2){depic_x[i] = 1;}
      if(depic_x[i] < 1){depic_x[i] = sc_x_max-2;}
    }

    for(i = 0; i < y_size; i++){
      if(depic_y[i] > sc_y_max-2){depic_y[i] = 1;}
      if(depic_y[i] < 1){depic_y[i] = sc_y_max-2;}
    }
  }

  void change_vel(int forward){
    int max_vel = 3;
    get_dx_dy();
    if(!forward){
      vx -= dx;
      vy -= dy;   
    } else {
      vx += dx;
      vy += dy;
    }

    if(vx > max_vel){vx = max_vel;}
    if(vx < -max_vel){vx = -max_vel;}
    if(vy > max_vel){vy = max_vel;}
    if(vy < -max_vel){vy = -max_vel;}
  }

  void move(){
    get_dx_dy();

    x_pos += vx;
    y_pos += vy;

    if(x_pos > sc_x_max-2){x_pos = 1;}
    if(x_pos < 1){x_pos = sc_x_max-2;}
    if(y_pos > sc_y_max){y_pos = 1;}
    if(y_pos < 1){y_pos = sc_y_max-2;}
  }

  void render(){
    int i, j;
    set_depic();
    char character = '+';

    if(direction % 2){
      character = 'x';
    }

    for(j = 0; j < y_size; j++){
      for(i = 0; i < x_size; i++){
        mvprintw(last_depic_y[j],last_depic_x[i],"%c",' ');
      }
    }

    for(j = 0; j < y_size; j++){
      for(i = 0; i < x_size; i++){
        mvprintw(depic_y[j],depic_x[i],"%c",character);
        last_depic_x[i] = depic_x[i];
      }
      last_depic_y[j] = depic_y[j];
    }
  }

  void reset_size(){
    free(depic_x);
    free(depic_y);
    free(last_depic_x);
    free(last_depic_y);
    depic_x = (int*) malloc(x_size*sizeof(int*));
    depic_y = (int*) malloc(y_size*sizeof(int*));
    last_depic_x = (int*) malloc(x_size*sizeof(int*));
    last_depic_y = (int*) malloc(y_size*sizeof(int*));
  }

  void clear(){
    int i,j;
    for(j = 0; j < y_size; j++){
      for(i = 0; i < x_size; i++){
        mvprintw(last_depic_y[j],last_depic_x[i],"%c",' ');
      }
    }
  }

  int in_asteroid(int x, int y){
    int i,j;

    for(i = 0; i < x_size; i++){
      for(j = 0; j < y_size; j++){
        if(x_pos+i == x && y_pos+j == y){
          return 1;
        }
      }
    }

    return 0;
  }

  void reset_last_depic(){
    int i;
    for(i = 0; i < x_size; i++){
      last_depic_x[i] = depic_x[i];
    }

    for(i = 0; i < y_size; i++){
      last_depic_y[i] = depic_y[i];
    }
  }

  asteroid_class(int x_max,int y_max,int sizey, int sizex, int vy_init, int vx_init){
    int i;
    x_size = sizex;
    y_size = sizey;
    vx = vx_init;
    vy = vy_init;
    sc_x_max = x_max;
    sc_y_max = y_max;
    active = 1;

    depic_x = (int*) malloc(x_size*sizeof(int*));
    depic_y = (int*) malloc(y_size*sizeof(int*));
    last_depic_x = (int*) malloc(x_size*sizeof(int*));
    last_depic_y = (int*) malloc(y_size*sizeof(int*));

    x_pos = 2 + rand() % (sc_x_max-5);
    y_pos = 2 + rand() % (sc_y_max-5);
    direction = 0;
    set_depic();

    for(i = 0; i < x_size; i++){
      last_depic_x[i] = depic_x[i];
    }

    for(i = 0; i < y_size; i++){
      last_depic_y[i] = depic_y[i];
    }
  }

  ~asteroid_class(){
    free(depic_x);
    free(depic_y);
    free(last_depic_x);
    free(last_depic_y);
  }
};