/**

  The Mana World
  Copyright 2004 The Mana World Development Team
    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "main.h"
#include "map.h"
#include "being.h"
#include "log.h"
#include "./gui/chat.h"
#include "./gui/gui.h"
#include "./gui/inventory.h"
#include "./graphic/super_eagle.h"
#include "./graphic/graphic.h"
#include "./sound/sound.h"

char map_path[480];

extern TmwSound sound;

unsigned short dest_x, dest_y, src_x, src_y;
unsigned int player_x, player_y;
bool refresh_beings = false;
unsigned char keyb_state;
volatile int tick_time;
volatile bool refresh = false, action_time = false;

#define MAX_TIME 10000

/** Finite states machine to keep track
		of player walking status (2 steps linear
		prediction)
		0 = Standing
		1 = Walking without confirm packet
		2 = Walking with confirm */
char walk_status = 0;

void refresh_time(void) {
	tick_time++;
	if(tick_time==MAX_TIME)tick_time = 0;
}
END_OF_FUNCTION(refresh_frame);

void refresh_screen(void) {
	refresh = true;
}
END_OF_FUNCTION(refresh_screen);

int fps = 0, frame = 0;
/** lets u only trigger an action every other second 
			tmp. counts fps*/
void second(void) {
	action_time = true;
	fps = (fps + frame)/2;
	frame = 0;
}
END_OF_FUNCTION(second);

/** Return elapsed time (Warning: very unsafe
    function. It supposes the delay is always < 10s) */
short get_elapsed_time(short start_time) {
	if(start_time<=tick_time)
		return tick_time-start_time;
	else
		return tick_time+(MAX_TIME-start_time);
}

/** Main game loop */
void game() {
  status("INIT");
  do_init();
	init_graphic();
  while(state!=EXIT) {
    status("INPUT");
    do_input();
    if(refresh) {
      status("GRAPHIC");
      do_graphic();
			refresh = false;
    }		
    status("PARSE");
    do_parse();
    status("FLUSH");
    flush();
		if(fps>30)
			rest(15);
  }

  exit_graphic();
  close_session();
}

/** Initialize game engine */
void do_init() {

  if(!load_map(map_path))error("Could not find map file");
    
  sound.StartMOD("./data/sound/Mods/somemp.xm", -1);

	// Initialize timers
  tick_time = 0;  
	refresh = false;
  LOCK_VARIABLE(tick_time);
	LOCK_VARIABLE(refresh);
  install_int_ex(refresh_time, MSEC_TO_TIMER(1));
	install_int_ex(refresh_screen, /*MSEC_TO_TIMER(2000)*/BPS_TO_TIMER(75)); // Set max refresh rate to 75 fps
	install_int_ex(second, BPS_TO_TIMER(1));  

  // Interrupt drawing while in background
  #ifdef WIN32
    set_display_switch_mode(SWITCH_AMNESIA);
  #else
    set_display_switch_mode(SWITCH_PAUSE);
  #endif	

  // Initialize beings
  empty();
  player_node = create_node();
	player_node->id = account_ID;
  player_node->type = ACTION_NODE;
  set_coordinates(player_node->coordinates, x, y, 0);
	player_node->speed = 150;
  add_node(player_node);
  keyb_state = IDLE;

	remove("packet.list");
}

/** Clean the engine */
void do_exit() {
}

/** Check user input */
void do_input() {

  if(walk_status==0) {
		int x = get_x(player_node->coordinates);
		int y = get_y(player_node->coordinates);

    if(key[KEY_UP]) {
      if(get_walk(x, y-1)!=0) {
        walk(x, y-1, NORTH);
				walk_status = 1;
        src_x = x;
        src_y = y;
        player_node->action = WALK;
        player_node->tick_time = tick_time;
        set_coordinates(player_node->coordinates, x, y-1, NORTH);
      } else set_coordinates(player_node->coordinates, x, y, NORTH); 
    } else if(key[KEY_DOWN]) {
      if(get_walk(x, y+1)!=0) {
        walk(x, y+1, SOUTH);
				walk_status = 1;
        src_x = x;
        src_y = y;
        player_node->action = WALK;
        player_node->tick_time = tick_time;
        set_coordinates(player_node->coordinates, x, y+1, SOUTH);
      } else set_coordinates(player_node->coordinates, x, y, SOUTH); 
    } else if(key[KEY_LEFT]) {
      if(get_walk(x-1, y)!=0) {
        walk(x-1, y, WEST);
				walk_status = 1;
        src_x = x;
        src_y = y;
        player_node->action = WALK;
				player_node->tick_time = tick_time;
        set_coordinates(player_node->coordinates, x-1, y, WEST);
      } else set_coordinates(player_node->coordinates, x, y, WEST); 
    } else if(key[KEY_RIGHT]) {
      if(get_walk(x+1, y)!=0) {
        walk(x+1, y, EAST);
				walk_status = 1;
        src_x = x;
        src_y = y;
        player_node->action = WALK;
				player_node->tick_time = tick_time;
        set_coordinates(player_node->coordinates, x+1, y, EAST);
      } else set_coordinates(player_node->coordinates, x, y, EAST); 
    }  
	}

  if(player_node->action==STAND)
		if(key[KEY_LCONTROL]) {
			player_node->action = ATTACK;
			attack(x, y, get_direction(player_node->coordinates));
			player_node->tick_time = tick_time;
		}
	
  if(key[KEY_F1]) {
    save_bitmap("./Graphic/screenshot.bmp",double_buffer,NULL);
  } else if(key[KEY_F12]){
    sound.SetAdjVol( 1, 1, 1);
  } else if(key[KEY_F11]){
    sound.SetAdjVol(-1,-1,-1);
  }
	if(key[KEY_F5] && action_time==true) {
		if(player_node->action==STAND)
      action(2, 0);
		else if(player_node->action==SIT)
			action(3, 0);
		action_time = false;
	}
	if(key[KEY_F6] && action_time==true) {
		inventory.toggle();
		action_time = false;
	}

  if(key[KEY_ENTER]) {    
    if(strcmp(speech, "")!=0) {
       chatlog.chat_send(char_info[0].name, speech);
       strcpy(speech,"");
    }
  } 

	//rect(screen, (mouse_x/32)*32,(mouse_y/32)*32, ((mouse_x/32)+1)*32, ((mouse_y/32)+1)*32, makecol(255,255,255));

  if(mouse_b&2) {
    if(!show_npc_dialog) {
      int npc_x = mouse_x/32+map_x/32;
      int npc_y = mouse_y/32+map_y/32;
      int id = find_npc(npc_x, npc_y);
      if(id!=0) {
        WFIFOW(0) = net_w_value(0x0090);
        WFIFOL(2) = net_l_value(id);
        WFIFOB(6) = 0;
        WFIFOSET(7);
      }
    }
  }

  // Emotions, Skill dialog
	if(key_shifts & KB_ALT_FLAG && action_time == true) {
		if(player_node->emotion==0) {
      unsigned char emotion = 0;
      if(key[KEY_1])
        emotion = 1;
      else if(key[KEY_2])
        emotion = 2;
			else if(key[KEY_3])
        emotion = 3;
			else if(key[KEY_4])
        emotion = 4;
			else if(key[KEY_5])
        emotion = 5;
			else if(key[KEY_6])
        emotion = 6;
			else if(key[KEY_7])
        emotion = 7;
			else if(key[KEY_8])
        emotion = 8;
			else if(key[KEY_9])
        emotion = 9;
			else if(key[KEY_0])
        emotion = 10;
			if(emotion!=0) {
        WFIFOW(0) = net_w_value(0x00bf);
        WFIFOB(2) = emotion;
        WFIFOSET(3);
        action_time = false;
			}
		}

		if(key[KEY_S]) {
			show_skill_dialog = !show_skill_dialog;
			action_time = false;
		}
	}
	
  if(key[KEY_ESC])state = EXIT;

}

/** Calculate packet length */
int get_packet_length(short id) {
    int len = get_length(id);
  if(len==-1)len = RFIFOW(2);
  return len;
}

/** Parse data received from map server into input buffer */
void do_parse() {
  unsigned short id;
  char *temp;
  char direction;
  NODE *node = NULL;
  int len;


  // We need at least 2 bytes to identify a packet
  if(in_size>=2) {
    // Check if the received packet is complete
    while(in_size>=(len = get_packet_length(id = RFIFOW(0)))) {
      // Add infos to log file and dump the latest received packet
      char pkt_nfo[60];
      sprintf(pkt_nfo,"In-buffer size: %i Packet id: %x Packet length: %i",in_size,RFIFOW(0),len);
      /*log_hex("Packet", "Packet_ID", RFIFOW(0));
      log_int("Packet", "Packet_length", get_length(RFIFOW(0)));
      log_int("Packet", "Packet_in_size", RFIFOW(2));
      log_int("Packet", "In_size", in_size);
      FILE *file = fopen("packet.dump", "wb");
      for(int i=0;i<len;i++) {
        fprintf(file, "%x|%i|%c ", RFIFOB(i), RFIFOB(i), RFIFOB(i));
        if((i+1)%10==0)fprintf(file, "\n");
      }
      fclose(file);
			file = fopen("packet.list", "ab");
			fprintf(file, "%x\n", RFIFOW(0));
			fclose(file);*/

      // Parse packet based on their id                  
      switch(id) {
        // Received speech
        case 0x008d:
          temp = (char *)malloc(RFIFOW(2)-8);
          memset(temp, '\0', RFIFOW(2)-8);
          memcpy(temp, RFIFOP(8), RFIFOW(2)-8);
          node = find_node(RFIFOL(4));
          if(node!=NULL) {
            if(node->speech!=NULL) {
              free(node->speech);
              node->speech = NULL;
							node->speech_time = 0;
            }
            node->speech = temp;
            node->speech_time = SPEECH_TIME;
						node->speech_color = makecol(255, 255, 255);
            chatlog.chat_log(node->speech, BY_OTHER, gui_font);
          }
          break;
        case 0x008e:
        case 0x009a:
					if(RFIFOW(2)>4) {
						if(player_node->speech!=NULL) {
							free(player_node->speech);
							player_node->speech = NULL;
						}  
	
						player_node->speech = (char *)malloc(RFIFOW(2)-4);
						memset(player_node->speech, '\0', RFIFOW(2)-4);
						memcpy(player_node->speech, RFIFOP(4), RFIFOW(2)-5);
						
						player_node->speech_time = SPEECH_TIME;
						player_node->speech_color = makecol(255, 255, 255);
	
						if(id==0x008e)
							chatlog.chat_log(player_node->speech, BY_PLAYER, gui_font);
						else
							chatlog.chat_log(player_node->speech, BY_GM, gui_font);
          }
				break;
        // Success to walk request
        case 0x0087:
					if(walk_status==1)
              walk_status = 2;
          break;
        // Add new being
        case 0x0078:
					if(find_node(RFIFOL(2))==NULL) {
            node = create_node();
            node->id = RFIFOL(2);
						node->speed = RFIFOW(6);
						if(node->speed==0)node->speed = 150; // Else division by 0 when calculating frame
            node->job = RFIFOW(14);
            memcpy(node->coordinates, RFIFOP(46), 3);
            add_node(node);
					}
          break;
        // Remove a being
        case 0x0080:
					node = find_node(RFIFOL(2));
					if(node!=NULL) {
						if(RFIFOB(6)==1) { // Death
              if(node->job>110) {
                node->action = MONSTER_DEAD;
                node->frame = 0;
                node->tick_time = tick_time;
              } else remove_node(RFIFOL(2));
						} else remove_node(RFIFOL(2));
					}
          break;
        // Player moving
        case 0x01d8:
        case 0x01d9:
          node = find_node(RFIFOL(2));
          if(node==NULL) {
            node = create_node();
            node->id = RFIFOL(2);
            node->job = RFIFOW(16);
            memcpy(node->coordinates, RFIFOP(46), 3);
            add_node(node);
						node->tick_time = tick_time;
						node->speed = RFIFOW(6);
          }
          break;
        // Monster moving
        case 0x007b:          
          node = find_node(RFIFOL(2));
					if(node==NULL) {
						node = create_node();
						node->action = STAND;
						set_coordinates(node->coordinates, get_src_x(RFIFOP(50)), get_src_y(RFIFOP(50)), 0);
						node->id = RFIFOL(2);
						node->speed = RFIFOW(6);
						node->job = RFIFOW(14);
						add_node(node);
					}
          node->path = calculate_path(get_src_x(RFIFOP(50)),get_src_y(RFIFOP(50)),get_dest_x(RFIFOP(50)),get_dest_y(RFIFOP(50)));
          if(node->path!=NULL) {
            set_coordinates(node->coordinates, node->path->x, node->path->y, 0);
            node->action = WALK;
            node->tick_time = tick_time;
          }
          break;
        // Being moving
        case 0x01da:
          node = find_node(RFIFOL(2));
          if(node==NULL) {
            node = create_node();
            node->id = RFIFOL(2);
            node->job = RFIFOW(14);
            set_coordinates(node->coordinates, get_src_x(RFIFOP(50)), get_src_y(RFIFOP(50)), 0);
            add_node(node);
          }
          if(node->action!=WALK) {
          direction = get_direction(node->coordinates);
          node->action = WALK;
          if(get_dest_x(RFIFOP(50))>get_x(node->coordinates))direction = EAST;
          else if(get_dest_x(RFIFOP(50))<get_x(node->coordinates))direction = WEST;
          else if(get_dest_y(RFIFOP(50))>get_y(node->coordinates))direction = SOUTH;
          else if(get_dest_y(RFIFOP(50))<get_y(node->coordinates))direction = NORTH;
          else node->action = STAND;
					if(node->action==WALK)node->tick_time = tick_time;
          set_coordinates(node->coordinates, get_dest_x(RFIFOP(50)), get_dest_y(RFIFOP(50)), direction);          
          }
          break;
        // NPC dialog
        case 0x00b4:
          if(!strstr(npc_text, RFIFOP(8))) {
            strcat(npc_text, RFIFOP(8));
            strcat(npc_text, "\n");
            show_npc_dialog = 1;
          }
          break;
		case 0x01ee: //Get the items
			for(int loop = 0; loop < RFIFOW(4); loop++)
				{
				inventory.addItem(loop,RFIFOW(10+18*loop));
				}
			break;
		case 0x00a8: // could I use the item?
			// index RFIFOW(2)
			// succes or not RFIFOB(6);
			if(RFIFOB(6))
			{
			inventory.addItem(RFIFOW(2),RFIFOW(4));
			}
			break;
        // Warp
        case 0x0091:
          memset(map_path, '\0', 480);
          append_filename(map_path, "./data/map/", RFIFOP(2), 480);
          if(load_map(map_path)) {
            empty();
            player_node = create_node();
						player_node->job = 0;
						player_node->action = STAND;
						player_node->frame = 0;
            player_node->type = ACTION_NODE;
						player_node->speed = 150;
						player_node->id = account_ID;
						set_coordinates(player_node->coordinates, RFIFOW(18), RFIFOW(20), 0);
            add_node(player_node);
						walk_status = 0;
            // Send "map loaded"
            WFIFOW(0) = net_w_value(0x007d);
            WFIFOSET(2);
            while(out_size>0)flush();
          } else error("Could not find map file");
          break;
				// Skill ...
				case 0x011a:
					break;
				case 0x01a4:
					break;
				// Action failed (ex. sit because you have not reached the right level)
				case 0x0110:
					CHATSKILL action;

					action.skill   = RFIFOW(2);
					action.bskill  = RFIFOW(4);
					action.unused  = RFIFOW(6);
					action.success = RFIFOB(8);
					action.reason  = RFIFOB(9);
      		
					if(action.success != SKILL_FAILED &&
						action.bskill == BSKILL_EMOTE ) {
						printf("Action: %d/%d", action.bskill, action.success);
					}
					chatlog.chat_log(action, gui_font);
					break;
				// Update stat values
				case 0x00b0:
					switch(RFIFOW(2)) {
            case 5:
              char_info->hp = RFIFOW(4);
							break;
						case 6:
							char_info->max_hp = RFIFOW(4);
							break;
						case 7:
							char_info->sp = RFIFOW(4);
							break;
						case 8:
							char_info->max_sp = RFIFOW(4);
							break;
						case 11:
							char_info->lv = RFIFOW(4);
							break;
					}
					if(char_info->hp==0) {
						ok("Message", "You're now dead, press ok to restart");
						WFIFOW(0) = net_w_value(0x00b2);
						WFIFOB(2) = 0;
						WFIFOSET(3);
					}
					break;
				// Stop walking
				/*case 0x0088:  // Disabled because giving some problems
					if(node = find_node(RFIFOL(2))) {
						if(node->id!=player_node->id) {
              node->action = STAND;
              node->frame = 0;
              set_coordinates(node->coordinates, RFIFOW(6), RFIFOW(8), get_direction(node->coordinates));
						}
					}
          break;*/
				// Damage, sit, stand up
				case 0x008a:
					switch(RFIFOB(26)) {
            case 0: // Damage
							node = find_node(RFIFOL(6));
							if(node!=NULL) {
                if(node->speech!=NULL) {
                  free(node->speech);
                  node->speech = NULL;
                  node->speech_time = SPEECH_TIME;
                }
                node->speech = (char *)malloc(5);
                memset(node->speech, '\0', 5);
								if(RFIFOW(22)==0) {
									sprintf(node->speech, "miss");
									node->speech_color = makecol(255, 255, 0);
								} else {
                  sprintf(node->speech, "%i", RFIFOW(22));
									if(node->id!=player_node->id)node->speech_color = makecol(0, 0, 255);
									else node->speech_color = makecol(255, 0, 0);
								}
                node->speech_time = SPEECH_TIME;
								if(RFIFOL(2)!=player_node->id) {
                  node = find_node(RFIFOL(2));
									if(node!=NULL) {
										if(node->job<10)
                      node->action = ATTACK;
										else node->action = MONSTER_ATTACK;
										node->frame = 0;
									}
								}
							}
              break;
            case 2: // Sit
            case 3: // Stand up
              node = find_node(RFIFOL(2));
							if(node!=NULL) {
                if(RFIFOB(26)==2)
                  node->action = SIT;
                else if(RFIFOB(26)==3)
                  node->action = STAND;
							}
              break;
					}
					break;
				// Status change
				case 0x00b1:
					switch(RFIFOW(2)) {
            case 1:
							char_info->xp = RFIFOL(4);
							break;
						case 2:
							char_info->job_xp = RFIFOL(4);
							break;
						case 14:
							char_info->zeny = RFIFOL(4);
							break;
						// case 16 and 17 missing
					}
					break;
				// Level up
				case 0x019b:
					if(RFIFOW(2)==0) {
						ok("Info", "Level up!");
					} else if(RFIFOW(2)==1) {
						ok("Info", "Job level up!");
					}
					break;
				// Emotion
				case 0x00c0:
					node = find_node(RFIFOL(2));
					if(node) {
						node->emotion = RFIFOB(6);
						node->emotion_time = EMOTION_TIME;
					}
					break;
				// Update skill values
				case 0x0141:
					switch(RFIFOL(2)) {
            case 0x000d:
							char_info->STR = RFIFOL(6) + RFIFOL(10); // Base + Bonus
							break;
            case 0x000e:
							char_info->AGI = RFIFOL(6) + RFIFOL(10);
							break;
            case 0x000f:
							char_info->VIT = RFIFOL(6) + RFIFOL(10);
							break;
            case 0x0010:
							char_info->INT = RFIFOL(6) + RFIFOL(10);
							break;
            case 0x0011:
							char_info->DEX = RFIFOL(6) + RFIFOL(10);
							break;
            case 0x0012:
							char_info->LUK = RFIFOL(6) + RFIFOL(10);
							break;
					}
					break;
        // Manage non implemented packets
        default:
          //alert(pkt_nfo,"","","","",0,0);                  
          break;
      }
      //alert(pkt_nfo,"","","","",0,0);                  
            
      RFIFOSKIP(len);
    }
  }
}
