/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "jevil.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Jevil_ArcMain_Jevil0,
	Jevil_ArcMain_Jevil1,
	
	Jevil_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Jevil_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Jevil;

//Dad character definitions
static const CharFrame char_jevil_frame[] = {
	{Jevil_ArcMain_Jevil0, {  0,   0,  66, 112}, { 32, 107}}, //0 idle 1
	{Jevil_ArcMain_Jevil0, { 67,   0,  65, 112}, { 31, 107}}, //1 idle 2
	{Jevil_ArcMain_Jevil0, {133,   0,  65, 113}, { 31, 108}}, //2 idle 3
	{Jevil_ArcMain_Jevil0, {  0, 113,  67, 116}, { 31, 111}}, //3 idle 4
	{Jevil_ArcMain_Jevil0, { 71, 113,  67, 115}, { 31, 110}}, //4 idle 5
	
	{Jevil_ArcMain_Jevil1, {  0,   0,  59, 115}, { 33, 110}}, //5 left 1
	{Jevil_ArcMain_Jevil1, { 60,   0,  61, 115}, { 35, 110}}, //6 left 2
	
	{Jevil_ArcMain_Jevil1, {122,   0,  63, 107}, { 31, 102}}, //7 down 1
	{Jevil_ArcMain_Jevil1, {186,   1,  62, 108}, { 30, 103}}, //8 down 2
	
	{Jevil_ArcMain_Jevil1, {  0, 116,  64, 122}, { 31, 115}}, //9 up 1
	{Jevil_ArcMain_Jevil1, { 65, 116,  66, 122}, { 32, 115}}, //10 up 2
	
	{Jevil_ArcMain_Jevil1, {132, 110,  65, 114}, { 27, 109}}, //11 right 1
	{Jevil_ArcMain_Jevil0, {139, 114,  64, 114}, { 28, 109}}, //12 right 2
};

static const Animation char_jevil_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  4,  4,  0,  1,  2,  3,  4,  4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
};

//Dad character functions
void Char_Jevil_SetFrame(void *user, u8 frame)
{
	Char_Jevil *this = (Char_Jevil*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_jevil_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Jevil_Tick(Character *character)
{
	Char_Jevil *this = (Char_Jevil*)character;
	
	//Camera stuff
	if ((stage.flag & STAGE_FLAG_JUST_STEP) && stage.song_step >= 912)
	{
		this->character.focus_x = FIXED_DEC(4,1);
		this->character.focus_y = FIXED_DEC(-74,1);
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Jevil_SetFrame);
	Character_Draw(character, &this->tex, &char_jevil_frame[this->frame]);
}

void Char_Jevil_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Jevil_Free(Character *character)
{
	Char_Jevil *this = (Char_Jevil*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Jevil_New(fixed_t x, fixed_t y)
{
	//Allocate jevil object
	Char_Jevil *this = Mem_Alloc(sizeof(Char_Jevil));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Jevil_New] Failed to allocate jevil object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Jevil_Tick;
	this->character.set_anim = Char_Jevil_SetAnim;
	this->character.free = Char_Jevil_Free;
	
	Animatable_Init(&this->character.animatable, char_jevil_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 9;
	
	this->character.focus_x = FIXED_DEC(24,1);
	this->character.focus_y = FIXED_DEC(-66,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\JEVIL.ARC;1");
	
	const char **pathp = (const char *[]){
		"jevil0.tim", //Jevil_ArcMain_Jevil0
		"jevil1.tim", //Jevil_ArcMain_Jevil1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
