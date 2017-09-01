# EE120B_Summer2017
Sean Pickman
spick002@ucr.edu

https://youtu.be/SR6Lsp7ge14

Custom Project Title: JukeBox_Hero

High-Level Description:
The program starts the state machine in the initial state, which then transitions to a 2nd “initial” state, where the welcome screen is displayed.  The state transitions to an idle state, where it waits for an input from the buttons.  The A0 button increments though the list of songs.  The A1 button decrements through the list of songs.  The A2 button plays the selected song and transitions the state to a song state, where the song state transitions to different frequency states, where each of these are selected depending on the cnt variable which is incremented every time the frequency states transition back to the song state.  The songs can be slowed down by pressing the A0 button by use of the TimerSet function within the song state. The songs can be sped up by pressing the A1 button by use of the TimerSet function within the song state.  The songs can be stopped by pressing the A2 button, which also sets the LCD screen back to the welcome screen.

User Guide: 
	Rules:
		The 4th Button does not affect the Jukebox.
		Components must stay connected in original form of completed project
Controls:
The Jukebox displays a welcome screen on the LCD display when turned on.  By pressing the button second to the left, the user can run down the list of songs.  By pressing the button closest to the left, the user can run back up the list of songs.  Pressing the button third to the left plays the song currently on display.  Once this is done, three buttons can be used. By pressing the button second to the left, the user can slow down the speed of the song.  By pressing the button closest to the left, the user can speed up the song.  Pressing the button third to the left can be pressed to stop the song and return to the welcome screen.  Once the song completes, the LCD returns to the welcome screen.  The display contrast can be increased or decreased through use of potentiometer next to display.

Special Considerations:
-	Data Communication through sound waves is loud enough to hear
-	-speed that someone may press a button
-	Light display to change with songs


Technologies: Components Required:
1.  LCD Display
2. ATmega1284
3. WT-1205 speaker
4. 3 Buttons
5. Breadboard Power Supply Module
6. 104 Capacitors
7. 330 Ω resistors

			
Sources: None, except for myself.


