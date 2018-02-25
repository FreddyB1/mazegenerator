/*
			Maze in Game Generator

The code was originally written in c++.


//Purpy Pupple's amazing maze generator. 
//Released under the CC-BY-SA 3.0 License and the GFDL
//Available at: https://en.wikipedia.org/wiki/File:MAZE_30x20_DFS.ogv
I adapted it to PAWN and added some features.

Credits:

- ThreeKingz | ThePhenix
- Crayder
- Purpy Pupple for the original version in c++



Changelog:
		
	ThreeKingz(ThePhenix):
	- MAX height is 16382 and max width is 16382
	- Optimized code. Code stores all the required information per maze cell on a single PAWN cell(4 bytes) through bitwise operations, results:
	- Reduces amx size dramatically (2000x2000 maze used 94 007KB amx size, it uses 4013KB now)
	- Reduces compiling time dramatically
	- Minor optimizations, using bitwise AND instead of modulo to determine whether a number is odd or not


native CreateInGameMaze(Float:origin_x, Float:origin_y, Float:origin_z, length, width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);
native CreateMazeBitmap(const filename[], length, width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);
native CreateColorfulMaze(Float:origin_x, Float:origin_y, Float:origin_z, length, width, spaces_color = 0xFFFFFFFF, start_color = 0xFFFFFFFF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);

*/

#if defined _maze_generator_included
	#endinput
#endif
#define _maze_generator_included

#if !defined _samp_included
	#tryinclude <a_samp>
	#if !defined _samp_included
		#error  "<a_samp.inc> was not found. Make sure the include is correctly placed on your includes folder."
	#endif
#endif

#if !defined _INC_y_bitmap
	#tryinclude <YSI\y_bitmap>
	#if !defined _INC_y_bitmap
		#error  "<YSI\y_bitmap> was not found on your includes folder. Make sure you have the y_bitmap include by Y_Less on your includes folder."
	#endif
#endif

#if !defined _streamer_included
	#tryinclude <streamer>
	#if !defined _streamer_included
		#error  "<streamer> was not found on your includes folder."
	#endif
#endif

#define MAX_MAZE_XSIZE							(500)
#define MAX_MAZE_YSIZE							(500)
#define MAX_OBJECTS_PER_MAZE					(MAX_MAZE_XSIZE * MAX_MAZE_YSIZE)
#define START_X 								1
#define START_Y 								1

//m_randomEx(min, max); 
#if !defined m_randomEx
	#define m_randomEx(%0,%1) (random(%1-%0+1) + %0)
#endif

/*=================================================================================*/



new MAZE_A[MAX_MAZE_XSIZE][MAX_MAZE_YSIZE];

enum paths
{
	UP,
	DOWN,
	LEFT,
	RIGHT
}

//By ThreeKingz
//First 3 bits store boolean values in, up, left, 4th bit is unused, the rest of bits store prevx(14 bits) and prevy (14 bits)
#define SET(%0,%1,%2,%3)		SET_%0_%3(%1,%2)
#define SET_IN_1(%0,%1)			MAZE_A[%0][%1]|=1
#define SET_IN_0(%0,%1)			MAZE_A[%0][%1]&=~1
#define SET_UP_1(%0,%1)			MAZE_A[%0][%1]|=2
#define SET_UP_0(%0,%1)			MAZE_A[%0][%1]&=~2
#define SET_LEFT_1(%0,%1)		MAZE_A[%0][%1]|=4
#define SET_LEFT_0(%0,%1)		MAZE_A[%0][%1]&=~4
//SET_PREVX(x, y, value); value must not exceed 0x3FFF
#define SET_PREVX(%0,%1,%2)		MAZE_A[%0][%1]|=(%2<<4)
#define SET_PREVY(%0,%1,%2)		MAZE_A[%0][%1]|=(%2<<18)

#define GET(%0,%1,%2)			GET_%0(%1,%2)
#define GET_IN(%0,%1)			(MAZE_A[%0][%1]&1)
#define GET_UP(%0,%1)			(MAZE_A[%0][%1]&2)
#define GET_LEFT(%0,%1)			(MAZE_A[%0][%1]&4)
#define GET_PREVX(%0,%1)		((MAZE_A[%0][%1]&0x3FFFF)>>>4)
#define GET_PREVY(%0,%1)		(MAZE_A[%0][%1]>>>18)
//------------------------------------------------------------------------//


static stock internal_generate_maze(length, width, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	//At least one of the two methods must be chosen
	if(!use_backtrack && !use_prim) return 0;
	new numin=1;
	new const xsize = (length + 3) / 2;
	new const ysize = (width + 3) / 2;
	//Initialize
	for(new x=0;x<xsize;x++)
	{
		for(new y=0;y<ysize;y++)
		{
			MAZE_A[x][y] = 0;
			//The maze cells on the edges of the maze are "in" to provide padding. Otherwise, all maze cells are not in.
			if((x==0||x==xsize-1||y==0||y==ysize-1)) SET(IN,x,y,1);
			else SET(IN,x,y,0);

			//All maze cells have all walls existing by default, except the perimeter cells.
			if((x==0||x==xsize-1||y==0)) SET(UP,x,y,0);
			else SET(UP,x,y,1);

			if((x==0||y==0||y==ysize-1)) SET(LEFT,x,y,0);
			else SET(LEFT,x,y,1);
		}
	}
	//Generate
	new xcur=START_X, ycur=START_Y;
    SET(IN,xcur,ycur,1);
    new whichway = 0;
    new bool:success = false;
    do 
    {
		if(no_dead_end == true)
		{
            if(GET(IN,xcur,ycur-1) && GET(IN,xcur,ycur+1) && GET(IN,xcur-1,ycur) && GET(IN,xcur+1,ycur))
	        {
	            //If at a dead end, randomly destroy a wall to make it not a dead end!
	            do 
	            {
					success = false;
					whichway = random(4);
					switch(whichway) 
					{
						case UP:
						{
							if(GET(UP,xcur,ycur)&&ycur!=1) 
							{
								success=true;
								SET(UP,xcur,ycur,0);
							}
						} 
						case DOWN:
						{
							if(GET(UP,xcur,ycur+1)&&ycur!=ysize-2) 
							{
								success=true;
								SET(UP,xcur,ycur+1,0);
							}
						}  
						case LEFT:
						{
							if(GET(LEFT,xcur,ycur)&&xcur!=1) 
							{
								success=true;
								SET(LEFT,xcur,ycur,0);
							}
						}  
						case RIGHT:
						{
							if(GET(LEFT,xcur+1,ycur)&&xcur!=xsize-2) 
							{
								success=true;
								SET(LEFT,xcur+1,ycur,0);
							}
						}
					}
	            } 
	            while(!success);
	        }
	    }
	    if(use_backtrack == true)
	    {
            while(GET(IN,xcur,ycur-1) && GET(IN,xcur,ycur+1) && GET(IN,xcur-1,ycur) && GET(IN,xcur+1,ycur))
            {
	            //If all the neighbourhood cells are in, backtrack.
	            new xcur2=GET_PREVX(xcur,ycur);
	            ycur=GET_PREVY(xcur,ycur);
	            xcur=xcur2;
	        }
	    }
	    if(use_prim == true)
	    {
	        do 
	        {
	            //randomly find a cell that's in the maze
	            xcur = m_randomEx(1, xsize-2);
	            ycur = m_randomEx(1, ysize-2);
	        } 
        	while(!GET(IN,xcur,ycur) ||	GET(IN,xcur,ycur-1)&&GET(IN,xcur,ycur+1)&&	GET(IN,xcur-1,ycur)&&GET(IN,xcur+1,ycur));	
    	}
        do 
        {
			//Randomly grow the maze if possible.
			success=false;
			whichway= random(4);
			switch(whichway) 
			{
				case UP:
				{
					if(!GET(IN,xcur,ycur-1)) 
					{
						success=true;
						SET(UP,xcur,ycur,0);
						SET_PREVX(xcur,ycur-1,xcur);
						SET_PREVY(xcur,ycur-1,ycur);
						ycur--;
					}
				}   
				case DOWN:
				{
					if(!GET(IN,xcur,ycur+1)) 
					{
						success=true;
						SET(UP,xcur,ycur+1,0);
						SET_PREVX(xcur,ycur+1,xcur);
						SET_PREVY(xcur,ycur+1,ycur);
						ycur++;
					}
				}  
				case LEFT:
				{
					if(!GET(IN,xcur-1,ycur)) 
					{
						success=true;
						SET(LEFT,xcur,ycur,0);
						SET_PREVX(xcur-1,ycur,xcur);
						SET_PREVY(xcur-1,ycur,ycur);
						xcur--;
					}
				} 
				case RIGHT:
				{
					if(!GET(IN,xcur+1,ycur)) 
					{
						success=true;
						SET(LEFT,xcur+1,ycur,0);
						SET_PREVX(xcur+1,ycur,xcur);
						SET_PREVY(xcur+1,ycur,ycur);
						xcur++;
					}
				}
			}
		} 			
		while(!success);
		SET(IN,xcur,ycur,1);
		numin++; //Every iteration of this loop, one maze cell is added to the maze.
	} 
	while(numin<(xsize-2)*(ysize-2));
	return 1;
}


//Actual functions


stock CreateMazeBitmap(const filename[], length, width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	if(use_backtrack && use_prim) return 0; //both can't be enabled simultaneously.
	if(length >= MAX_MAZE_XSIZE || width >= MAX_MAZE_YSIZE) return 0;
	internal_generate_maze(length, width, use_backtrack, use_prim, no_dead_end);
    //Bitmap creation
	new Bitmap:bmap = Bitmap_Create(length, width);
	for(new y = 0; y <= width - 1; y++)
	{
		for(new x = 0; x <= length - 1; x++)
		{
			if(x&1 == 1 && y&1 == 1)
			{
				if(x/2+1 == START_X && y/2+1 == START_Y) Bitmap_WriteCtx(bmap, x, y, start_color);
				else
				{
					Bitmap_WriteCtx(bmap, x, y, GET(IN,x/2+1,y/2+1) ? spaces_color : walls_color); 
				}
			}
			else if(x&1 == 0 && y&1 == 0)
			{
				Bitmap_WriteCtx(bmap, x, y, walls_color); 
			}
			else if(x&1 == 0 && y&1 == 1)
			{
				Bitmap_WriteCtx(bmap, x, y, GET(LEFT,x/2+1,y/2+1) ? walls_color : spaces_color); 
			}
			else if(x&1 == 1 && y&1 == 0)
			{
				Bitmap_WriteCtx(bmap, x, y, GET(UP,x/2+1,y/2+1) ? walls_color : spaces_color); 
			}
		}
	}
	Bitmap_Write(bmap, filename);
	Bitmap_Destroy(bmap);
	printf("Bitmap image for the %dx%d maze was successfully saved to: %s", length, width, filename);
	return 1;

}

stock CreateInGameMaze(Float:origin_x, Float:origin_y, Float:origin_z, length, width, walls_color = 0xFF000000, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{

	if(use_backtrack && use_prim) return 0; //both can't be enabled simultaneously.
	if(length >= MAX_MAZE_XSIZE || width >= MAX_MAZE_YSIZE) return 0;
	internal_generate_maze(length, width, use_backtrack, use_prim, no_dead_end);
	//Object 19790 5x5 units.

	for(new y = 0; y <= width - 1; y++)
	{
		for(new x = 0; x <= length - 1; x++)
		{
			if(x&1 == 1 && y&1== 1)
			{
				if(x/2+1 == START_X && y/2+1 == START_Y)
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = start_color);
				} 
				else
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(IN,x/2+1,y/2+1)?origin_z-5:origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(IN,x/2+1,y/2+1)?spaces_color : walls_color);
				}
			}
			else if(x&1 == 0 && y&1== 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = walls_color);
			}
			else if(x&1 == 0 && y&1== 1)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(LEFT,x/2+1,y/2+1)? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(LEFT,x/2+1,y/2+1)?walls_color : spaces_color);
			}
			else if(x&1 == 1 && y&1== 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(UP,x/2+1,y/2+1)? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(UP,x/2+1,y/2+1)?walls_color : spaces_color);
			}
		}
	}
	printf("A %dx%d maze was successfully created in game.", length, width);
	return 1;
}



stock CreateColorfulMaze(Float:origin_x, Float:origin_y, Float:origin_z, length, width, spaces_color = 0xFFFFFFFF, start_color = 0xFFFFFFFF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	if(use_backtrack && use_prim) return 0; //both can't be enabled simultaneously.
	if(length >= MAX_MAZE_XSIZE || width >= MAX_MAZE_YSIZE) return 0;


	internal_generate_maze(length, width, use_backtrack, use_prim, no_dead_end);
	//Object 19790 5x5 units.
	new count = 0, walls_color = 0;
	for(new y = 0; y <= width - 1; y++)
	{
		for(new x = 0; x <= length - 1; x++)
		{
			switch(count % 4)
			{
				case 0: walls_color = 0xFFFF0000;
				case 1: walls_color = 0xFFFFFF00;
				case 2: walls_color = 0xFF00FF00;
				case 3: walls_color = 0xFF0000FF;
			}
			if(x&1 == 1 && y&1== 1)
			{
				if(x/2+1 == START_X && y/2+1 == START_Y)
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = start_color);
					count++;
				} 
				else
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(IN,x/2+1,y/2+1)?origin_z-5:origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(IN,x/2+1,y/2+1)?spaces_color : walls_color);
					count++;
				}
			}
			else if(x&1 == 0 && y&1== 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = walls_color);
				count++;
			}
			else if(x&1 == 0 && y&1== 1)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(LEFT,x/2+1,y/2+1)? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(LEFT,x/2+1,y/2+1)?walls_color : spaces_color);
				count++;
			}
			else if(x&1 == 1 && y&1== 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), GET(UP,x/2+1,y/2+1)? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = GET(UP,x/2+1,y/2+1)?walls_color : spaces_color);
				count++;
			}
		}
	}
	printf("A %dx%d maze was successfully created in game.", length, width);
	return 1;
}
