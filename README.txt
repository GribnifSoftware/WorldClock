                     World Clock 1.1 by Dan Wilga
                  Copyright ½ 1995, Gribnif Software
                         All Rights Reserved.

This program may be distributed freely, provided it and this text file are
unmodified.

See the end of this file for the revision history.

ChocolateWare
-------------

This program is ChocolateWare. If you find it useful, you are encouraged
to show your appreciation by sending a quantity of chocolate candy to the
address below. What kind and how much are up to you. Please just limit
yourself to chocolate, either with or without nuts.

  Dan Wilga
  Gribnif Software
  PO Box 779
  Northampton, MA  01061

     GEnie: GRIBNIF
  Internet: gribnif@genie.geis.com


Too Much Time On My Hands
-------------------------

I often need to call someone in some remote corner of the globe (I've
always hated that expression because, as you probably know, globes don't
have corners) and I find myself wondering if I'm going to be calling at
some ridiculously early (or late) time. Most of Europe is easy for me to
remember, but I've already found out that calling an Australian at 5:00 on
Saturday morning is not an acceptable thing to do.

I looked around and could not find a program that suited my needs: I wanted
a picture of the world, not just a text list of cities that are probably
nowhere near the place I really want to call. Of course, in order to keep
track of things accurately, a world clock also needs to have an idea about
daylight savings, and not just assume everyone uses the same method.

[Bored yet? If so, you can skip ahead to the next section. I would have
already, y'know.]

Well, I couldn't find anything that fit the bill, or even came close.  So,
I set about creating my own program. The first thing was to find a map. OK,
that was the easy part. Next, I wanted it to be scalable to any size, so I
used Convector Professional to vectorize the map and save it as a GEM file.

Fortunately, the map I used employs what is probably the most common method
for stretching the continents so that the map of our spherical globe will
end up flat: it's called Mercator's Projection. Unfortunately, it took more
hours of research in the local library than I care to mention to find out
what the mathematical relationship between latitude and map location are
when using this projection. But I finally found a simple formula.

Along the way, I also came across an invaluable book called "The
International Atlas" (see the bibliography at the end of this file), which
not only gives the coordinates of thousands of cities in the world, but
also gives very precise information about how each country (and even each
region within the country) handles daylight savings.

Now I just had to write a program to display the map in a useful way, and
pop a list of cities into a text file. World Clock is the result.


Setting it Up
-------------

World Clock works as a program or a desk accessory. It works in any
resolution, and probably with any graphics card. It also works with
multitasking environments like Geneva, MultiTOS, and MagiC.

To run it as a program, these files are necessary:

  WORLDCLK.PRG          Program
  WORLDCLK.RSC          Resource file
  WORLDCLK.DAT          Data file with cities and other preferences
  WORLDCLK.GEM          The map
  
There is also a second map called DETAILED.GEM. You can use this in place
of WORLDCLK.GEM by renaming WORLDCLK.GEM to something else (like
LOWREZ.GEM) and then renaming DETAILED.GEM to WORLDCLK.GEM. The
disadvantage to DETAILED.GEM is that it takes more memory to load (just
look at the difference in the file sizes) and whenever the window redraws,
it may redraw more slowly depending on your setup.

To run it as a desk accessory, you would simply rename WORLDCLK.PRG to
WORLDCLK.ACC and put it (along with the other files) in the same directory
as all your other desk accessories.

In order to prevent too much clutter, you can also put everything but the
WORLDCLK.ACC file into a folder called WORLDCLK. For instance, if your desk
accessories load from C:, you would put WORLDCLK.ACC in C:\, and put the
rest of the files in a folder called C:\WORLDCLK\.

The only functional difference between using World Clock as a desk
accessory versus using it as a program is that when used as a desk
accessory, World Clock will only allow you to add 10 places to the list in
one session.


Running it
----------

Once World Clock is running, there is one thing you need to do in order to
configure it for the first time:

                                IMPORTANT
                                ---------
                                
Before World Clock can be of any real use to you, you need to tell it where
you live. Do this by clicking on the "?" icon in the lower right of the
window. This will give you a popup menu. Select the "Edit Places" option.

In the "Edit Places" dialog, you will see some information about the
various places World Clock knows about by default. What you need to do is
choose a place which is in the same time zone as you currently are, and
uses the same method for daylight savings.

Scroll though the list of places by using the "+" and "-" buttons next to
the name of the current place. Don't worry if you live in Florida and the
closest thing you can find is New York. That's good enough for now.

To help you find a place which is close to you, here is a West to East
list of some of the cities, presented by region:

           USA: Honolulu, HI            Anchorage, AK
                Los Angeles, CA         Denver, CO
                Dallas, TX              Chicago, IL
                Indianapolis, IN        New York, NY
                
        Canada: Vancouver, BC           Edmonton, AB
                Toronto, ON             Montr‚al, QC
                Halifax, NS             St. John's, NF

        Europe: Reykjav¡k               Belfast
                Glasgow                 Madrid
                London                  Paris
                Amsterdam               Roma (Rome)
                Berlin                  Athinai (Athens)
                Helsingfors (Helsinki)  Kijev
                Moskva (Moscow)         Volgograd (Stalingrad)
                
     Australia: Perth                   Adelaide
                Melbourne               Brisbane
                Sydney

When you have found a place that is in the same time zone and uses the same
emthod for daylight savings, select the "Here" button. This indicates that
the place you chose should be used for computing other times in the world.
When finished, select the "Done" button.

If you can't find a place which is close to where you are, refer to the
section about the "Edit Places" dialog, for more information on how to add
new places to the list.

Now it gets interesting. On the map you will notice that a number of
locations have either a diamond (on a monochrome monitor) or a red plus
sign (on a color monitor). If you move the mouse arrow over one of these
markers and press the left mouse button, a dialog will pop up and tell you
what time it is in that place. If you either let go of the mouse or drag
the pointer away from a place, the popup will disappear.


More Features in the Main Window
--------------------------------

As you move the mouse around the main window with the left mouse button
pressed, you may also notice a coordinate display in the lower right of the
window. This tells you the latitude and longitude the mouse pointer is
currently at, and gets updated as long as the mouse button is pressed.
This feature can be turned off in the "Options" dialog (see below).

By default, World Clock shows the local time in the mover bar of the
window.  This can be turned off in the "Options" dialog, as well.

To conserve screen space, World Clock uses two icons in the lower right
corner of its window. You've already seen what the "?" gadget does (it
gives you a menu of choices), now it's time for you to try the resize
gadget to the far right.

To resize the window, grab the resize gadget and drag the outline which
appears. You will notice that the horizontal and vertical directions always
stay in a constant ratio. This is so that the map will not look distorted.
When you let go of the mouse, the map will redraw at the new size.


The Popup Menu
--------------

The popup menu, which is accessed by clicking on the "?" gadget in the
lower right of the window, contains several options:

  "Options..."
  ------------
  
    o  am/pm or 24-hour mode affects how time is displayed in the popup
       which appears next to a place, in the local time, and in the hours
       display. By default, World Clock tries to read an "_IDT" system
       "cookie" to determine whether 24-hour time is preferable for your
       country.

    o  Show Day in Popup controls whether or not the place popup will
       contain an extra line which describes whether time at the place the
       mouse is over is "Yesterday", "Today", or "Tomorrow".

    o  Show Hours gives you a bar at the top of the main window which
       contains the relative time at even increments along the map. Of
       course, many places differ from this number, but it is useful as a
       reference. Note that if the window is too small, some or all of the
       hours may not be visible because they do not fit.

    o  Show Coordinates, as stated before, controls whether or not the
       latitude and longitude of the current map location will be displayed
       when the left mouse button is pressed in the main window.

    o  Show Current Time controls the display of local time in the window's
       mover bar.
       
  "Edit Places..."
  ----------------
  
    This is probably the most important dialog. As you've seen already, it
    can be used to look at the list of places that World Clock is
    displaying.  It also lets you enter new places and edit existing ones.

    At the top of the dialog is the name of the current place. There are
    also "+" and "-" button which will cycle through the list of places.

    Below the name is where the longitude and latitude of the place can be
    changed. All coordinates are listed in degrees and minutes (60ths of a
    degree). This means that the largest number that can appear in the
    minutes section of a coordinate is 59. If you enter an incorrect value,
    you will be prompted to correct the error.

    Longitudes can either be east ("E") or west ("W") of 0 degrees, so you
    should select the appropriate button when entering a new coordinate.
    Otherwise, the marker for the place will not appear in the correct
    location. Latitudes can either be north ("N") or south ("S") of the
    equator. You can usually find the coordinates of a place by using an
    atlas or map. It is not necessary to be too accurate, unless you use
    World Clock with the main window at a large scale.

    Underneath the coordinates you should enter the number of hours and
    minutes that the local time of the place differs from Greenwich Mean
    Time (GMT). For example, New York is 5 hours 0 minutes to the West of
    GMT.

    Below this is where you can choose the type of daylight savings used at
    the place. When World Clock notices that the current day and time fall
    within the times of the year that daylight savings is present, one hour
    will be added to the local time of the place when it is displayed. If
    you are unsure what daylight savings method a particular place uses,
    you can just select "<None>" from the popup and no adjustment will be
    made. See the "WORLDCLK.DAT Format" section for information about how
    to add more daylight savings entries to the list.

    The "Attributes" section provides you with two options:

      o Here: In order for World Clock to be able to calculate the times of
        other places, it needs to know where you are located now. You
        can only select the "Here" button of one place in the list. Choose
        a place which is at least in the same time zone that you are in,
        and preferably one that uses the same method for daylight savings.

      o Show Marker: When this button is selected, a marker will be shown
        at the coordinates of the place on the main map window. Turning the
        marker off for a particular place can often help to make other
        markers more visible when they are tightly packed together.

    The list of cities can be sorted in one of three ways, either alphabet-
    ically by name, in a north to south direction, or in an east to west
    direction based on coordinates.

    The "New" button will add a new, blank place to the list. You must then
    edit the various parts of the place's description. When using World
    Clock as a desk accessory you are only allowed to add 10 places to the
    list in one session. You must "Save Settings" and reboot the computer
    in order to add more places. When using World Clock as a program, you
    can add as many places to the list in one session as memory permits.

    To remove the current place from the list, use the "Delete" button.
    Deleted places cannot be recovered, so use this button carefully.

    "Go To" will complete any changes you have made to the list, and return
    you to the main window. You will then be shown the location on the map
    of the place you were last looking at in the "Edit Places" dialog.

    The "Done" button completes changes to the list and returns you to the
    main window. If you have made any changes to the list when you quit
    World Clock, you will be prompted to save the changes before quitting.
    Note that you will only get this prompt if you are using World Clock as
    a program, not a desk accessory.

  "Set Date/Time..."
  ------------------
  
    This feature allows you to change your system clock's date and time.
    The _IDT cookie is used here, too, to determine what order the month,
    day, and year are displayed in the date.

  "Save Options"
  --------------
  
    This saves all configurable options, including the window size and
    position, and the list of places to WORLDCLK.DAT. If a change has been
    made to the list of places when you quit World Clock, you will be asked
    to save the changes.

  "About..."
  ----------
  
    This dialog gives information about the version of World Clock you are
    using.


Daylight Savings
----------------

  Many locations change their method of daylight savings every few years.
  For instance, the USA changed their start to the first Sunday in April
  just a few years ago, and many older references on this subject are
  still not updated. If you find that one of the default cities is
  incorrect, please let me know at the address above and I will correct it
  in a future version. 


WORLDCLK.DAT Format
-------------------

  This section describes the format of WORLDCLK.DAT. You do not need to
  read this section unless you either want to add a large number of places
  without using the "Edit Places" dialog, or you need to add more daylight
  savings descriptions.

  The file is a regular ASCII text file. Any line beginning with a * can be
  in any order. The commands which follow a * can be one of these:
  
    *Window     Window x, y, width, height
    *Sort       0=Name, 1=N->S, 2=W->E
    *am_pm      am/pm or 24-hour time
    *DispCoords Display mouse coordinates
    *DispDay    Show "Yesterday", "Today", "Tomorrow" in popup
    *DispHours  Show relative time at top of window
    *DispTime   Show current time in mover bar
    *Daylight   A description of daylight savings:
    
            ---- start savings ----   ----- end savings -----
    index   hour  which  month  day   hour  which  month  day   change   name

    index: a letter (case is unimportant) to be used in the flags field of
           places that use this method of daylight savings

    start savings: the day and time at which daylight savings begins
      hour: hour of the day, in 24-hour format
      which: if 0, then "day" is the date of the month when the change
                 takes place, regardless of weekday. For instance, if
                 "which" is 0 and "day" is 10, then the change occurs on
                 the 10th.
             if >0, then "day" is the day of the week of the change occurs
                 on, where Sunday is 0. When "which" is 1, that means the
                 first "day" of the month, 2 means the second "day" f the
                 month, etc.  See the examples below.
             if <0, then it indicates the last "day" of the month (-1), the
                 next-to-last (-2), etc.
      month: the month of the change, where 1=January
      day: see description of "which"

    end savings: the day and time at which daylight savings ends. In the
        northern hemisphere, this is usually later in the year than the
        starting date. In the southern hemisphere, like in Australia, it is
        earlier.

    change: the number of hours of offset from GMT during daylight savings. This is most likely
        always -1. 

    name: up to 16 characters

    Examples:

      i   h  w  m  d   h  w  m  d    c
      i   2 -1  4  0   2 -1 10  0   -1   Canada

      Daylight savings starts at 02:00 on the last Sunday in April.
      It ends at 02:00 on the last Sunday in October.

      i   h  w  m  d   h  w  m  d    c
      a   2 -1 10  0   2  1  3  0   -1   Australia
        
      Daylight savings starts at 02:00 on the last Sunday in October.
      It ends at 02:00 on the first Sunday in March.

      i   h  w  m  d   h  w  m  d    c
      k   0  0  4  1   0  0 10  1   -1   Former Soviet
        
      Daylight savings starts at 00:00 on April 1.
      It ends at 00:00 on October 1.

  Any lines not beginning with * are assumed to be places. These lines have
  the format:
  
    offset   flags   latitude longitude   name
    
    offset: the hours offset from GMT during standard time. Locations that
        are East of GMT have a negative offset.

    flags:
      the first flag is "a" if the a marker for the place is visible
          ("active"), otherwise it is a "-"
      the second flag is "h" if the place was chosen as "Here".
      the third flag is a daylight savings index, or "-" if none.

    latitude, longitude: these are in the format "degrees.minutes". A
        negative latitude is south of the equator, and a negative longitude
        is west of the prime meridian.

    name: up to 27 characters


Bibliography
------------

  "The International Atlas, World Latitudes, Longitudes and Time Changes",
    Thomas G. Shanks, ACS Publications, Inc., San Diego, 1985.


Revision History
----------------

    The "?" popup menu now works with MagiC.
    Exit buttons in dialogs look correct under MagiC.

  1.1:

    Canada is now set for the same daylight savings method as the USA and
        the Mexican Government. The description for New Zealand's daylight
        savings has been fixed a bit.

  1.0a:

    It will no longer crash when loaded on a machine that does not have a
        cookie jar (like TOS 1.0 or 1.2).
    If you run it without a WORLDCLK.DAT, entering the Edit Places dialog
        to add new places now works.
    12pm is now displyaed properly in the time that appears in the window's
        name bar
