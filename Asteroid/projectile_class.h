class projectile_class{
public:
  int x_pos;
  int y_pos;
  int sc_x_max;
  int sc_y_max;
  int dx;
  int dy;
  int vx;
  int vy;
  int active;

  int depic_x;
  int depic_y;
  int last_depic_x;
  int last_depic_y;

  void set_depic(){
    depic_x = x_pos;
    depic_y = y_pos;
  }


  void move(){
    x_pos += vx;
    y_pos += vy;

    if(x_pos > sc_x_max-2 || x_pos < 1 || y_pos > sc_y_max-2 || y_pos < 1){
      active = 0;
      mvprintw(last_depic_y,last_depic_x,"%c",' ');
    }
  }

  void render(){
    int i;
    set_depic();
    char character = 'o';

    mvprintw(last_depic_y,last_depic_x,"%c",' ');
    mvprintw(depic_y,depic_x,"%c",character);
    last_depic_x = depic_x;
    last_depic_y = depic_y;
  }

  void clear(){
    mvprintw(last_depic_y,last_depic_x,"%c",' ');
  }

  void activate(int x, int y, int x_vel, int y_vel){
    x_pos = x;
    y_pos = y;
    depic_x = x;
    depic_y = y;
    last_depic_x = x;
    last_depic_y = y;
    vx = x_vel;
    vy = y_vel;

    if(vx > 0){
      dx = 1;
    } else if(vx < 0){
      dx = -1;
    } else {
      dx = 0;
    }

    if(vy > 0){
      dy = 1;
    } else if(vy < 0){
      dy = -1;
    } else {
      dy = 0;
    }

    active = 1;
  }

  projectile_class(int x_max, int y_max, int x_pos_init, int y_pos_init){
    int i;
    sc_x_max = x_max;
    sc_y_max = y_max;
    x_pos = x_pos_init;
    y_pos = y_pos_init;
    set_depic();
    vx = 0;
    vy = 0;
    last_depic_x = depic_x;
    last_depic_y = depic_y;
    active = 0;
  }

  ~projectile_class(){

  }
};