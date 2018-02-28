class ship_class{
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
        mvprintw(0,0,"ERROR SETTING SHIP");
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

  int depic_x[5];
  int depic_y[5];
  int last_depic_x[5];
  int last_depic_y[5];

  void set_ship(){
    int i;
    get_dx_dy();
    depic_x[0] = x_pos+dx;
    depic_y[0] = y_pos+dy;
    depic_x[1] = x_pos;
    depic_y[1] = y_pos;
    depic_x[2] = x_pos-dx;
    depic_y[2] = y_pos-dy;
    depic_x[3] = x_pos-dy;
    depic_y[3] = y_pos+dx;
    depic_x[4] = x_pos+dy;
    depic_y[4] = y_pos-dx;

    for(i = 0; i < 5; i++){
      if(depic_x[i] > sc_x_max-2){depic_x[i] = 1;}
      if(depic_x[i] < 1){depic_x[i] = sc_x_max-2;}
      if(depic_y[i] > sc_y_max-2){depic_y[i] = 1;}
      if(depic_y[i] < 1){depic_y[i] = sc_y_max-2;}
    }
  }

  void change_vel(int forward){
    int max_ship_vel = 3;
    get_dx_dy();
    if(!forward){
      vx -= dx;
      vy -= dy;   
    } else {
      vx += dx;
      vy += dy;
    }

    if(vx > max_ship_vel){vx = max_ship_vel;}
    if(vx < -max_ship_vel){vx = -max_ship_vel;}
    if(vy > max_ship_vel){vy = max_ship_vel;}
    if(vy < -max_ship_vel){vy = -max_ship_vel;}
  }

  void move_ship(){
    get_dx_dy();

    x_pos += vx;
    y_pos += vy;

    if(x_pos > sc_x_max-2){x_pos = 1;}
    if(x_pos < 1){x_pos = sc_x_max-2;}
    if(y_pos > sc_y_max-2){y_pos = 1;}
    if(y_pos < 1){y_pos = sc_y_max-2;}
  }

  void render_ship(){
    int i;
    set_ship();
    char ship_char = '+';

    if(direction % 2){
      ship_char = 'x';
    }

    mvprintw(last_depic_y[0],last_depic_x[0],"%c",' ');
    mvprintw(depic_y[0],depic_x[0],"%c",'o');
    for(i = 1 ; i < 5; i++){
      mvprintw(last_depic_y[i],last_depic_x[i],"%c",' ');
      mvprintw(depic_y[i],depic_x[i],"%c",ship_char);
      last_depic_x[i] = depic_x[i];
      last_depic_y[i] = depic_y[i];
    }
    last_depic_x[0] = depic_x[0];
    last_depic_y[0] = depic_y[0];
  }

  ship_class(int x_max, int y_max){
    int i;
    sc_x_max = x_max;
    sc_y_max = y_max;
    x_pos = 10 + rand() % (sc_x_max-20);
    y_pos = 10 + rand() % (sc_y_max-20);
    vel = 0;
    direction = 0;
    set_ship();
    vx = 0;
    vy = 0;
    for(i = 0; i < 5; i++){
      last_depic_x[i] = depic_x[i];
      last_depic_y[i] = depic_y[i];
    }
  }

  ~ship_class(){

  }
};