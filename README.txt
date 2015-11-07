REAME for RPG V3.0

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
	|-----Map Editor----<exe file for map editor program>
	|-----Source-------<exe for the game program>

Remember to link to ddraw.lib
To run map editor, double click the map editor exe file in Map Editor folder.
To run RPG V3.0, double click the exe file in Source folder.

** The Map Editor program and folder is not necessary for the game to run properly.

IMPROVEMENT IN V3.0
--------------------
1)Added animated background in world map.
2)Corresponding changes in map editor to achieve the above task.
3)Improved the rate of meeting enemies in world map.
4)Added multi-character support.
5)Added dead character display in battle scene.
6)Added multi-character walking in world map.
7)Improved display quality and easy to understand battle scene messages.
8)Fixes the bug of quiting the application when a monster is killed.
9)Fixes the bug of garbage display of dead monster's status.
10)Use DEX value with an algorithm to determine the fighting sequence
   in each round in the battle.
11)Allows user to choose which enemy to attack during battle.
12)Player can use defense technique in battle.
13)Supports either 640x480x24 or 640x480x16 display mode.
14)Implementation of Magic during Battle added.
15)Characters in world map enlarged.
16)Fixes the bug of not being able to run on CD and every machine! YEAH!!

IMPROVEMENT IN V2.1
--------------------
1) Added a whole NEW FEATURE---ACTIVE LEVEL-UP SYSTEM(ALS). With ALS, u're allowed to
   advance levels during battles. Whenever u kill an enemy, your exp will increase and
   if u've reach the point where u can level up, u'll level up rite away without
   having to wait for the end of the battle.
2) Allow multiple enemies in battle scene.
3) Fixes the bug where players will sometimes miss battle scenes when they are walking
   south.

POSSIBLE FUTURE ENHANCEMENTS
----------------------------
1)Add item in battle scene.
2)Use a better approach to determine the action of monsters during battle
  rather than hard code it to attack.
3)Add a Transition scene from world map to battle.
4)Add Forest, Mountains and other landscapes.
5)Change the rate of encounter in different types of land.
6)Add animation to characters in world map scene.
7)Add sprite animations in battle scene.
8)Add a simple dungeon.
9)Add a simple village.(use a different scrolling technique there)
10)Add ability enhancements during level up.
11)Design items and magic.
12)Improve the input method.

KNOWN BUGS
----------
1)If u ever try to press 'Alt-Tab' during game play and try to switch back to the game
  after it's minimized, u will HANG your system!! This is caused by other Windows program
  or Windows itself uses the same resources as directdraw surface, so the surface is lost.
  The fix for this will be implement in later versions.

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
	When using Magic, 'R' key is for selecting the magic on the right and 'L' key
	is to select the magic on the left.

Caution: Many functions are not implemented yet. If u don't get something
	working, that's becuz that function isn't implemented.