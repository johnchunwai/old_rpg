REAME for RPG V2.2

To make the program work, create a directory structure like this.

<anything>----Bitmaps-----Characters-----<bmp files for character tile>
	|	  |______
	|	  |      Landscapes-----<bmp files for land tiles>
	|	  |______
	|	  |	 Monsters-------<bmp for monsters>
	|	  |______
	|	  |	 Backgrounds-----<bmp for battle background>
	|	  |______
	|	  	 WarCharacters----<bmp for character in battle>
	|-----Map----<dat files to store map data>
	|-----Map Editor----<resource files and exe file for map editor program>
	|-----Source-------<exe for the game program>

To run map editor, double click the map editor exe file in Map Editor folder.
To run RPG V2.1, double click the exe file in Source folder.

** The Map Editor program and folder is not necessary for the game to run properly.

IMPROVEMENT IN V2.1
--------------------
1) Added a whole NEW FEATURE---ACTIVE LEVEL-UP SYSTEM(ALS). With ALS, u're allowed to
   advance levels during battles. Whenever u kill an enemy, your exp will increase and
   if u've reach the point where u can level up, u'll level up rite away without
   having to wait for the end of the battle.
2) Allow multiple enemies in battle scene.
3) Fixes the bug where players will sometimes miss battle scenes when they are walking
   south.

KNOWN BUG
------------
1)If u ever try to press 'Alt-Tab' during game play and try to switch back to the game
  after it's minimized, u will HANG your system!! This is caused by other Windows program
  or Windows itself uses the same resources as directdraw surface, so the surface is lost.
  The fix for this will be implement in later versions.
2)The enemy data will output garbage value after it's dead. It's completely okay for this
  cuz those data are just for design purpose and debug purpose only. The release version
  will not allow user to view the status of the monsters.

CONTROL
---------
In World Map:
	Arrow Key ---- move the character
	'S' Key	  ---- show status of character
	'ESC' Key ---- quit game
In Battle:
	Control is listed on the screen but some functions (eg. magic) may not be
	available yet.
	There is a trick in battle that u can run away at any time of the battle by
	holding the 'Q' key and press any key listed for use in the battle screen.
	And next time if u meet the same monster(by name), its HP is the same as when
	u met it the last time.

Caution: Many functions are not implemented yet. If u don't get something
	working, that's becuz that function isn't implemented.