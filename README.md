# DMelody
An elegant music player for Linux written in GTK.
A simple music player to browse and play your local music collection, it uses mpv as a backend.
It was tested on Debian 12.

# What you can do with DMelody
   1. Browse your local music collection.
   2. Manage your playlists.
   3. All the basic standard functionality.
   4. Option to automatically start playback on opening.
   5. Option to automatically minimize the window on opening.
   6. Option to show a little control bar to quickly play/pause/prev/next and quit.
   7. Support for command line arguments like: dmelody --toggle-pause, --quit etc.

# Installation/Usage
  1. Open a terminal and run:

		 ./configure
		 
		 (first make sure the 'configure' file is executable)

  2. if all went well then run:

		 make
		 sudo make install
		 
		 (if you just want to test it then run: make run)
		
  4. Run the application:
  
		 dmelody

# Screenshots

![Alt text](https://github.com/DiogenesN/dmelody/blob/main/dmelody.png)
 
![Alt text](https://github.com/DiogenesN/dmelody/blob/main/dmelody-settings.png)

That's it!

 Make sure you have the following packages installed:

		mpv
		gawk
		make
		perl
		pkgconf
		libgtk-4-dev

# Support

   My Libera IRC support channel: #linuxfriends
   
   Email: nicolas.dio@protonmail.com

