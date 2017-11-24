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



native CreateInGameMaze(Float:origin_x, Float:origin_y, Float:origin_z, m_height, m_width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);
native CreateMazeBitmap(filename[], m_height, m_width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);
native CreateColorfulMaze(Float:origin_x, Float:origin_y, Float:origin_z, m_height, m_width, spaces_color = 0xFFFFFFFF, start_color = 0xFFFFFFFF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false);

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

#if !defined Streamer_IncludeFileVersion
	#include <streamer>
#endif

/*
In case more memory is needed.
#undef MALLOC_MEMORY
#define MALLOC_MEMORY							
*/
#define MAX_MAZE_XSIZE							(120)
#define MAX_MAZE_YSIZE							(120)
#define MAX_OBJECTS_PER_MAZE					(MAX_MAZE_XSIZE * MAX_MAZE_YSIZE)



#define UP 0     //-y
#define DOWN 1   //+y
#define LEFT 2   //-x
#define RIGHT 3  //+x

//m_randomEx(min, max); 
#if !defined m_randomEx
	#define m_randomEx(%0,%1) (random(%1-%0+1) + %0)
#endif

/*=================================================================================*/

enum T_MAZE_DATA
{
	in,  //Is this cell in the maze?
	up,  //Does the wall above this cell exist?
	left,//Does the wall to the left of this cell exist?
	prevx, 
	prevy,//The coordinates of the previous cell, used for backtracking.
};
static MAZE[MAX_MAZE_XSIZE][MAX_MAZE_YSIZE][T_MAZE_DATA];


stock CreateInGameMaze(Float:origin_x, Float:origin_y, Float:origin_z, m_height, m_width, walls_color = 0xFF000000, spaces_color = 0xFFFFFFFF, start_color = 0xFFFFFFFF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	if(use_backtrack && use_prim) return 0; 
	new numin=1;

	//Fixing width and height so the maze gets the surface area.

	new xsize = (m_height + 3) / 2;
	new ysize = (m_width + 3) / 2;
	//Initialize
	for(new x=0;x<xsize;x++)
	{
		for(new y=0;y<ysize;y++)
		{
			//The maze cells on the edges of the maze are "in" to provide padding. Otherwise, all maze cells are not in.
			MAZE[x][y][in]   = (x==0||x==xsize-1||y==0||y==ysize-1)?1:0;
			//All maze cells have all walls existing by default, except the perimeter cells.
			MAZE[x][y][up]   = (x==0||x==xsize-1||y==0)?0:1;
			MAZE[x][y][left] = (x==0||y==0||y==ysize-1)?0:1;
		}
	}

	new xcur=1, ycur=1;//start growing from the corner. It could theoretically start growing from anywhere, doesn't matter.
    MAZE[xcur][ycur][in] = 1;
    new whichway;
    new success;
    do 
    {
		if(no_dead_end == true)
		{
	        if( MAZE[xcur][ycur-1][in]	&&	MAZE[xcur][ycur+1][in] && MAZE[xcur-1][ycur][in] &&	MAZE[xcur+1][ycur][in]) 
	        {
	            //If at a dead end, randomly destroy a wall to make it not a dead end!
	            do 
	            {
	                success = 0;
	                whichway = random(4);
	                switch(whichway) 
	                {
		                case UP:
		                {
		                    if(MAZE[xcur][ycur][up]&&ycur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][up]=0;
		                    }
		                } 
		                case DOWN:
		                {
		                    if(MAZE[xcur][ycur+1][up]&&ycur!=ysize-2) {
		                        success=1;
		                        MAZE[xcur][ycur+1][up]=0;
		                    }
		                  }  
		                case LEFT:
		                {
		                    if(MAZE[xcur][ycur][left]&&xcur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][left]=0;
		                    }
		                  }  
		                case RIGHT:
		                {
		                    if(MAZE[xcur+1][ycur][left]&&xcur!=xsize-2) {
		                        success=1;
		                        MAZE[xcur+1][ycur][left]=0;
		                    }
		                   }
	                    
	                }
	            } while(!success);
	        }
	    }
	    if(use_backtrack == true)
	    {
	        while( MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
	                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in] ) {
	            //If all the neighbourhood cells are in, backtrack.
	            new xcur2=MAZE[xcur][ycur][prevx];
	            ycur=MAZE[xcur][ycur][prevy];
	            xcur=xcur2;
	        }
	    }
	    if(use_prim == true)
	    {
        do {
            //randomly find a cell that's in the maze
            xcur = m_randomEx(1, xsize-2);
            ycur = m_randomEx(1, ysize-2);
        } while(!MAZE[xcur][ycur][in] ||
                MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in]);	
    }
        do {
            //Randomly grow the maze if possible.
            success=0;
            whichway= random(4);
            switch(whichway) {
            case UP:{
                if(!MAZE[xcur][ycur-1][in]) {
                    success=1;
                    MAZE[xcur][ycur][up]=0;
                    MAZE[xcur][ycur-1][prevx]=xcur;
                    MAZE[xcur][ycur-1][prevy]=ycur;
                    ycur--;
                }
             }   
            case DOWN:{
                if(!MAZE[xcur][ycur+1][in]) {
                    success=1;
                    MAZE[xcur][ycur+1][up]=0;
                    MAZE[xcur][ycur+1][prevx]=xcur;
                    MAZE[xcur][ycur+1][prevy]=ycur;
                    ycur++;
                }
              }  
            case LEFT:{
                if(!MAZE[xcur-1][ycur][in]) {
                    success=1;
                    MAZE[xcur][ycur][left]=0;
                    MAZE[xcur-1][ycur][prevx]=xcur;
                    MAZE[xcur-1][ycur][prevy]=ycur;
                    xcur--;
                }
               } 
            case RIGHT:
            {
                if(!MAZE[xcur+1][ycur][in]) {
                    success=1;
                    MAZE[xcur+1][ycur][left]=0;
                    MAZE[xcur+1][ycur][prevx]=xcur;
                    MAZE[xcur+1][ycur][prevy]=ycur;
                    xcur++;
                }
            }
                
            }
        } while(!success);
        MAZE[xcur][ycur][in]=1;
        numin++; //Every iteration of this loop, one maze cell is added to the maze.
    } while(numin<(xsize-2)*(ysize-2));

    //Bitmap creation
    new width=(xsize-1)*2-1;
	new height=(ysize-1)*2-1;
	//Object 19790 5x5 units.

	for(new y = 0; y <= height - 1; y++)
	{
		for(new x = 0; x <= width - 1; x++)
		{
			if(x%2 == 1 && y%2 == 1)
			{
				if(x/2+1 == xcur && y/2+1 == ycur)
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = start_color);
				} 
				else
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][in]?origin_z-5:origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][in]?spaces_color : walls_color);
				}
			}
			else if(x%2 == 0 && y%2 == 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = walls_color);
			}
			else if(x%2 == 0 && y%2 == 1)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][left]? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][left]?walls_color : spaces_color);
			}
			else if(x%2 == 1 && y%2 == 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][up]? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][up]?walls_color : spaces_color);
			}
		}
	}
	printf("A %dx%d maze was successfully created in game.", height, width);
	return 1;
}

stock CreateColorfulMaze(Float:origin_x, Float:origin_y, Float:origin_z, m_height, m_width, spaces_color = 0xFFFFFFFF, start_color = 0xFFFFFFFF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	if(use_backtrack && use_prim) return 0; 
	new numin=1;

	new walls_color, count;
	//Fixing width and height so the maze gets the surface area.

	new xsize = (m_height + 3) / 2;
	new ysize = (m_width + 3) / 2;
	//Initialize
	for(new x=0;x<xsize;x++)
	{
		for(new y=0;y<ysize;y++)
		{
			//The maze cells on the edges of the maze are "in" to provide padding. Otherwise, all maze cells are not in.
			MAZE[x][y][in]   = (x==0||x==xsize-1||y==0||y==ysize-1)?1:0;
			//All maze cells have all walls existing by default, except the perimeter cells.
			MAZE[x][y][up]   = (x==0||x==xsize-1||y==0)?0:1;
			MAZE[x][y][left] = (x==0||y==0||y==ysize-1)?0:1;
		}
	}

	new xcur=1, ycur=1;//start growing from the corner. It could theoretically start growing from anywhere, doesn't matter.
    MAZE[xcur][ycur][in] = 1;
    new whichway;
    new success;
    do 
    {
		if(no_dead_end == true)
		{
	        if( MAZE[xcur][ycur-1][in]	&&	MAZE[xcur][ycur+1][in] && MAZE[xcur-1][ycur][in] &&	MAZE[xcur+1][ycur][in]) 
	        {
	            //If at a dead end, randomly destroy a wall to make it not a dead end!
	            do 
	            {
	                success = 0;
	                whichway = random(4);
	                switch(whichway) 
	                {
		                case UP:
		                {
		                    if(MAZE[xcur][ycur][up]&&ycur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][up]=0;
		                    }
		                } 
		                case DOWN:
		                {
		                    if(MAZE[xcur][ycur+1][up]&&ycur!=ysize-2) {
		                        success=1;
		                        MAZE[xcur][ycur+1][up]=0;
		                    }
		                  }  
		                case LEFT:
		                {
		                    if(MAZE[xcur][ycur][left]&&xcur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][left]=0;
		                    }
		                  }  
		                case RIGHT:
		                {
		                    if(MAZE[xcur+1][ycur][left]&&xcur!=xsize-2) {
		                        success=1;
		                        MAZE[xcur+1][ycur][left]=0;
		                    }
		                   }
	                    
	                }
	            } while(!success);
	        }
	    }
	    if(use_backtrack == true)
	    {
	        while( MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
	                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in] ) {
	            //If all the neighbourhood cells are in, backtrack.
	            new xcur2=MAZE[xcur][ycur][prevx];
	            ycur=MAZE[xcur][ycur][prevy];
	            xcur=xcur2;
	        }
	    }
	    if(use_prim == true)
	    {
        do {
            //randomly find a cell that's in the maze
            xcur = m_randomEx(1, xsize-2);
            ycur = m_randomEx(1, ysize-2);
        } while(!MAZE[xcur][ycur][in] ||
                MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in]);	
    }
        do {
            //Randomly grow the maze if possible.
            success=0;
            whichway= random(4);
            switch(whichway) {
            case UP:{
                if(!MAZE[xcur][ycur-1][in]) {
                    success=1;
                    MAZE[xcur][ycur][up]=0;
                    MAZE[xcur][ycur-1][prevx]=xcur;
                    MAZE[xcur][ycur-1][prevy]=ycur;
                    ycur--;
                }
             }   
            case DOWN:{
                if(!MAZE[xcur][ycur+1][in]) {
                    success=1;
                    MAZE[xcur][ycur+1][up]=0;
                    MAZE[xcur][ycur+1][prevx]=xcur;
                    MAZE[xcur][ycur+1][prevy]=ycur;
                    ycur++;
                }
              }  
            case LEFT:{
                if(!MAZE[xcur-1][ycur][in]) {
                    success=1;
                    MAZE[xcur][ycur][left]=0;
                    MAZE[xcur-1][ycur][prevx]=xcur;
                    MAZE[xcur-1][ycur][prevy]=ycur;
                    xcur--;
                }
               } 
            case RIGHT:
            {
                if(!MAZE[xcur+1][ycur][in]) {
                    success=1;
                    MAZE[xcur+1][ycur][left]=0;
                    MAZE[xcur+1][ycur][prevx]=xcur;
                    MAZE[xcur+1][ycur][prevy]=ycur;
                    xcur++;
                }
            }
                
            }
        } while(!success);
        MAZE[xcur][ycur][in]=1;
        numin++; //Every iteration of this loop, one maze cell is added to the maze.
    } while(numin<(xsize-2)*(ysize-2));

    //Bitmap creation
    new width=(xsize-1)*2-1;
	new height=(ysize-1)*2-1;
	//Object 19790 5x5 units.

	for(new y = 0; y <= height - 1; y++)
	{
		for(new x = 0; x <= width - 1; x++)
		{
			switch(count % 4)
			{
				case 0: walls_color = 0xFFFF0000;
				case 1: walls_color = 0xFFFFFF00;
				case 2: walls_color = 0xFF00FF00;
				case 3: walls_color = 0xFF0000FF;
			}
			if(x%2 == 1 && y%2 == 1)
			{
				if(x/2+1 == xcur && y/2+1 == ycur)
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = start_color);
					count++;
				} 
				else
				{
					SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][in]?origin_z-5:origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][in]?spaces_color : walls_color);
					count++;
				}
			}
			else if(x%2 == 0 && y%2 == 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), origin_z, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = walls_color);
				count++;
			}
			else if(x%2 == 0 && y%2 == 1)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][left]? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][left]?walls_color : spaces_color);
				count++;
			}
			else if(x%2 == 1 && y%2 == 0)
			{
				SetDynamicObjectMaterialText(CreateDynamicObject(19790, origin_x+(x*5), origin_y+(y*5), MAZE[x/2+1][y/2+1][up]? origin_z : origin_z-5, 0.0, 0.0, 0.0, -1, -1, -1, 700.0), 
						0, " ", .backcolor = MAZE[x/2+1][y/2+1][up]?walls_color : spaces_color);
				count++;
			}
		}
	}
	printf("A %dx%d maze was successfully created in game.", height, width);
	return 1;
}
stock CreateMazeBitmap(filename[], m_height, m_width, walls_color = 0x000000FF, spaces_color = 0xFFFFFFFF, start_color = 0xFF0000FF, bool:use_backtrack = true, bool:use_prim = false, bool: no_dead_end = false)
{
	if(use_backtrack && use_prim) return 0; //both can't be enabled simultaneously.
	new numin=1;
	new xsize = (m_height + 3) / 2;
	new ysize = (m_width + 3) / 2;
	//Initialize
	for(new x=0;x<xsize;x++)
	{
		for(new y=0;y<ysize;y++)
		{
			//The maze cells on the edges of the maze are "in" to provide padding. Otherwise, all maze cells are not in.
			MAZE[x][y][in]   = (x==0||x==xsize-1||y==0||y==ysize-1)?1:0;
			//All maze cells have all walls existing by default, except the perimeter cells.
			MAZE[x][y][up]   = (x==0||x==xsize-1||y==0)?0:1;
			MAZE[x][y][left] = (x==0||y==0||y==ysize-1)?0:1;
		}
	}

	new xcur=1, ycur=1;//start growing from the corner. It could theoretically start growing from anywhere, doesn't matter.
    MAZE[xcur][ycur][in] = 1;
    new whichway;
    new success;
    do 
    {
		if(no_dead_end == true)
		{
	        if( MAZE[xcur][ycur-1][in]	&&	MAZE[xcur][ycur+1][in] && MAZE[xcur-1][ycur][in] &&	MAZE[xcur+1][ycur][in]) 
	        {
	            //If at a dead end, randomly destroy a wall to make it not a dead end!
	            do 
	            {
	                success = 0;
	                whichway = random(4);
	                switch(whichway) 
	                {
		                case UP:
		                {
		                    if(MAZE[xcur][ycur][up]&&ycur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][up]=0;
		                    }
		                } 
		                case DOWN:
		                {
		                    if(MAZE[xcur][ycur+1][up]&&ycur!=ysize-2) {
		                        success=1;
		                        MAZE[xcur][ycur+1][up]=0;
		                    }
		                  }  
		                case LEFT:
		                {
		                    if(MAZE[xcur][ycur][left]&&xcur!=1) {
		                        success=1;
		                        MAZE[xcur][ycur][left]=0;
		                    }
		                  }  
		                case RIGHT:
		                {
		                    if(MAZE[xcur+1][ycur][left]&&xcur!=xsize-2) {
		                        success=1;
		                        MAZE[xcur+1][ycur][left]=0;
		                    }
		                   }
	                    
	                }
	            } while(!success);
	        }
	    }
	    if(use_backtrack == true)
	    {
	        while( MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
	                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in] ) {
	            //If all the neighbourhood cells are in, backtrack.
	            new xcur2=MAZE[xcur][ycur][prevx];
	            ycur=MAZE[xcur][ycur][prevy];
	            xcur=xcur2;
	        }
	    }
	    if(use_prim == true)
	    {
        do {
            //randomly find a cell that's in the maze
            xcur = m_randomEx(1, xsize-2);
            ycur = m_randomEx(1, ysize-2);
        } while(!MAZE[xcur][ycur][in] ||
                MAZE[xcur][ycur-1][in]&&MAZE[xcur][ycur+1][in]&&
                MAZE[xcur-1][ycur][in]&&MAZE[xcur+1][ycur][in]);	
    }
        do {
            //Randomly grow the maze if possible.
            success=0;
            whichway= random(4);
            switch(whichway) {
            case UP:{
                if(!MAZE[xcur][ycur-1][in]) {
                    success=1;
                    MAZE[xcur][ycur][up]=0;
                    MAZE[xcur][ycur-1][prevx]=xcur;
                    MAZE[xcur][ycur-1][prevy]=ycur;
                    ycur--;
                }
             }   
            case DOWN:{
                if(!MAZE[xcur][ycur+1][in]) {
                    success=1;
                    MAZE[xcur][ycur+1][up]=0;
                    MAZE[xcur][ycur+1][prevx]=xcur;
                    MAZE[xcur][ycur+1][prevy]=ycur;
                    ycur++;
                }
              }  
            case LEFT:{
                if(!MAZE[xcur-1][ycur][in]) {
                    success=1;
                    MAZE[xcur][ycur][left]=0;
                    MAZE[xcur-1][ycur][prevx]=xcur;
                    MAZE[xcur-1][ycur][prevy]=ycur;
                    xcur--;
                }
               } 
            case RIGHT:
            {
                if(!MAZE[xcur+1][ycur][in]) {
                    success=1;
                    MAZE[xcur+1][ycur][left]=0;
                    MAZE[xcur+1][ycur][prevx]=xcur;
                    MAZE[xcur+1][ycur][prevy]=ycur;
                    xcur++;
                }
            }
                
            }
        } while(!success);
        MAZE[xcur][ycur][in]=1;
        numin++; //Every iteration of this loop, one maze cell is added to the maze.
    } while(numin<(xsize-2)*(ysize-2));

    //Bitmap creation
    new width=(xsize-1)*2-1;
	new height=(ysize-1)*2-1;
	new Bitmap:bmap = Bitmap_Create(width, height);

	for(new y = 0; y <= height - 1; y++)
	{
		for(new x = 0; x <= width - 1; x++)
		{
			if(x%2 == 1 && y%2 == 1)
			{
				if(x/2+1 == xcur && y/2+1 == ycur) Bitmap_WriteCtx(bmap, x, y, start_color);
				else
				{
					Bitmap_WriteCtx(bmap, x, y, MAZE[x/2+1][y/2+1][in] ? spaces_color : walls_color); 
				}
			}
			else if(x%2 == 0 && y%2 == 0)
			{
				Bitmap_WriteCtx(bmap, x, y, walls_color); 
			}
			else if(x%2 == 0 && y%2 == 1)
			{
				Bitmap_WriteCtx(bmap, x, y, MAZE[x/2+1][y/2+1][left] ? walls_color : spaces_color); 
			}
			else if(x%2 == 1 && y%2 == 0)
			{
				Bitmap_WriteCtx(bmap, x, y, MAZE[x/2+1][y/2+1][up] ? walls_color : spaces_color); 
			}
		}
	}
	Bitmap_Write(bmap, filename);
	Bitmap_Destroy(bmap);
	printf("Bitmap image for the %dx%d maze was successfully saved to: %s.bmp", height, width, filename);
	return 1;

}
