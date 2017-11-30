#!/bin/bash

# (c)2012 Jannik Vogel
#
# Now requires menu file as first parameter
#
# Recommended with: tput civis
#

headline="Lindbergh Multiboot (Version 1.00)"
help="Here could be a help message"

function menu 
{

  if [ $index -eq $selected ]; then

    # Check if start was pressed 
    if [ $start -gt 0 ]; then

      lastMenu="$menu"
      menu="$2"
      lastSelected=$selected
      selected=$[$3]
      ./sound sel.wav > /dev/null
      start=0
      redraw=1
    fi

  fi
  game "$1" " "

}

function game
{
  echo -n "     "
  if [ $index -eq $selected ]; then

    # Check if start was pressed 
    if [ $start -gt 0 ]; then
#      reset
#      echo ""
#      echo "  Loading: $1"

      # Sound yay

      ./sound sel.wav > /dev/null

      # Start the game (and make sure we don't start it a second time) and wait for it to exit
      
      start=0
      restart=1
      echo "$2" | bash > /dev/null

    fi

    echo -en "\e[1;31m"
    echo -n ">"
  else
    echo -n " "
  fi
  echo " $1"
  echo -en "\e[00m"
  index=$[$index+1]
  
}

function label
{
  echo ""
  echo -en "\e[1;30m"
  echo "   - $1"
  echo -en "\e[00m"
}

function checkError
{
  error=$?
  if [ $error -ne 0 ]; then
    reset
    echo ""
    echo "  Error: $error - $1"
    exit 1
  fi
}

# Preselect entry and set autoboot, also clear any active filters

restart=1
menu="$1"
selected=$[$2]
start=$[$3]

# Mainloop

while true; do

  # Start recounting

  index=0 

  # Draw menu head

  clear
  echo ""
  echo -en "\e[1;37m"
  echo "  $headline"
  echo -en "\e[00m"
  echo ""

  # Draw the list of games

  source "$menu"

  # Draw footer template
  
  echo ""
  echo ""
  echo -n "  "

  # Check if we have to restart and print that footer, or print the help message

  if [ $restart -gt 0 ]; then

    echo "Please wait while initializing JVS"

    # Reset JVS and check if an I/O board was found

    nodes=`./jvs reset`
    checkError "JVS Reset failed"

    if [ $nodes -lt 1 ]; then echo "Need at least one JVS node!"; exit 1; fi

    restart=0
    redraw=1

  else
    if [ "$help" != "" ]; then
      echo "($help)"
    fi
  fi

  # Check if we have to redraw

  if [ $redraw -gt 0 ]; then
    redraw=0
    continue
  fi


  # Time to release the buttons from last time

  usleep 150000

  # Loop until the selection is changed

  while true; do

    # Get JVS input

    # New code which doesn't hammer the JVS bus!

    buttons=$(seq 0 39) # Numbers 0 to 39
    buttons="$(echo $buttons)" # Replace newlines by space
    buttons=`./jvs digital 1 $buttons`
    checkError "Could not read buttons"
    # echo "$buttons"

    # Reset buttons
    up=0
    down=0
    start=0

    # Now add the buttons which influence the actions

    start=$[$start+${buttons:7:1}]
    down=$[$down+${buttons:14:1}]
    up=$[$up+${buttons:30:1}]

    # Calculate the new menu entry which is selected

    newSelected=$[$selected-$up+$down] 

    # Abort if the selection changed

    if [ $selected -ne $newSelected ]; then
      # Create a cool sound
      ./sound nav.wav > /dev/null &
      break
    fi

    # Abort if the game should be started now

    if [ $start -ne 0 ]; then
      break
    fi

    # Time we idle for a new button to be pressed

    usleep 33000

  done


  # Make the menu wrap

  if [ $newSelected -ge $index ]; then newSelected=0; fi
  if [ $newSelected -lt 0 ]; then newSelected=$[$index-1]; fi
  selected=$newSelected

done
