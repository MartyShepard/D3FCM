# D3FCM
 Doom3 Port For Classic Mods
DOOM3 For Classic Mods (D3FCM)

Bumped to v1.3.3

Doom3 for Classic Mods. Doom 3 Port Updated for Windows 7 - 10. 
Support

* Widescreen Resolutions
* Aspect Ratio
* Nativ Borderless Mode
* Original OpenAL Sound
* Correct Grafik Memory Read
* Console High and Font Size
* Mod Loading outside from D3 

Support "D3 v1.3 Mods on D3 v1.31"

Install:
Copy Doom3.exe oder copy and Rename to your Doom3
Copy base\* to your Doom3\Base directory
Copy d3xp\* to your Doom3\d3xp directory



Engine Features 1.3.3 #########################################################################
# Full Path Support for Mods
 Ability to use Fullpath: "L:\Sortet Games\Doom 3X\xyz\Mods SP"
	Remember in this Subdirectory must be contain a gamename how:
	<foldername>. No need to copy a mod to the Doom3 Root Directory.
	Command:
	Doom3 +set fs_addonpath <Path> +set fs_game <gamename>
	
# Widescreen Resolution Support Enhanced
Adpated from dhewm3. Fully widescreen Resolution inc. Menu 4:3 Scaling
- r_aspectRatio (Updated and added from dhew3)	
  : 0 = 4:3/  1 = 16:9/ 2 = 16:10/ 3 = 5:3/ 4 = 5:4/ 5 = 14:9/ 6 = 21:9
    7 = 32:9/-1 = auto (guess from resolution)

  : Added Upto 40 Screenmodes.
    Note: I Adpated the Injection Code for the Resolutions from dhew3.
    Changed to support Mods. Not all. A few has own Technic. Venom,
    Hexen - Edge of Chaos.
    Supportet:
    - Classic Doom v1.31
    - Desolated: The Crying Fate
    - GRIMM: Quest For the Gatherers Key
    - Predator: Blade Yautja

- cmp_UseDefModeIndex
  : Use the standard Doom3 resolution method and turn off overwriting
    the new method. (Compatibility Settings)

- r_scaleMenuTo43
  : 1/0 (Scaling PDA, Crosshair to 4:3 format in Widescreen...)

- r_borderless
  : 1=Remove Border in Window Mode & Center game Screen

- r_mode -2
  : Default Mode, Desktop Resolution Mode and Fail Safe mode in
    Window Mode. It was never a good idea if the program crashes
    and you go back in full screen.

# Sound 
  : EAX identification changed so that the Official OpenAL Library
    can use it. Testet with 2.1.8.1 and 2.2.0.7. EAX is back :)
    If you don't like this, Delete or disable the openal.dll

  : Crashfix with OpenAL and Silent Wave files

  : Wave files are now more preferred than Ogg files.
    No more unnecessary compression necessary. 

# Filesystem
- fs_addonpath:
  : Full Path to a Mod eq L:\Modifications\Doom3 Mod\ without Gamename.

# Console changes
I changed the look a little  You can Scale the Font. increase the 
Consize  and chnage the Linewidth. I dont like too big fonts.

- con_fontsize_width :
  : Scale Console Font Width (8 is default and maximum, 3 Minimum)

- con_fontsize_height
  : Scale Console Font height (16 is default and maximum, 5 minimum)

  Note:
  Scale the font below 2 it's not really readable 

- con_sizeHeight
  : Set Console Height (480 is Default, 960 is Maximum, 50 is Minimum)

  Note: This calculation is based on the internal Doom 3 scaling of 640x480. 

- con_sizeLinewidth
  : Set Console Line Width (78 is Default and Minimum, 212 is Maximum)

- con_sizeLineScale
  : 1 = Scale the the console Font width with the line width.

  Note:
  If these option is deactivated, the font can be set independently of the line width. 

# Various
  The video memory can be read from Windows 7 and higher. Not more -1
  Videomemory Gui console Optic changes .
  Ability to use short filepath for fs_saves how ".\saves\"
  Loadlibrary get the required open dll fullpath.

  Thare are few fixes from Dhew3 and me. For mods how Blade Yautja
  i had to increase the Stack Size in the VC++ Project up to 24MB....
  if not set tzhe Mod freezed in the Gameplay.
   
  Console Cmd 'listmodes' updated for more info

  Various minor changes at the ID Edit Tools

  Partial Adpated from dhewm3. 
  Resolution Injection for Doom3, Roe and Mods

 		
Engine Features 1.3.2 ##########################################################################
- Widescreen Resolution Support
  Supported in the engine is the ability to render at higher resolutions
  up to 1920x1080. The resolutions are selectable right from the menu system.

- Crosshair
  The ability to disable the crosshair has been added to the system. The
  crosshair can be switched on or off by setting the console variable

"ui_showCrosshair" to 0 or 1.


   Windows Compatibility: This version of the engine is compatible with
   Windows 7 and newer Microsoft operating systems.


Port History:
ID Doom3 -> UQE (Ultimate Quake Engine) (2013) -> D3FCM (Doom3 For Classic Mods) (2021)

ID Doom 3
Source: https://github.com/TTimo/doom3.gpl

Jacques Krige
http://www.corvinstein.com 
Source: https://github.com/jacqueskrige/uqe-doom3

Marty Shepard
Source: https://github.com/MartyShepard/D3FCM
