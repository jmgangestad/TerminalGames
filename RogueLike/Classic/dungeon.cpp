#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <climits>
#include "heap.h"
#include <ncurses.h>
#include <fstream>




#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (weight[pair[dim_y]][pair[dim_x]])

#define DUNGEON_X  80
#define DUNGEON_Y  21



char buffer[21][80];
char tunneling_map[21][80];
char non_tunneling_map[21][80];
char weight[21][80];
unsigned char hardness[21][80];
int num_rooms, up_stair_x, down_stair_x, up_stair_y, down_stair_y;
int num_mon_types = 0;
int num_obj_types = 0;
int num_objects;
int items_allocated;
char room_char = '.';
char path_char = '#';
char rock_char = ' ';
char pc_char   = '@';
char up_stair_char = '<';
char down_stair_char = '>';
char message_top[80] = "Gameplay Messages";
const char *message_1 = "Status information 1";
const char *message_2 = "Status information 2";



/* Structure to store room data */
typedef struct room {
  int x_pos, y_pos, x_size, y_size;
} room_t;


room_t room_arr[1482];


typedef struct corridor_path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} corridor_path_t;


typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

typedef struct monster{
  char race;
  int color;
  unsigned int intel;
  unsigned int tel;
  unsigned int tunneling;
  unsigned int erratic;
  unsigned int turn;
  unsigned int alive;
  int x;
  int y;
  int index;
  int speed;
  int hp;
  int dam[3];
} monster_t;


/* Function declerations */
int buffer_init();
int new_room(int i);
int new_room(int i);
int buffer_update(int x_beg,int x_leng, int y_beg,int y_leng,char c);
int room_check(int i);
int generate_rooms();
int connect_rooms();
int rand_dungeon();
int load_dungeon(char *dungeon_file);
int save_dungeon(char *dungeon_file);
int dijkstra(pair_t from, int tunneling);
int place_pc_rand();
int assign_hardness();
int place_monster_rand(monster_t *m);
int new_monster(monster_t *m);
int print_screen2(struct monster *mon,int num_mon);
int in_same_room(monster_t *c);
int LOS_move(monster_t *c);
int character_turn(monster_t *c,struct monster *mon, int num_mon);
int dijkstra_move(monster_t *c);
int random_move(monster_t *c);
int move_character(monster_t *c, int dx, int dy);
int place_stairs();
int print_monster_window(struct monster *mon, int num_mon, int l_edge, int mon_start);
int monster_list(struct monster *mon,int num_mon);
int regenerate_dungeon(struct monster *mon,int num_mon);
int pc_move(int *dx, int *dy, struct monster *mon,int num_mon);
int parse_monsters();
int display_parsed_monsters();
int deallocate_leftovers();
int roll_dice(int dice[3]);
int parse_objects();
int drop_item_input(struct monster *mon, int num_mon);
int equip_item_input(struct monster *mon, int num_mon);
int expunge_item_input(struct monster *mon, int num_mon);
int item_screen_print(struct monster *mon, int l_edge, int arrow, int num_mon);
static int call_dijkstra(int from_x, int from_y, int tunneling);

static int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}



monster_t pc;



static int32_t monster_turn_cmp(const void *key, const void *with) {
  return ((monster_t *) key)->turn - ((monster_t *) with)->turn;
}



class character_type{
public:
  char NAME[80];
  char *DESC;
  int SMART;
  int TELE;
  int TUNNEL;
  int ERRATIC;
  int PASS;
  char SYMB;
  int COLOR[8];
  int SPEED[3];
  int HP[3];
  int DAM[3];
  character_type(){
    SMART = 0;
    TELE = 0;
    TUNNEL = 0;
    ERRATIC = 0;
    PASS = 0;
    for(int i = 0;i < 8;i++){
      COLOR[i] = 0;
    }
  }
  ~character_type(){
    free(DESC);
  }
};

class object_type{
public:
  char NAME[80];
  char TYPE[11];
  char *DESC;
  int COLOR[8];
  int HIT[3];
  int DAM[3];
  int DODGE[3];
  int DEF[3];
  int WEIGHT[3];
  int SPEED[3];
  int ATTR[3];
  int VAL[3];

  object_type(){

    strcpy(TYPE,"0000000000");
    for(int i = 0;i < 8;i++){
      COLOR[i] = 0;
    }

  }

  ~object_type(){
    free(DESC);
  }

};




object_type **item;


class object_class{
public:
  char* DESC;
  char NAME[80];
  char TYPE[11];
  char SYMB;
  int HIT;
  int DAM[3];
  int DODGE;
  int DEF;
  int WEIGHT;
  int SPEED;
  int ATTR;
  int VAL;
  int x;
  int y;
  int color;
  int status[3]; // 0 for on dungeon floor, 1 for in inventory, 2 for equipped


  void reset_item(){
    strcpy(TYPE,"NONE\0");
    SYMB = ' ';
    HIT = 0;
    DAM[0] = 0;
    DAM[1] = 0;
    DAM[2] = 1;
    DODGE = 0;
    DEF = 0;
    WEIGHT = 0;
    SPEED = 0;
    ATTR = 0;
    VAL = 0;
    x = 0;
    y = 0;
    color = COLOR_WHITE;
  }

  object_class(int j){
    reset_item();
  }

  object_class(){
    int t = rand() % num_obj_types;
    strcpy(TYPE,item[t]->TYPE);
    strcpy(NAME,item[t]->NAME);
    DESC = item[t]->DESC;

    if(!strcmp(TYPE,"ARMOR")){
      SYMB = '[';
    } else if (!strcmp(TYPE,"WEAPON")){
      SYMB = '|';
    } else if (!strcmp(TYPE,"OFFHAND")){
      SYMB = ')';
    } else if (!strcmp(TYPE,"RANGED")){
      SYMB = '}';
    } else if (!strcmp(TYPE,"HELMET")){
      SYMB = ']';
    } else if (!strcmp(TYPE,"CLOAK")){
      SYMB = '(';
    } else if (!strcmp(TYPE,"GLOVES")){
      SYMB = '{';
    } else if (!strcmp(TYPE,"BOOTS")){
      SYMB = '\\';
    } else if (!strcmp(TYPE,"RING")){
      SYMB = '=';
    } else if (!strcmp(TYPE,"AMULET")){
      SYMB = '"';
    } else if (!strcmp(TYPE,"LIGHT")){
      SYMB = '_';
    } else if (!strcmp(TYPE,"SCROLL")){
      SYMB = '~';
    } else if (!strcmp(TYPE,"BOOK")){
      SYMB = '?';
    } else if (!strcmp(TYPE,"FLASK")){
      SYMB = '!';
    } else if (!strcmp(TYPE,"GOLD")){
      SYMB = '$';
    } else if (!strcmp(TYPE,"AMMUNITION")){
      SYMB = '/';
    } else if (!strcmp(TYPE,"FOOD")){
      SYMB = ',';
    } else if (!strcmp(TYPE,"WAND")){
      SYMB = '-';
    } else if (!strcmp(TYPE,"CONTAINER")){
      SYMB = '%';
    } else {
      SYMB = '*';
    }

    HIT = roll_dice(item[t]->HIT);
    DAM[0] = item[t]->DAM[0];
    DAM[1] = item[t]->DAM[1];
    DAM[2] = item[t]->DAM[2];
    DODGE = roll_dice(item[t]->DODGE);
    DEF = roll_dice(item[t]->DEF);
    WEIGHT = roll_dice(item[t]->WEIGHT);
    SPEED = roll_dice(item[t]->SPEED);;
    ATTR = roll_dice(item[t]->ATTR);
    VAL = roll_dice(item[t]->VAL);
    status[0] = 1;
    status[1] = 0;
    status[2] = 0;
  
    while(1){
      int br = 0;
      x = rand() % 80;
      y = rand() % 21;

      for(int i = 0; i < num_rooms; i++){
        if(x < room_arr[i].x_pos+room_arr[i].x_size && x >= room_arr[i].x_pos){
          if(y < room_arr[i].y_pos+room_arr[i].y_size && y >= room_arr[i].y_pos){
            br = 1;
          }
        }
      }
      if(br == 1){
        break;
      }
    }
    if(item[t]->COLOR[COLOR_WHITE] == 1){
      color = COLOR_WHITE;
    } else if (item[t]->COLOR[COLOR_RED] == 1){
      color = COLOR_RED;
    } else if (item[t]->COLOR[COLOR_BLUE] == 1){
      color = COLOR_BLUE;
    } else if (item[t]->COLOR[COLOR_GREEN] == 1){
      color = COLOR_GREEN;
    } else if (item[t]->COLOR[COLOR_MAGENTA] == 1){
      color = COLOR_MAGENTA;
    } else if (item[t]->COLOR[COLOR_YELLOW] == 1){
      color = COLOR_YELLOW;
    } else if (item[t]->COLOR[COLOR_CYAN] == 1){
      color = COLOR_CYAN;
    } else if (item[t]->COLOR[COLOR_BLACK] == 1){
      color = COLOR_BLACK;
    } else {
      color = COLOR_WHITE;
    }
  }

  ~object_class(){

  }

};



character_type **c;


object_class **object;


int roll_dice(int dice[3]){
  int i;
  int sum = 0;

  for (i = 0; i < dice[1]; i++){
    sum += 1 + rand() % dice[2];
  }

  return dice[0] + sum;
}


struct equipment_slots{
  object_class *equipment[12];
  object_class *inventory[10];

};

equipment_slots pc_slot;


int swap_item_stats(object_class *from,object_class *to){

          to->DESC = from->DESC;
          strcpy(to->TYPE, from->TYPE);
          strcpy(to->NAME, from->NAME);
          to->SYMB = from->SYMB;
          to->HIT = from->HIT;
          to->DAM[0] = from->DAM[0];
          to->DAM[1] = from->DAM[1];
          to->DAM[2] = from->DAM[2];
          to->DODGE = from->DODGE;
          to->DEF = from->DEF;
          to->WEIGHT = from->WEIGHT;
          to->SPEED = from->SPEED;
          to->ATTR = from->ATTR;
          to->VAL = from->VAL;
          to->x = from->x;
          to->y = from->y;
          to->color = from->color;
          to->status[0] = from->status[0];
          to->status[1] = from->status[1];
          to->status[2] = from->status[2];


          return 0;
}


int pickup_item(){
  int i, j;

  for(i = 0; i < 10; i++){
    if(pc_slot.inventory[i]->status[1] == 0){
      
      for(j = 0; j < items_allocated; j++){
        if(pc.x == object[j]->x && pc.y == object[j]->y && object[j]->status[0] == 1){

          swap_item_stats(object[j],pc_slot.inventory[i]);

          object[j]->status[0] = 0;
          pc_slot.inventory[i]->status[1] = 1;

          break;
        }
      }


    }
  }

  return 0;
}






int equipped_screen_print(struct monster *mon, int l_edge, int arrow, int num_mon,int which){
  int i,j;
  const char side_char = '|';
  const char top_bot_char = '=';
  int t_edge = 2;
  int b_edge = 20;
  int r_edge = l_edge+48;


  print_screen2(mon,num_mon);

  for(j = t_edge+1;j < b_edge; j++){
    for(i = l_edge+1; i < r_edge; i++){
      mvprintw(j,i," ");
    }
  }

  for(j = t_edge+1;j < b_edge; j++){
      mvprintw(j,l_edge,"%c",side_char);
      mvprintw(j,r_edge,"%c",side_char);
  }


  for(i = l_edge+1; i < r_edge; i++){
    mvprintw(t_edge,i,"%c",top_bot_char);
    mvprintw(b_edge,i,"%c",top_bot_char);
    mvprintw(t_edge+2,i,"-");
  }

  mvprintw(t_edge,l_edge,"+");
  mvprintw(t_edge,r_edge,"+");
  mvprintw(b_edge,l_edge,"+");
  mvprintw(b_edge,r_edge,"+");


  if(which == 0){
    mvprintw(t_edge+1,l_edge+1,"Select item to take off: ");
  } else if (which == 1){
    mvprintw(t_edge+1,l_edge+1,"Equipment: ");
  }

int base_start = 4;

if(arrow == -1){
  base_start = 1;
}


  for(i = 0; i < 12; i++){
    if(pc_slot.equipment[i]->status[2]){
      attron(COLOR_PAIR(pc_slot.equipment[i]->color));
      mvprintw(i+t_edge+3,l_edge+base_start+3,"%c",pc_slot.equipment[i]->SYMB);
      attroff(COLOR_PAIR(pc_slot.equipment[i]->color));
      mvprintw(i+t_edge+3,l_edge+base_start+6,"%s",pc_slot.equipment[i]->NAME);
    }

    mvprintw(i+t_edge+3,l_edge+base_start,"%c",'a'+i);
    mvprintw(i+t_edge+3,l_edge+base_start+1,")[");
    mvprintw(i+t_edge+3,l_edge+base_start+4,"]");
  }

  mvprintw(22,0,"Arrows to move                         ");
  mvprintw(23,0,"Escape key to exit                ");

  if(arrow != -1){
    mvprintw(t_edge+3+arrow,l_edge+1,"->");
    mvprintw(22,0,"Arrows to move, Enter to select   ");
  }



  refresh();

  return 0;
}






int unequip_item_input(struct monster *mon, int num_mon){
  int key_pressed = 0;
  int l_edge = 30;
  int arrow = 0;
  int i;
  int switching;

  equipped_screen_print(mon, l_edge, arrow, num_mon,0);

  while(1){
    switching = 0;

    key_pressed = getch();

    switch(key_pressed){
      case 012: // Enter
        if(pc_slot.equipment[arrow]->status[2] == 1){

            for(i = 0; i < 10; i++){
              if(pc_slot.inventory[i]->status[1] == 0){
                switching = 1;
                break;
              }
            }


            if(switching == 1){

              swap_item_stats(pc_slot.equipment[arrow],pc_slot.inventory[i]);
              //pc_slot.equipment[arrow]->reset_item();

              pc_slot.equipment[arrow]->status[1] = 0;
              pc_slot.equipment[arrow]->status[2] = 0;
              pc_slot.inventory[i]->status[1] = 1;
              pc_slot.inventory[i]->status[2] = 0;
            } else {
              message_1 = "There are no open inventory slots";
            }



          return 0;
        }
        break;
      case 0404: // Left arrow
        if(l_edge > 0){
          l_edge--;
        }
        if(l_edge == 0){
          l_edge = 80-50;
        }
        equipped_screen_print(mon, l_edge, arrow, num_mon,0);
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        equipped_screen_print(mon, l_edge, arrow, num_mon,0);
        break;
      default:
        break;
      case 0402: // Down arrow
          arrow++;
          if(arrow > 11){
            arrow = 0;
          }
          equipped_screen_print(mon, l_edge, arrow, num_mon,0);
        break;
      case 0403: // Up arrow
          arrow--;
          if(arrow < 0){
            arrow = 11;
          }
          equipped_screen_print(mon, l_edge, arrow, num_mon,0);
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}

int attacked = 0;
int damage_dealt;

int pc_attack(){
  int i; 
  int sum = 0;
  //int pc_base_damage = 10;

  for(i = 1; i < 12; i++){
    if(pc_slot.equipment[i]->status[2]){
      sum += roll_dice(pc_slot.equipment[i]->DAM);
    }
  }

  sum += roll_dice(pc.dam)*(1-pc_slot.equipment[0]->status[2]);
  sum += roll_dice(pc_slot.equipment[0]->DAM)*pc_slot.equipment[0]->status[2];

  damage_dealt = sum;

  return sum;
}





int main(int argc, char *argv[])
{
  int i,dx,dy,monsters_left;
  int save = 0;
  int load = 0;
  srand(time(NULL));
  int num_mon = 6 + rand() % 6;
  int quit_game = 0;


  /* Check the arguments passed in */

  for(i = 0; i < argc; i++)
  {
    if(!strcmp(argv[i], "--save"))
    {
      save = 1;
    } else if(!strcmp(argv[i], "--load")){
      load = 1;
    } else if(!strcmp(argv[i], "--nummon")){
      num_mon = atoi(argv[i+1]);
    }
  }



  /* Create a character variable containing the filepath
     to the default save location*/
  char dungeon_src[] = "/.rlg327/dungeon";
  char dungeon_file[100];
  strcpy(dungeon_file,getenv("HOME"));
  strcat(dungeon_file,dungeon_src);

  /*
  char dungeon_file[] = "save_files/hello.rlg327";
  */



  
  /* Run the correct load/save sequence */
  if(load == 1)
  {
    num_rooms = load_dungeon(dungeon_file);
     if(num_rooms == -1){
      return -1;
     }
  } else {
    num_rooms = 6 + rand() % 3;
    rand_dungeon();
  }



  if(save == 1)
  {
    save_dungeon(dungeon_file);
  }
    
  assign_hardness();

  parse_monsters();
  parse_objects();
  //display_parsed_monsters();
  //display_parsed_objects();

  num_objects = 10+rand() % 5;

  object = (object_class**) malloc(num_objects*sizeof(object_class*));

  for(i = 0;i<num_objects;i++){
    object[i] = new object_class;
  }
  items_allocated = num_objects;

  //return 0;


  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_WHITE,COLOR_WHITE,COLOR_BLACK);
  init_pair(COLOR_CYAN,COLOR_CYAN,COLOR_BLACK);
  init_pair(COLOR_MAGENTA,COLOR_MAGENTA,COLOR_BLACK);
  init_pair(COLOR_YELLOW,COLOR_YELLOW,COLOR_BLACK);
  init_pair(COLOR_GREEN,COLOR_GREEN,COLOR_BLACK);
  init_pair(COLOR_BLUE,COLOR_BLUE,COLOR_BLACK);
  init_pair(COLOR_RED,COLOR_RED,COLOR_BLACK);
  init_pair(COLOR_BLACK,COLOR_WHITE,COLOR_BLACK);






  place_pc_rand();
  place_stairs();
  pc.race = '@';
  pc.tunneling = 0;
  pc.turn = 0;
  pc.index = 0;
  pc.speed = 10;
  pc.alive = 1;
  pc.color = COLOR_WHITE;
  pc.hp = 500;
  pc.dam[0] = 0;
  pc.dam[1] = 1;
  pc.dam[2] = 4;


  for(i = 0; i < 12; i++){
    pc_slot.equipment[i] = new object_class(1);
    pc_slot.equipment[i]->status[0] = 0;
    pc_slot.equipment[i]->status[1] = 0;
    pc_slot.equipment[i]->status[2] = 0;
  }

  strcpy(pc_slot.equipment[0]->TYPE,"WEAPON");
  strcpy(pc_slot.equipment[1]->TYPE,"OFFHAND");
  strcpy(pc_slot.equipment[2]->TYPE,"RANGED");
  strcpy(pc_slot.equipment[3]->TYPE,"ARMOR");
  strcpy(pc_slot.equipment[4]->TYPE,"HELMET");
  strcpy(pc_slot.equipment[5]->TYPE,"CLOAK");
  strcpy(pc_slot.equipment[6]->TYPE,"GLOVES");
  strcpy(pc_slot.equipment[7]->TYPE,"BOOTS");
  strcpy(pc_slot.equipment[8]->TYPE,"AMULET");
  strcpy(pc_slot.equipment[9]->TYPE,"LIGHT");
  strcpy(pc_slot.equipment[10]->TYPE,"RING");
  strcpy(pc_slot.equipment[11]->TYPE,"RING");

  for(i = 0; i < 10; i++){
    pc_slot.inventory[i] = new object_class(1);
  }

/*
  mvprintw(23,0,"Enter keystroke");
  int butt = getch();
  //pc_slot.weapon->reset_item();
  clear();
  mvprintw(23,0,"%#o",butt);
  getch();
*/

  monster_t mon[num_mon];

  for(i = 0; i < num_mon; i++){
    new_monster(&mon[i]);
    mon[i].index = i+1;
    place_monster_rand(&mon[i]);
    buffer_update(mon[i].x, 1, mon[i].y, 1, mon[i].race);
  }

  

  print_screen2(mon,num_mon);



  heap_t tq;
  heap_init(&tq, monster_turn_cmp, NULL);

  for(i = 0; i < num_mon; i++){
    heap_insert(&tq, &mon[i]);
  }

  heap_insert(&tq,&pc);



  monster_t *c;



  while(pc.alive == 1 && quit_game == 0){

    c = (monster_t*) heap_remove_min(&tq);

    if(c->alive == 1){
      if(c->index == 0){
        
        quit_game = pc_move(&dx, &dy, mon, num_mon);
        attacked = 0;

        if(dx != 0 || dy != 0){
          move_character(&pc,dx,dy);


          for(i = 0; i < num_mon; i++){
            if(c->index != mon[i].index && c->x == mon[i].x && c->y == mon[i].y && mon[i].alive == 1){
              mon[i].hp -= pc_attack();
              attacked = 1;
              if(mon[i].hp <= 0){
                mon[i].alive = 0;
                mon[i].race = 'X';                
              } else {
                move_character(&pc,-dx,-dy);                
              }
            }
          }

          for(i = 0; i < items_allocated; i++){
            if(pc.x == object[i]->x && pc.y == object[i]->y && object[i]->status[0] == 1){
              pickup_item();

            }
          }
          

          call_dijkstra(pc.x,pc.y,0);
          call_dijkstra(pc.x,pc.y,1);
        }

        
        print_screen2(mon,num_mon);


      } else {
        character_turn(c,mon,num_mon);
      }

 
    }

    c->turn += 100/c->speed;
    heap_insert(&tq,c);

    monsters_left = 0;

    for(i = 0; i < num_mon; i++){
      if(mon[i].alive == 1){
        monsters_left = 1;
        break;
      }
    }

    if(monsters_left == 0){
      break;
    }
  }



  heap_delete(&tq);


  if(!quit_game){
    print_screen2(mon,num_mon);
    if(pc.alive == 0){
      mvprintw(22,0,"GAME OVER: YOU ARE DIE");
      mvprintw(23,0,"PRESS ENTER TO CONTINUE");
    } else {
      mvprintw(22,0,"ALL MONSTERS ARE DESTROY. YOU WIN.");
      mvprintw(23,0,"GOOD JOB: PRESS ENTER TO CONTINUE");
    }


    int ch = 0;

    while(ch != 012){
      ch = getch();
    }
  }

  endwin();

    deallocate_leftovers();
  
  return 0;
}






/*============================================================================
                                    FUNCTIONS
  ============================================================================*/


int item_screen_print(struct monster *mon, int l_edge, int arrow, int num_mon, int which){
  int i,j;
  const char side_char = '|';
  const char top_bot_char = '=';
  int t_edge = 2;
  int b_edge = 20;
  int r_edge = l_edge+48;


  print_screen2(mon,num_mon);

  for(j = t_edge+1;j < b_edge; j++){
    for(i = l_edge+1; i < r_edge; i++){
      mvprintw(j,i," ");
    }
  }

  for(j = t_edge+1;j < b_edge; j++){
      mvprintw(j,l_edge,"%c",side_char);
      mvprintw(j,r_edge,"%c",side_char);
  }


  for(i = l_edge+1; i < r_edge; i++){
    mvprintw(t_edge,i,"%c",top_bot_char);
    mvprintw(b_edge,i,"%c",top_bot_char);
    mvprintw(t_edge+2,i,"-");
  }

  mvprintw(t_edge,l_edge,"+");
  mvprintw(t_edge,r_edge,"+");
  mvprintw(b_edge,l_edge,"+");
  mvprintw(b_edge,r_edge,"+");


  if(which == 0){
    mvprintw(t_edge+1,l_edge+1,"Select item to drop: ");
  } else if(which == 1) {
    mvprintw(t_edge+1,l_edge+1,"Select item to equip: ");
  } else if(which == 2) {
    mvprintw(t_edge+1,l_edge+1,"Select item to expunge: ");
  } else if (which == 3){
    mvprintw(t_edge+1,l_edge+1,"Items: ");
  } else if (which == 4){
    mvprintw(t_edge+1,l_edge+1,"Select item to inspect: ");
  }

int base_start = 4;
if(arrow == -1){
  base_start = 1;
}

  for(i = 0; i < 10; i++){
    if(pc_slot.inventory[i]->status[1] == 1){
      attron(COLOR_PAIR(pc_slot.inventory[i]->color));
      mvprintw(i+t_edge+3,l_edge+base_start+3,"%c",pc_slot.inventory[i]->SYMB);
      attroff(COLOR_PAIR(pc_slot.inventory[i]->color));

      mvprintw(i+t_edge+3,l_edge+base_start+6,"%s",pc_slot.inventory[i]->NAME);
    }
    mvprintw(i+t_edge+3,l_edge+base_start,"%d",0+i);
    mvprintw(i+t_edge+3,l_edge+base_start+1,")[");
    mvprintw(i+t_edge+3,l_edge+base_start+4,"]");
  }

  mvprintw(22,0,"Arrows to move                         ");
  mvprintw(23,0,"Escape key to exit                ");
                
  if(arrow != -1){
    mvprintw(t_edge+3+arrow,l_edge+1,"->");
    mvprintw(22,0,"Arrows to move, Enter to select   ");
  }



  refresh();

  return 0;
}



int expunge_item_input(struct monster *mon, int num_mon){
  int key_pressed = 0;
  int l_edge = 30;
  int arrow = 0;
  //int i;


  item_screen_print(mon, l_edge, arrow, num_mon, 2);

  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 012: // Enter
        if(pc_slot.inventory[arrow]->status[1] == 1){

          pc_slot.inventory[arrow]->status[0] = 0;
          pc_slot.inventory[arrow]->status[1] = 0;
          pc_slot.inventory[arrow]->status[2] = 0;

          return 0;
        }
        break;
      case 0404: // Left arrow
          l_edge--;
        if(l_edge == 0){
          l_edge = 80-50;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 2);
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 2);
        break;
      default:
        break;
      case 0402: // Down arrow
          arrow++;
          if(arrow > 9){
            arrow = 0;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 2);
        break;
      case 0403: // Up arrow
          arrow--;
          if(arrow < 0){
            arrow = 9;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 2);
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}

int list_input(struct monster *mon, int num_mon,int which){
  int key_pressed = 0;
  int l_edge = 30;

  if(which == 0){
    item_screen_print(mon, l_edge, -1, num_mon, 3);
  } else if(which == 1){
    equipped_screen_print(mon, l_edge, -1, num_mon,1);
  }

  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 0404: // Left arrow
          l_edge--;
        if(l_edge == 0){
          l_edge = 80-50;
        }
        if(which == 0){
          item_screen_print(mon, l_edge, -1, num_mon, 3);
        } else if(which == 1){
          equipped_screen_print(mon, l_edge, -1, num_mon,1);
        }
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        if(which == 0){
          item_screen_print(mon, l_edge, -1, num_mon, 3);
        } else if(which == 1){
          equipped_screen_print(mon, l_edge, -1, num_mon,1);
        }
        break;
      default:
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}

int display_item_desc(int k){
  int key = 0;
  int i,j;

  for(j = 0; j < 19;j++){
    for(i = 0; i < 80; i++){
      mvprintw(j+2,i,"%c",' ');
    }
  }

  mvprintw(2,0,"[");
  attron(COLOR_PAIR(pc_slot.inventory[k]->color));
  printw("%c",pc_slot.inventory[k]->SYMB);
  attroff(COLOR_PAIR(pc_slot.inventory[k]->color));
  printw("] - %s:",pc_slot.inventory[k]->NAME);
  mvprintw(3,0,"%s",pc_slot.inventory[k]->DESC);

  mvprintw(22,0,"Escape key to exit                     ");
  mvprintw(23,0,"                                  ");

  while(key != 033){
    key = getch();
  }
  return 0;
}

int inspect_item_input(struct monster *mon, int num_mon){
  int key_pressed = 0;
  int l_edge = 30;
  int arrow = 0;


  item_screen_print(mon, l_edge, arrow, num_mon, 4);

  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 012: // Enter
        if(pc_slot.inventory[arrow]->status[1] == 1){
          display_item_desc(arrow);
          item_screen_print(mon, l_edge, arrow, num_mon, 4);
        } else {
          mvprintw(22,0,"No item in this slot to inspect");
        }
        break;
      case 0404: // Left arrow
          l_edge--;
        if(l_edge == 0){
          l_edge = 80-50;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 4);
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 4);
        break;
      default:
        break;
      case 0402: // Down arrow
          arrow++;
          if(arrow > 9){
            arrow = 0;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 4);
        break;
      case 0403: // Up arrow
          arrow--;
          if(arrow < 0){
            arrow = 9;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 4);
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}

int equip_item_input(struct monster *mon, int num_mon){
  int key_pressed = 0;
  int l_edge = 30;
  int arrow = 0;
  int i;


  item_screen_print(mon, l_edge, arrow, num_mon, 1);

  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 012: // Enter
        if(pc_slot.inventory[arrow]->status[1] == 1){

            object_class * temp;
            temp = new object_class(1);

            if(!strcmp(pc_slot.inventory[arrow]->TYPE,"RING")){
              if(pc_slot.equipment[11]->status[2] == 0){
                i = 11;
              } else {
                i = 10;
              }
            } else {
              for(i = 0; i < 10; i++){
                if(!strcmp(pc_slot.inventory[arrow]->TYPE,pc_slot.equipment[i]->TYPE)){
                  break;
                }
              }
            }

            swap_item_stats(pc_slot.equipment[i],temp);
            swap_item_stats(pc_slot.inventory[arrow],pc_slot.equipment[i]);
            swap_item_stats(temp,pc_slot.inventory[arrow]);

            pc_slot.equipment[i]->status[1] = 0;
            pc_slot.equipment[i]->status[2] = 1;
            pc_slot.inventory[arrow]->status[1] = temp->status[2];
            pc_slot.inventory[arrow]->status[2] = 0;


            delete temp;

          return 0;
        }
        break;
      case 0404: // Left arrow
          l_edge--;
        if(l_edge == 0){
          l_edge = 80-50;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 1);
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 1);
        break;
      default:
        break;
      case 0402: // Down arrow
          arrow++;
          if(arrow > 9){
            arrow = 0;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 1);
        break;
      case 0403: // Up arrow
          arrow--;
          if(arrow < 0){
            arrow = 9;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 1);
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}



int drop_item_input(struct monster *mon, int num_mon){
  int key_pressed = 0;
  int l_edge = 30;
  int arrow = 0;
  int i;


  item_screen_print(mon, l_edge, arrow, num_mon, 0);

  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 012: // Enter
        if(pc_slot.inventory[arrow]->status[1] == 1){
          for(i = 0; i < items_allocated; i++){
            if(object[i]->status[0] == 0){
              break;
            }
          }

          swap_item_stats(pc_slot.inventory[arrow],object[i]);



          pc_slot.inventory[arrow]->status[1] = 0;
          object[i]->status[0] = 1;
          object[i]->x = pc.x;
          object[i]->y = pc.y;

          return 0;
        }
        break;
      case 0404: // Left arrow
          l_edge--;
        if(l_edge == 0){
          l_edge = 80-50;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 0);
        break;
      case 0405: // Right arrow
          l_edge++;
        if(l_edge == 80-49){
          l_edge = 1;
        }
        item_screen_print(mon, l_edge, arrow, num_mon, 0);
        break;
      default:
        break;
      case 0402: // Down arrow
          arrow++;
          if(arrow > 9){
            arrow = 0;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 0);
        break;
      case 0403: // Up arrow
          arrow--;
          if(arrow < 0){
            arrow = 9;
          }
          item_screen_print(mon, l_edge, arrow, num_mon, 0);
        break;
      case 033: // Escape key
        return -1;
        break;
    }
  }

  return 0;
}





int new_monster(monster_t *m){
  int t;
  t = rand() % num_mon_types;
  
  m->intel = (unsigned int) c[t]->SMART;
  m->tel = (unsigned int) c[t]->TELE;
  m->tunneling = (unsigned int) c[t]->TUNNEL;
  m->erratic = (unsigned int) c[t]->ERRATIC;
  m->speed = roll_dice(c[t]->SPEED);
  m->hp = roll_dice(c[t]->HP);
  m->dam[0] = c[t]->DAM[0];
  m->dam[1] = c[t]->DAM[1];
  m->dam[2] = c[t]->DAM[2];
  m->turn = 1;
  m->alive = 1;

  
  m->race = m->intel | m->tel << 1 | m->tunneling << 2 | m->erratic << 3;

  
  m->race = c[t]->SYMB;



    if(c[t]->COLOR[COLOR_WHITE] == 1){
      m->color = COLOR_WHITE;
    } else if (c[t]->COLOR[COLOR_RED] == 1){
      m->color = COLOR_RED;
    } else if (c[t]->COLOR[COLOR_BLUE] == 1){
      m->color = COLOR_BLUE;
    } else if (c[t]->COLOR[COLOR_GREEN] == 1){
      m->color = COLOR_GREEN;
    } else if (c[t]->COLOR[COLOR_MAGENTA] == 1){
      m->color = COLOR_MAGENTA;
    } else if (c[t]->COLOR[COLOR_YELLOW] == 1){
      m->color = COLOR_YELLOW;
    } else if (c[t]->COLOR[COLOR_CYAN] == 1){
      m->color = COLOR_CYAN;
    } else if (c[t]->COLOR[COLOR_BLACK] == 1){
      m->color = COLOR_BLACK;
    } else {
      m->color = COLOR_WHITE;
    }





  return 0;
}





int parse_objects(){
  int i;
  char str[100];
  int obj_num = 0;
  int desc_size;
  char desc_lim[4];

  desc_lim[3] = 0;



  char object_src[] = "/.rlg327/object_desc.txt";
  char object_file[300];
  strcpy(object_file,getenv("HOME"));
  strcat(object_file,object_src);

  
  //const char object_file[] = "object_desc.txt";


  printf("Parsing Object Types\n");


  std::ifstream f(object_file);

  f.seekg (0, f.end);
  int length = f.tellg();
  f.seekg (0, f.beg);
  

   
  while(1){
  
   while(f.peek()=='\n' || f.peek()==' ' || f.peek()=='\t'){
     f.seekg(1,f.cur);
   }
   f.get(str,length);

    if(!strncmp(str,"BEGIN OBJECT", 12)){
      num_obj_types++;
    }
    
    
    f.seekg(1,f.cur);
    
    
    
    
    if(f.tellg() == -1){
      break;
    }
    
    
  }

  
  f.close();
  f.open(object_file);
  
  item = (object_type**) malloc(num_obj_types*sizeof(object_type*));

  for(i = 0;i<num_obj_types;i++){
    item[i] = new object_type;
  }


  printf("%s%d%s\n","There are ",num_obj_types," types of items.");


  while(1){
    
    while(f.peek()=='\n' || f.peek()==' ' || f.peek()=='\t'){
      f.seekg(1,f.cur);
    }
    
    f.get(str,length);

    if(!strncmp(str,"BEGIN OBJECT", 13)){
      
    
      while(1){

        f.seekg(1,f.cur);
      
        while(f.peek()=='\n' || f.peek() ==' ' || f.peek()=='\t'){
          f.seekg(1,f.cur);
        }
      
        f.get(str,length);
        if (!strncmp(str,"NAME", 4)){
          strcpy(item[obj_num]->NAME,str+5);
        } else if (!strncmp(str,"TYPE", 4)){
          strcpy(item[obj_num]->TYPE,str+5);
        } else if (!strncmp(str,"DESC", 4)){
           desc_size = 0;
           while(1){
            
              f.seekg(1,f.cur);
                desc_lim[0] = desc_lim[1];
                desc_lim[1] = desc_lim[2];
                desc_lim[2] = f.peek();
                desc_size++;
                if(!strncmp(desc_lim,"\n.", 2)){
                  break;
                }
           }
          f.seekg(-desc_size,f.cur);
          
          
          
          item[obj_num]->DESC = (char*) malloc(desc_size+1*sizeof(char));
          f.get(item[obj_num]->DESC,desc_size,'`');
          
          
        } else if (!strncmp(str,"COLOR", 5)){
        
          if(strstr(str,"BLACK") != 0){
            item[obj_num]->COLOR[COLOR_BLACK] = 1;
          }

          if(strstr(str,"RED") != 0){
            item[obj_num]->COLOR[COLOR_RED] = 1;
          }

          if(strstr(str,"BLUE") != 0){
            item[obj_num]->COLOR[COLOR_BLUE] = 1;
          }          

          if(strstr(str,"GREEN") != 0){
            item[obj_num]->COLOR[COLOR_GREEN] = 1;
          }

          if(strstr(str,"YELLOW") != 0){
            item[obj_num]->COLOR[COLOR_YELLOW] = 1;
          }
          
          if(strstr(str,"MAGENTA") != 0){
            item[obj_num]->COLOR[COLOR_MAGENTA] = 1;
          }
          if(strstr(str,"CYAN") != 0){
            item[obj_num]->COLOR[COLOR_CYAN] = 1;
          }
          if(strstr(str,"WHITE") != 0){
            item[obj_num]->COLOR[COLOR_WHITE] = 1;
          }
        } else if (!strncmp(str,"SPEED", 5)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->SPEED[0],&item[obj_num]->SPEED[1],&item[obj_num]->SPEED[2]);          
        } else if (!strncmp(str,"DAM", 3)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->DAM[0],&item[obj_num]->DAM[1],&item[obj_num]->DAM[2]);
        } else if (!strncmp(str,"HIT", 3)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->HIT[0],&item[obj_num]->HIT[1],&item[obj_num]->HIT[2]);
        } else if (!strncmp(str,"DODGE", 5)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->DODGE[0],&item[obj_num]->DODGE[1],&item[obj_num]->DODGE[2]);
        } else if (!strncmp(str,"DEF", 3)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->DEF[0],&item[obj_num]->DEF[1],&item[obj_num]->DEF[2]);
        } else if (!strncmp(str,"WEIGHT", 6)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->WEIGHT[0],&item[obj_num]->WEIGHT[1],&item[obj_num]->WEIGHT[2]);
        } else if (!strncmp(str,"ATTR", 4)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->ATTR[0],&item[obj_num]->ATTR[1],&item[obj_num]->ATTR[2]);
        } else if (!strncmp(str,"VAL", 3)){
          sscanf(str,"%*s %d%*c%d%*c%d",&item[obj_num]->VAL[0],&item[obj_num]->VAL[1],&item[obj_num]->VAL[2]);
        } else if (!strncmp(str,"END", 3)){
          break;
        }
      }

      obj_num++;
    } 


    f.seekg(1,f.cur);
  
    
      
    if(f.tellg()==-1){
      break;
    }
  }

  f.close();
  

  printf("Done Parsing Object Types\n");
  return 0;
}


  
int display_parsed_objects(){
  int i;
  printf("Displaying Parsed Object Templates\n");

  for(i = 0;i < num_obj_types; i++){
    printf("\n");
    printf("Name: %s",item[i]->NAME);
    printf("\nTYPE: %s",item[i]->TYPE);
    printf("\nDescription: %s",item[i]->DESC);
    printf("Color: ");
    if(item[i]->COLOR[COLOR_WHITE] == 1){
      printf("WHITE ");
    }
    if(item[i]->COLOR[COLOR_CYAN] == 1){
      printf("CYAN ");
    }
    if(item[i]->COLOR[COLOR_MAGENTA] == 1){
      printf("MAGENTA ");
    }
    if(item[i]->COLOR[COLOR_YELLOW] == 1){
      printf("YELLOW ");
    }
    if(item[i]->COLOR[COLOR_GREEN] == 1){
      printf("GREEN ");
    }
    if(item[i]->COLOR[COLOR_BLUE] == 1){
      printf("BLUE ");
    }
    if(item[i]->COLOR[COLOR_RED] == 1){
      printf("RED ");
    }
    if(item[i]->COLOR[COLOR_BLACK] == 1){
      printf("BLACK ");
    }


    printf("\nSpeed: %d+%dd%d\n",item[i]->SPEED[0],item[i]->SPEED[1],item[i]->SPEED[2]);
    printf("DAM: %d+%dd%d\n",item[i]->DAM[0],item[i]->DAM[1],item[i]->DAM[2]);
    printf("HIT: %d+%dd%d\n",item[i]->HIT[0],item[i]->HIT[1],item[i]->HIT[2]);
    printf("DODGE: %d+%dd%d\n",item[i]->DODGE[0],item[i]->DODGE[1],item[i]->DODGE[2]);
    printf("DEF: %d+%dd%d\n",item[i]->DEF[0],item[i]->DEF[1],item[i]->DEF[2]);
    printf("WEIGHT: %d+%dd%d\n",item[i]->WEIGHT[0],item[i]->WEIGHT[1],item[i]->WEIGHT[2]);
    printf("ATTR: %d+%dd%d\n",item[i]->ATTR[0],item[i]->ATTR[1],item[i]->ATTR[2]);
    printf("VAL: %d+%dd%d\n",item[i]->VAL[0],item[i]->VAL[1],item[i]->VAL[2]);
  }
  return 0;
}



  
int display_parsed_monsters(){
  int i;
  printf("Displaying Parsed Monster Templates\n");

  for(i = 0;i < num_mon_types; i++){
    printf("\n");
    printf("%s",c[i]->NAME);
    printf("%s",c[i]->DESC);
    printf("%c\n",c[i]->SYMB);
    if(c[i]->COLOR[COLOR_WHITE] == 1){
      printf("WHITE ");
    }
    if(c[i]->COLOR[COLOR_CYAN] == 1){
      printf("CYAN ");
    }
    if(c[i]->COLOR[COLOR_MAGENTA] == 1){
      printf("MAGENTA ");
    }
    if(c[i]->COLOR[COLOR_YELLOW] == 1){
      printf("YELLOW ");
    }
    if(c[i]->COLOR[COLOR_GREEN] == 1){
      printf("GREEN ");
    }
    if(c[i]->COLOR[COLOR_BLUE] == 1){
      printf("BLUE ");
    }
    if(c[i]->COLOR[COLOR_RED] == 1){
      printf("RED ");
    }
    if(c[i]->COLOR[COLOR_BLACK] == 1){
      printf("BLACK ");
    }


    printf("\n%d+%dd%d\n",c[i]->SPEED[0],c[i]->SPEED[1],c[i]->SPEED[2]);
    if(c[i]->SMART == 1){
      printf("SMART ");
    }
    if(c[i]->TELE == 1){
      printf("TELE ");
    }
    if(c[i]->TUNNEL == 1){
      printf("TUNNEL ");
    }
    if(c[i]->ERRATIC == 1){
      printf("ERRATIC ");
    }
    if(c[i]->PASS == 1){
      printf("PASS");
    }
    printf("\n%d+%dd%d\n",c[i]->HP[0],c[i]->HP[1],c[i]->HP[2]);
    printf("%d+%dd%d\n",c[i]->DAM[0],c[i]->DAM[1],c[i]->DAM[2]);



  }
  return 0;
}

int deallocate_leftovers(){
  int i;
  int inventory_slots = 0;

  for(i = 0;i<num_mon_types;i++){
    delete c[i];
  }


  free(c);

  for(i = 0;i<num_obj_types;i++){
    delete item[i];
  }  

  free(item);


  for(i = 0; i < 10; i++){
    if(pc_slot.inventory[i]->status[1] == 1){
      inventory_slots++;
    }
  }


  for(i = 0; i < items_allocated;i++){
    delete object[i];
  }

  free(object);



  for(i = 0; i < 12; i++){
    delete pc_slot.equipment[i];
  }


  for(i = 0; i < 10; i++){
    delete pc_slot.inventory[i];
  }


  return 0;
}
  
  
int parse_monsters(){
  int i;
  char str[100];
  int mon_num = 0;
  int desc_size;
  char desc_lim[4];

  desc_lim[3] = 0;



  char monster_src[] = "/.rlg327/monster_desc.txt";
  char monster_file[300];
  strcpy(monster_file,getenv("HOME"));
  strcat(monster_file,monster_src);

  
  //char monster_file[] = "monster_desc.txt";


  printf("Parsing Monster Types\n");


  std::ifstream f(monster_file);

  f.seekg (0, f.end);
  int length = f.tellg();
  f.seekg (0, f.beg);
  

   
  while(1){
  
   while(f.peek()=='\n' || f.peek()==' ' || f.peek()=='\t'){
     f.seekg(1,f.cur);
   }
   f.get(str,length);

    if(!strncmp(str,"BEGIN MONSTER", 13)){
      num_mon_types++;
    }
    
    
    f.seekg(1,f.cur);
    
    
    
    
    if(f.tellg() == -1){
      break;
    }
    
    
  }

  
  f.close();
  f.open(monster_file);
  
  c = (character_type**) malloc(num_mon_types*sizeof(character_type*));

  for(i = 0;i<num_mon_types;i++){
    c[i] = new character_type;
  }


  printf("%s%d%s\n","There are ",num_mon_types," types of monsters.");


  while(1){
    
    while(f.peek()=='\n' || f.peek()==' ' || f.peek()=='\t'){
      f.seekg(1,f.cur);
    }
    
    f.get(str,length);

    if(!strncmp(str,"BEGIN MONSTER", 13)){
      
    
      while(1){

        f.seekg(1,f.cur);
      
        while(f.peek()=='\n' || f.peek() ==' ' || f.peek()=='\t'){
          f.seekg(1,f.cur);
        }
      
        f.get(str,length);
        if (!strncmp(str,"NAME", 4)){
          strcpy(c[mon_num]->NAME,str+5);
        } else if (!strncmp(str,"DESC", 4)){
           desc_size = 0;
           while(1){
            
              f.seekg(1,f.cur);
                desc_lim[0] = desc_lim[1];
                desc_lim[1] = desc_lim[2];
                desc_lim[2] = f.peek();
                desc_size++;
                if(!strncmp(desc_lim,"\n.", 2)){
                  break;
                }
           }
          f.seekg(-desc_size,f.cur);
          
          
          
          c[mon_num]->DESC = (char*) malloc(desc_size+1*sizeof(char));
          f.get(c[mon_num]->DESC,desc_size,'`');
          
          
        } else if (!strncmp(str,"COLOR", 5)){
        
          if(strstr(str,"BLACK") != 0){
            c[mon_num]->COLOR[COLOR_BLACK] = 1;
          }

          if(strstr(str,"RED") != 0){
            c[mon_num]->COLOR[COLOR_RED] = 1;
          }

          if(strstr(str,"BLUE") != 0){
            c[mon_num]->COLOR[COLOR_BLUE] = 1;
          }          

          if(strstr(str,"GREEN") != 0){
            c[mon_num]->COLOR[COLOR_GREEN] = 1;
          }

          if(strstr(str,"YELLOW") != 0){
            c[mon_num]->COLOR[COLOR_YELLOW] = 1;
          }
          
          if(strstr(str,"MAGENTA") != 0){
            c[mon_num]->COLOR[COLOR_MAGENTA] = 1;
          }
          if(strstr(str,"CYAN") != 0){
            c[mon_num]->COLOR[COLOR_CYAN] = 1;
          }
          if(strstr(str,"WHITE") != 0){
            c[mon_num]->COLOR[COLOR_WHITE] = 1;
          }
        } else if (!strncmp(str,"SPEED", 5)){
          sscanf(str,"%*s %d%*c%d%*c%d",&c[mon_num]->SPEED[0],&c[mon_num]->SPEED[1],&c[mon_num]->SPEED[2]);
        } else if (!strncmp(str,"ABIL", 4)){

          if(strstr(str,"SMART") != 0){
            c[mon_num]->SMART = 1;
          }
          if(strstr(str,"PASS") != 0){
            c[mon_num]->PASS = 1;
          }
          if(strstr(str,"TELE") != 0){
            c[mon_num]->TELE = 1;
          }
          if(strstr(str,"TUNNEL") != 0){
            c[mon_num]->TUNNEL = 1;
          }
          if(strstr(str,"ERRATIC") != 0){
            c[mon_num]->ERRATIC = 1;
          }
          
        } else if (!strncmp(str,"HP", 2)){
          sscanf(str,"%*s %d%*c%d%*c%d",&c[mon_num]->HP[0],&c[mon_num]->HP[1],&c[mon_num]->HP[2]);
        } else if (!strncmp(str,"DAM", 3)){
          sscanf(str,"%*s %d%*c%d%*c%d",&c[mon_num]->DAM[0],&c[mon_num]->DAM[1],&c[mon_num]->DAM[2]);
        } else if (!strncmp(str,"SYMB", 4)){
          sscanf(str,"%*s %c",&c[mon_num]->SYMB);
        } else if (!strncmp(str,"END", 3)){
          break;
        }
      }

      mon_num++;
    } 


    f.seekg(1,f.cur);
  
    
      
    if(f.tellg()==-1){
      break;
    }
  }

  f.close();
  

  printf("Done Parsing Monster Types\n");
  return 0;
}
  


int print_screen2(struct monster *mon,int num_mon){
  /* Takes the buffer[][] array and prints it line by line to the terminal*/

  int i, j;
  int monsters_alive=0;
  
clear();

  for(i = 0;i < num_rooms; i++)
  {
    buffer_update(room_arr[i].x_pos,room_arr[i].x_size,room_arr[i].y_pos,room_arr[i].y_size,room_char);
  }


  for(i = 0; i < 80; i++){
    for(j = 0; j < 21; j++){
      if((hardness[j][i] == 0) && (buffer[j][i] != room_char))
      {
        buffer[j][i] = path_char;
      }
    }
  }

  //for(i = 0;i < num_objects;i++){
  //  buffer[object[i]->y][object[i]->x] = object[i]->SYMB;
  //}


  buffer[up_stair_y][up_stair_x] = up_stair_char;
  buffer[down_stair_y][down_stair_x] = down_stair_char;


  for(i = 0;i < num_mon;i++){
    if(mon[i].alive == 1){
      buffer_update(mon[i].x, 1, mon[i].y, 1, mon[i].race);
      monsters_alive++;
    }
  }



  buffer[pc.y][pc.x] = pc.race;


  

  for(j = 0; j < 21; j++){
    for(i = 0; i < 80; i++){
      mvprintw(j+1,i,&buffer[j][i]);
    }
  }
  mvprintw(0,59,"Monsters left: ");
  printw("%2d",monsters_alive);
  mvprintw(22,0,message_1);
  mvprintw(23,0,message_2);
  mvprintw(22,39,"Inventory: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]");
  mvprintw(23,34,"Equipped: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]");
  for(i = 0; i < 10; i++){
    if(pc_slot.inventory[i]->status[1] == 1){
      attron(COLOR_PAIR(pc_slot.inventory[i]->color));
      mvprintw(22,51+3*i,"%c",pc_slot.inventory[i]->SYMB);
      attroff(COLOR_PAIR(pc_slot.inventory[i]->color));
    }
  }
  for(i = 0; i < 12; i++){
    if(pc_slot.equipment[i]->status[2] == 1){
      attron(COLOR_PAIR(pc_slot.equipment[i]->color));
      mvprintw(23,45+3*i,"%c",pc_slot.equipment[i]->SYMB);
      attroff(COLOR_PAIR(pc_slot.equipment[i]->color));
    }
  }



  for(i = 0;i < items_allocated;i++){
    if(object[i]->status[0] == 1){
      attron(COLOR_PAIR(object[i]->color));
      mvprintw(object[i]->y+1,object[i]->x,"%c",object[i]->SYMB);
      attroff(COLOR_PAIR(object[i]->color));
    }
  }

  for(i = 0;i < num_mon;i++){
    if(mon[i].alive == 1){
      attron(COLOR_PAIR(mon[i].color));
      mvprintw(mon[i].y+1,mon[i].x,"%c",mon[i].race);
      attroff(COLOR_PAIR(mon[i].color));
    }
  }

  mvprintw(0,0,"HP: %d   SPEED: %d",pc.hp,pc.speed);



  mvprintw(pc.y+1,pc.x,"%c",pc.race);

  if(attacked == 1){
    mvprintw(0,23,"Damage dealt:  %d",damage_dealt);
  }

  refresh();

  return 0;
}


int print_monster_window(struct monster *mon, int num_mon, int l_edge, int mon_start){
  int i,j;
  const char side_char = '|';
  const char top_bot_char = '=';
  const char *vert = "north";
  const char *horiz = "east";
  int t_edge = 2;
  int b_edge = 20;
  int r_edge = l_edge+24;
  int n = 0;

  print_screen2(mon,num_mon);

  for(j = t_edge+1;j < b_edge; j++){
    for(i = l_edge+1; i < r_edge; i++){
      mvprintw(j,i," ");
    }
  }

  for(j = t_edge+1;j < b_edge; j++){
      mvprintw(j,l_edge,"%c",side_char);
      mvprintw(j,r_edge,"%c",side_char);
  }


  for(i = l_edge+1; i < r_edge; i++){
    mvprintw(t_edge,i,"%c",top_bot_char);
    mvprintw(b_edge,i,"%c",top_bot_char);
    mvprintw(t_edge+2,i,"-");
  }

  mvprintw(t_edge,l_edge,"+");
  mvprintw(t_edge,r_edge,"+");
  mvprintw(b_edge,l_edge,"+");
  mvprintw(b_edge,r_edge,"+");





  mvprintw(t_edge+1,l_edge+1,"Monster Locations:");



  i = 1;
  for(j = 0; j < num_mon && i < 16; j++){
    if(mon[j].alive == 1){
      if(n >= mon_start){
        if(pc.y < mon[j].y){
          vert = "south";
        } else {
          vert = "north";
        }

        if(pc.x < mon[j].x){
          horiz = "east";
        } else {
          horiz = "west";
        }

        attron(COLOR_PAIR(mon[i].color));
        mvprintw(i+t_edge+2,l_edge+1,"%c",mon[j].race);
        attroff(COLOR_PAIR(mon[i].color));

        mvprintw(i+t_edge+2,l_edge+2,", %2d %s and %2d %s", abs(pc.y-mon[j].y), vert, abs(pc.x-mon[j].x), horiz);
        i++;
      }
      n++;
    }
  }


  refresh();

  return 0;
}




int monster_list(struct monster *mon,int num_mon){
  int i;
  int key_pressed = 0;
  int l_edge = 54;
  int mon_alive = 0;
  int mon_start = 0;


  print_monster_window(mon,num_mon, l_edge, mon_start);

  for(i = 0; i < num_mon; i++){
    if(mon[i].alive == 1){
      mon_alive++;
    }
  }



  while(1){

    key_pressed = getch();

    switch(key_pressed){
      case 0404: // Left arrow
        if(l_edge > 0){
          l_edge--;
        }
        if(l_edge == 0){
          l_edge = 80-24-2;
        }
        print_monster_window(mon,num_mon, l_edge, mon_start);
        break;
      case 0405: // Right arrow
        if(l_edge < 80-24-1){
          l_edge++;
        }
        if(l_edge == 80-24-1){
          l_edge = 1;
        }
        print_monster_window(mon,num_mon, l_edge, mon_start);
        break;
      default:
        break;
      case 0402: // Down arrow
        if(mon_alive > 15 && mon_start < mon_alive -15){
          mon_start++;
          print_monster_window(mon,num_mon, l_edge, mon_start);
        }
        break;
      case 0403: // Up arrow
        if(mon_alive > 15 && mon_start > 0){
          mon_start--;
          print_monster_window(mon,num_mon, l_edge, mon_start);
        }
        break;
      case 033: // Escape key
        return 0;
        break;
    }
  }

  return 0;
}


int regenerate_dungeon(struct monster *mon,int num_mon){
  int i;
  int inventory_slots = 0;
  int equipment_slots = 0;
  
  num_rooms = 6 + rand() % 3;
  rand_dungeon();
  assign_hardness();



  for(i = 0; i < items_allocated;i++){
    delete object[i];
  }

  free(object);

  for(i = 0; i < 10; i++){
    if(pc_slot.inventory[i]->status[1] == 1){
      inventory_slots++;
    }
  }

  for(i = 0; i < 12; i++){
    if(pc_slot.equipment[i]->status[2] == 1){
      equipment_slots++;
    }
  }

  num_objects = 10 + rand() % 5;

  items_allocated = num_objects + inventory_slots + equipment_slots;

  object = (object_class**) malloc(items_allocated*sizeof(object_class*));

  for(i = 0;i<num_objects;i++){
    object[i] = new object_class;
  }

  for(i = 0; i < inventory_slots; i++){
    object[num_objects+i] = new object_class(1);
    object[num_objects+i]->status[0] = 0;
  }

  for(i = 0; i < equipment_slots; i++){
    object[inventory_slots+num_objects+i] = new object_class(1);
    object[inventory_slots+num_objects+i]->status[0] = 0;
  }


  place_pc_rand();
  place_stairs();
  pc.turn = 0;
  for(i = 0; i < num_mon; i++){
    new_monster(&mon[i]);
    place_monster_rand(&mon[i]);
    buffer_update(mon[i].x, 1, mon[i].y, 1, mon[i].race);
  }

  return 0;
}


int pc_move(int *dx, int *dy, struct monster *mon,int num_mon){
  int valid_key, key_pressed, i;
  int something_to_drop;
  int something_to_wear;

  while(1){

    something_to_drop = 0;
    something_to_wear = 0;

    pc.speed = 10;
    for(i = 0; i < 12; i++){
      pc.speed += pc_slot.equipment[i]->SPEED;
    }

    if(pc.speed > 100){
      pc.speed = 100;
    } else if(pc.speed < 1){
      pc.speed = 1;
    }

    print_screen2(mon,num_mon);

    valid_key = 1;

    key_pressed = getch();

    switch(key_pressed){
      case 0111: // Inspect item (I)
        inspect_item_input(mon,num_mon);

        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0145: // List equipment (e)
        list_input(mon,num_mon, 1);

        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0151: // List inventory (i)
        list_input(mon,num_mon, 0);

        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0164: // Unequip item (t)
        for(i = 0; i < 12; i++){
          if(pc_slot.equipment[i]->status[2] == 1){
                something_to_wear = 1;
                break;
          }
        }

        if(something_to_wear == 1){
          unequip_item_input(mon,num_mon);
        } else {
          message_1 = "Invalid key pressed                                                ";
          message_2 = "Nothing equipped to take off                                       ";
        }
        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0170: // Expunge item (x)
        for(i = 0; i < 10; i++){
          if(pc_slot.inventory[i]->status[1] == 1){
                something_to_wear = 1;
                break;
          }
        }

        if(something_to_wear == 1){
          expunge_item_input(mon,num_mon);
        } else {
          message_1 = "Invalid key pressed                                            ";
          message_2 = "Nothing in inventory to expunge                             ";
        }
        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0167:  // Wear (w)
        for(i = 0; i < 10; i++){
          if(pc_slot.inventory[i]->status[1] == 1){
            something_to_wear = 1;
            break;
          }
        }

        if(something_to_wear == 1){
          equip_item_input(mon, num_mon);
        } else {
          message_1 = "Invalid key pressed                                        ";
          message_2 = "Nothing in inventory to equip                              ";
        }
        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 0144:  // Drop (d)
        for(i = 0; i < 10; i++){
          if(pc_slot.inventory[i]->status[1] == 1){
            something_to_drop = 1;
            break;
          }
        }

        if(something_to_drop == 1){
          drop_item_input(mon, num_mon);
        } else {
          message_1 = "Invalid key pressed                                            ";
          message_2 = "Nothing in inventory to drop                                    ";
        }
        valid_key = 0;
        *dx = 0;
        *dy = 0;
        break;
      case 061: // Down-left (1, b)
      case 0142:
        *dx = -1;
        *dy = 1;
        break;
      case 062: // Down (2, j)
      case 0152:
        *dx = 0;
        *dy = 1;
        break;
      case 063: // Down-right (3, n)
      case 0156:
        *dx = 1;
        *dy = 1;
        break;
      case 064: // Left (4, h)
      case 0150:
        *dx = -1;
        *dy = 0;
        break;
      case 065: // No move (5, 'space')
      case 040:
        *dx = 0;
        *dy = 0;
        break;
      case 066: // Right (6, l)
      case 0154:
        *dx = 1;
        *dy = 0;
        break;
      case 067: // Up-left (7, y)
      case 0171:
        *dx = -1;
        *dy = -1;
        break;
      case 070: // Up (8, k)
      case 0153:
        *dx = 0;
        *dy = -1;
        break;
      case 071: // Up-right (9, u)
      case 0165:
        *dx = 1;
        *dy = -1;
        break;
      case 0155: // Map (m)
        message_1 = "Arrows to move        ";
        message_2 = "Escape key to exit        ";
        monster_list(mon, num_mon);
        message_1 = "Status information 1";
        message_2 = "Status information 2";
	      *dx = 0;
	      *dy = 0; 
        //print_screen2(mon,num_mon);
        valid_key = 0;
        break;
      case 0161: // Quit (q)
        *dx = 0;
        *dy = 0;
        return 1;
        break;
      case 054: // Up-Stairs (,)
        if(pc.x == up_stair_x && pc.y == up_stair_y){
          regenerate_dungeon(mon, num_mon);
          message_1 = "Status information 1";
          message_2 = "Status information 2";
        } else {
          message_1 = "Invalid key pressed";
          message_2 = "No stairs to go up here";
        }
        *dx = 0;
        *dy = 0; 
        valid_key = 0;
        break;
      case 056: // Down-Stairs (.)
        if(pc.x == down_stair_x && pc.y == down_stair_y){
          regenerate_dungeon(mon, num_mon);
          message_1 = "Status information 1";
          message_2 = "Status information 2";
        } else {
          message_1 = "Invalid key pressed";
          message_2 = "No stairs to go down here";
        }
	*dx = 0;
  	*dy = 0;
        valid_key = 0;
        break;
      default:
        message_1 = "Invalid key pressed";
	*dx = 0;
	*dy = 0;        
        valid_key = 0;
        break;
    }

    if(pc.tunneling == 0 && hardness[pc.y+*dy][pc.x+*dx] != 0){
      valid_key = 0;
      message_1 = "Invalid move";
      message_2 = "Cannot tunnel through walls";
    }

    if(valid_key == 1){
      message_1 = "Status information 1";
      message_2 = "Status information 2";
      break;
    }
  }
  return 0;
}

int place_stairs(){

  while(1){
    up_stair_x = rand() % 80;
    up_stair_y = rand() % 21;
  
    if(buffer[up_stair_y][up_stair_x] == room_char){
      buffer_update(up_stair_x, 1, up_stair_y, 1, up_stair_char);
      break;
    }
  }

 while(1){
    down_stair_x = rand() % 80;
    down_stair_y = rand() % 21;
  
    if(buffer[down_stair_y][down_stair_x] == room_char){
      buffer_update(down_stair_x, 1, down_stair_y, 1, down_stair_char);
      break;
    }
  }

  return 0;
}


int move_character(monster_t *c, int dx, int dy){
  int hard;

  if(c->tunneling == 0){
    c->x += dx;
    c->y += dy;
  } else {
    hard = hardness[c->y+dy][c->x+dx];
    if(hard != 0 && hard != 255){
      hard -= 85;
      if(hard < 0){
        hardness[c->y+dy][c->x+dx] = 0;
      } else {
        hardness[c->y+dy][c->x+dx] -= 85;
      }
    }
    if(hardness[c->y+dy][c->x+dx] == 0){
      c->x += dx;
      c->y += dy;
    }
  }

  return 0;
}


int random_move(monster_t *c){

  int dx,dy;

  if(c->tunneling == 0){
    while(1){
      dx = -1 + rand() % 3;
      dy = -1 + rand() % 3;

      if(hardness[c->y+dy][c->x+dx] == 0 && (dx != 0 || dy != 0)){
        break;
      }
    }
    move_character(c,dx,dy);
  } else {
    while(1){
      dx = -1 + rand() % 3;
      dy = -1 + rand() % 3;

      if(hardness[c->y+dy][c->x+dx] != 255 && (dx != 0 || dy != 0)){
        break;
      }
    }
    move_character(c,dx,dy);
  }

  return 0;
}


int dijkstra_move(monster_t *c){
  int i,j,i2,j2,dx,dy;
  int min_cost = INT_MAX;

  typedef struct move_location{
    int dx;
    int dy;
    int is_min;
  } move_location_t;

  move_location_t m[3][3];




    for(i2 = 0; i2 < 3; i2++){
      for(j2 = 0; j2 < 3; j2++){
        m[j2][i2].is_min = 0;
      }
    }

  if(c->tunneling == 0){

    for(i = -1;i < 2;i++){
      for(j = -1; j < 2; j++){
        if(non_tunneling_map[c->y+j][c->x+i] < min_cost && hardness[c->y+j][c->x+i] == 0){
              for(i2 = 0; i2 < 3; i2++){
      for(j2 = 0; j2 < 3; j2++){
        m[j2][i2].is_min = 0;
      }
    }
          min_cost = non_tunneling_map[c->y+j][c->x+i];
          m[j+1][i+1].dx = i;
          m[j+1][i+1].dy = j;
          m[j+1][i+1].is_min = 1;
        } else if(non_tunneling_map[c->y+j][c->x+i] == min_cost && hardness[c->y+j][c->x+i] == 0){
          m[j+1][i+1].dx = i;
          m[j+1][i+1].dy = j;
          m[j+1][i+1].is_min = 1;
        }
      }
    }
  } else {

    for(i = -1;i < 2;i++){
      for(j = -1; j < 2; j++){
        if(tunneling_map[c->y+j][c->x+i] < min_cost && hardness[c->y+j][c->x+i] != 255){
              for(i2 = 0; i2 < 3; i2++){
      for(j2 = 0; j2 < 3; j2++){
        m[j2][i2].is_min = 0;
      }
    }
          min_cost = tunneling_map[c->y+j][c->x+i];
          m[j+1][i+1].dx = i;
          m[j+1][i+1].dy = j;
          m[j+1][i+1].is_min = 1;


        } else if(tunneling_map[c->y+j][c->x+i] == min_cost && hardness[c->y+j][c->x+i] != 255){
          m[j+1][i+1].dx = i;
          m[j+1][i+1].dy = j;
          m[j+1][i+1].is_min = 1;
        }
      }
    }
  }

  while(1){
    i = rand() % 3;
    j = rand() % 3;


    if(m[j][i].is_min == 1){
      dx = m[j][i].dx;
      dy = m[j][i].dy;
      break;
    }
  }

  move_character(c,dx,dy);
  return 0;
}



int character_turn(monster_t *c,struct monster *mon, int num_mon){
  int i;
  int x_original = c->x;
  int y_original = c->y;


    if(c->erratic == 1 && (rand() % 2) == 1){
      random_move(c);
    } else if(in_same_room(c)) {
      LOS_move(c);
    } else {
      if(c->tel == 0){
        if(c->intel == 0){
          if(in_same_room(c)){
            LOS_move(c);
          } else {
            if(in_same_room(c)){
              LOS_move(c);
            } else {
              random_move(c);
            }
          }
        } else {
          random_move(c);
        }
      } else {
        if(c->intel == 0){
          LOS_move(c);
        } else {
          dijkstra_move(c);
        }
      }
    }



  for(i = 0; i < num_mon; i++){
    if(c->index != mon[i].index && c->x == mon[i].x && c->y == mon[i].y && mon[i].alive == 1){
      mon[i].x = x_original;
      mon[i].y = y_original;
    }
  }

  if(c->index != 0 && c->x == pc.x && c->y == pc.y){
    pc.hp -= roll_dice(c->dam);
    if(pc.hp <= 0){
      pc.hp = 0;
      pc.alive = 0;
      pc.race = 'X';
    } else {
      c->x = x_original;
      c->y = y_original;
    }
  }

  return 0;
}


int LOS_move(monster_t *c){

  int dx = 0;
  int dy = 0;

  if(c->y > pc.y){
    dy = -1;
  } else if (c->y < pc.y){
    dy = 1;
  }

  if(c->x > pc.x){
    dx = -1;
  } else if(c->x < pc.x){
    dx = 1;
  }

  if(dx != 0 || dy != 0){
    if(c->tunneling == 0 && hardness[c->y+dy][c->x+dx] == 0){
      move_character(c,dx,dy);
    }
    if(c->tunneling == 1){
      move_character(c,dx,dy);
    }
  }

  return 0;
}


int in_same_room(monster_t *c){
  int i,monster_room,pc_room;


  for(i = 0; i < num_rooms;i++){
    if(c->x <= room_arr[i].x_pos+room_arr[i].x_size && c->x >= room_arr[i].x_pos){
      if(c->y <= room_arr[i].y_pos+room_arr[i].y_size && c->y >= room_arr[i].y_pos){
        monster_room = i;
        break;
      }
    }
  }

  for(i = 0; i < num_rooms;i++){
    if(pc.x <= room_arr[i].x_pos+room_arr[i].x_size && pc.x >= room_arr[i].x_pos){
      if(pc.y <= room_arr[i].y_pos+room_arr[i].y_size && pc.y >= room_arr[i].y_pos){
        pc_room = i;
        break;
      }
    }
  }

  if(monster_room == pc_room){
    return 1;
  } else {
    return 0;
  }

}


int print_screen(struct monster *mon,int num_mon){
  /* Takes the buffer[][] array and prints it line by line to the terminal*/

  int i, j;


  for(i = 0;i < num_rooms; i++)
  {
    buffer_update(room_arr[i].x_pos,room_arr[i].x_size,room_arr[i].y_pos,room_arr[i].y_size,room_char);
  }


  for(i = 0; i < 80; i++){
    for(j = 0; j < 21; j++){
      if((hardness[j][i] == 0) && (buffer[j][i] != room_char))
      {
        buffer[j][i] = path_char;
      }
    }
  }


  for(i = 0;i < num_mon;i++){
    buffer_update(mon[i].x, 1, mon[i].y, 1, mon[i].race);
  }

  buffer[pc.y][pc.x] = pc.race;


  for(j = 0; j < 21; j++){
    for(i = 0; i < 80; i++){
      printf("%c",buffer[j][i]);
    }
    printf("\n");
  }
  printf("Gameplay text\n");
  printf("Gameplay text\n");

  return 0;
}






int place_monster_rand(monster_t *m){
  while(1){
    m->x = rand() % 80;
    m->y = rand() % 21;

    if(buffer[m->y][m->x] == room_char){
      break;
    }
  }

  return 0;
}



int assign_hardness(){

  int i,j;


  for(j = 0; j < 21; j++){
    for(i = 0; i < 80; i++){

      if(hardness[j][i] >= 0 && hardness[j][i] <= 84){
        weight[j][i] = 1;
      } else if(hardness[j][i] >= 85 && hardness[j][i] <= 170){
        weight[j][i] = 2;
      } else if(hardness[j][i] >= 171 && hardness[j][i] <= 254){
        weight[j][i] = 3;
      } else {
        weight[j][i] = -1;
      }

      non_tunneling_map[j][i] = hardness[j][i];
      tunneling_map[j][i] = hardness[j][i];
    }
  }
  return 0;
}

static int call_dijkstra(int from_x, int from_y, int tunneling)
{
  pair_t e1;

  e1[dim_y] = from_y;
  e1[dim_x] = from_x;


  dijkstra(e1, tunneling);

  return 0;
}



int place_pc_rand(){
  while(1){
    pc.x = rand() % 80;
    pc.y = rand() % 21;
  
    if(buffer[pc.y][pc.x] == room_char){
      buffer_update(pc.x, 1, pc.y, 1, pc.race);
      break;
    }
  }

  return 0;
}


int dijkstra(pair_t from, int tunneling)
{
  int i, j;
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  static uint32_t times_through = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, corridor_path_cmp, NULL);

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      if(tunneling == 0){
        if (hardness[y][x] != 255 && buffer[y][x] != rock_char) {
          path[y][x].hn = heap_insert(&h, &path[y][x]);
        } else {
          path[y][x].hn = NULL;
        }        
      } else if(tunneling == 1){
        if (hardness[y][x] != 255) {
          path[y][x].hn = heap_insert(&h, &path[y][x]);
        } else {
          path[y][x].hn = NULL;
        }        
      }
    }
  }


  while ((p = (corridor_path_t*) heap_remove_min(&h))) {
    p->hn = NULL;

    
   // if(times_through > 3*80*21){
     // break;
    //}



    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x] + 1].hn);
    }
    times_through++;
  }

  heap_delete(&h);
  
  if(tunneling == 0){
    for(i = 0; i < 80; i++){
      for(j = 0; j < 21; j++){
        non_tunneling_map[j][i] = path[j][i].cost;
      }
    }
  } else {
    for(i = 0; i < 80; i++){
      for(j = 0; j < 21; j++){
        tunneling_map[j][i] = path[j][i].cost;
      }
    }
  }

  return 0;
}



int save_dungeon(char *dungeon_file)
{
  /* Open the binary save file and save the game state to it */
  char semantic[] = "RLG327";
  int version = 0;
  unsigned char xpos[num_rooms],ypos[num_rooms],xlen[num_rooms],ylen[num_rooms];
  int size = 4*num_rooms+1694;
  int i, size_be;


  FILE *f;
  f = fopen(dungeon_file, "wb");

  fwrite(semantic,1,6,f);
  fwrite(&version,4,1,f);

  size_be = htobe32(size);

  fwrite(&size_be,4,1,f);
  fwrite(hardness,1,80*21,f);

  for(i = 0;i < (size-1694)/4; i++)
  {
    xpos[i] = (unsigned char) room_arr[i].x_pos;
    ypos[i] = (unsigned char) room_arr[i].y_pos;
    xlen[i] = (unsigned char) room_arr[i].x_size;
    ylen[i] = (unsigned char) room_arr[i].y_size;

    fwrite(&xpos[i],1,1,f);
    fwrite(&xlen[i],1,1,f);
    fwrite(&ypos[i],1,1,f);
    fwrite(&ylen[i],1,1,f);
  }



  fclose(f);


  return 0;
}


int load_dungeon(char *dungeon_file)
{
  /* Open the binary save file and load the game state from it */
  int i, j, version, size;
  char semantic[6];

  buffer_init();

  FILE *f;
  f = fopen(dungeon_file, "rb");
  
  if(!f){
    fprintf(stderr,"Couldn't open file to load\n");
    return -1;
  }


  fread(semantic,1,6,f);
  fread(&version,sizeof(version),1,f);
  fread(&size,sizeof(size),1,f);



  size = be32toh(size);
  num_rooms = (size-1694)/4;

  fread(&hardness,1,80*21,f);


  unsigned char xpos[num_rooms],ypos[num_rooms],xlen[num_rooms],ylen[num_rooms];

  
  for(i = 0;i < (size-1694)/4; i++)
  {

    fread(&xpos[i],1,1,f);
    fread(&xlen[i],1,1,f);
    fread(&ypos[i],1,1,f);
    fread(&ylen[i],1,1,f);

    room_arr[i].x_pos = (int) xpos[i];
    room_arr[i].y_pos = (int) ypos[i];
    room_arr[i].x_size = (int) xlen[i];
    room_arr[i].y_size = (int) ylen[i];

    buffer_update(xpos[i],xlen[i],ypos[i],ylen[i],room_char);
  }
  
  fclose(f);


  for(i = 0; i < 80; i++)
  {
    for(j = 0; j < 21; j++)
    {
      if((hardness[j][i] == 0) && (buffer[j][i] != room_char))
      {
        buffer[j][i] = path_char;
      }
    }
  }

  return num_rooms;
}


int rand_dungeon()
{
  /* Creates a new random dungeon and adds it to the screen buffer*/
  int i,j;

  buffer_init();
  generate_rooms();
  connect_rooms();
  
  for(i = 0; i < 80; i++){
    for(j = 0; j < 21; j++){
      if(buffer[j][i] == path_char || buffer[j][i] == room_char){
        hardness[j][i] = 0;
      }
    }
  }

  return 0;
}



int buffer_init(){
  /* Initializes the buffer array to be all rock_char*/

  int i, j;

   for(i = 0; i < 80; i++){
    for(j = 0; j < 21; j++){
      buffer[j][i] = rock_char;
      hardness[j][i] = 1 + rand() % 254;
    }
   }

  for(i = 0;i < 21; i++)
  {
      hardness[i][0] = 255;
      hardness[i][79] = 255;
      buffer[i][0] = '|';
      buffer[i][79] = '|';
  }

   for(i = 0; i < 80;i++)
   {
    hardness[0][i] = 255;
    hardness[20][i] = 255;
    buffer[0][i] = '=';
    buffer[20][i] = '=';
   }



   return 0;
}





int new_room(int i){
  /* Fills in the random size and position data for the room with the given
     index i*/
  
  room_arr[i].x_pos = rand() % 74;
  room_arr[i].y_pos = rand() % 16;
  room_arr[i].x_size = 4 + rand() % 11;
  room_arr[i].y_size = 3 + rand() % 12;

  return 0;
}



int buffer_update(int x_beg,int x_leng, int y_beg,int y_leng,char c){
  /* Adds the character specified by <char c> to the buffer in a rectangle
     between the x star/end position and y start/end position*/

  int i, j;

  for(i = x_beg;i < x_beg+x_leng; i++){
    for(j = y_beg;j < y_beg+y_leng; j++){
      buffer[j][i] = c;
    }
  }

  return 0;
}


int room_check(int i){
  /* Checks the room with the given index i to see if its placement in
     the buffer meets all the criteria the room must conform to*/

  int k, j, x, y, x_l, y_l;
  x = room_arr[i].x_pos;
  x_l = room_arr[i].x_size;
  y = room_arr[i].y_pos;
  y_l = room_arr[i].y_size;

  for(k = x-2; k < x+x_l+2; k++){
    for(j = y-2; j < y+y_l+2; j++){
      if(buffer[j][k] == room_char || hardness[j][k] == 255){
	     return 1;
      }
    }
  }

  if(x+x_l > 78 || y+y_l > 19){
    return 1;
  }

  return 0;
}



int generate_rooms(){
  /* Generates six rooms by calling new_room() to generate the room data
     and checks if the room is OK, with room_check().  If not OK, it
     re-generates the room until it gets one that meets the criteria.
     It then adds the characters in the correct place in the buffer array*/

  int i, check, x, x_l, y, y_l;

  for(i = 0;i < num_rooms; i++){
    while(check != 0){
      new_room(i);
      check = room_check(i);
    }

    x = room_arr[i].x_pos;
    x_l = room_arr[i].x_size;
    y = room_arr[i].y_pos;
    y_l = room_arr[i].y_size;

    buffer_update(x, x_l, y, y_l, room_char);
    check = 1;
   }

  return 0;
}



int connect_rooms(){
  /* Connects each room to the next room in the room_array with straight lines.
     Paths can go through other rooms but do not change the room symbols in the
     buffer*/

  int i, j, start_x, end_x, start_y, end_y, side;

  for(i = 0; i < num_rooms-1; i++){
    start_x = (2*room_arr[i].x_pos+room_arr[i].x_size)/2;
    end_x = (2*room_arr[i+1].x_pos+room_arr[i+1].x_size)/2;
    start_y = (2*room_arr[i].y_pos+room_arr[i].y_size)/2;
    end_y = (2*room_arr[i+1].y_pos+room_arr[i+1].y_size)/2;

    if(start_x < end_x){
      side = end_x;
      for(j = start_x; j <= end_x; j++){
	if(buffer[start_y][j] == rock_char){
	  buffer[start_y][j] = path_char;
	}
      }
    } else {
      side = start_x;
      for(j = end_x; j <= start_x; j++){
	if(buffer[end_y][j] == rock_char){
	  buffer[end_y][j] = path_char;
	}
      }
    }

    if(start_y < end_y){
      for(j = start_y; j <= end_y; j++){
	if(buffer[j][side] == rock_char){
	  buffer[j][side] = path_char;
	}
      }
    } else {
      for(j = end_y; j <= start_y; j++){
	if(buffer[j][side] == rock_char){
	  buffer[j][side] = path_char;
	}
      }
    }
  }

  return 0;
}
