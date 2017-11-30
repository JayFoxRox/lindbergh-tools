# (c)2012 Jannik Vogel

echo "Please wait, initializing JVS"

# First, reset out JVS devices

nodes=$(./jvs reset)
echo "Found $nodes node(s)"

# Tell user how this works

echo "Will start reading buttons in 2 seconds!"
echo "Hold down buttons now and do not release them until output is complete"
sleep 2

# Let's expect a request with 2 players, 2 bytes per play and 1 test button before that: 5 bytes

echo ""
for i in $(seq 0 $[8*5-1]); do
  echo "Button $i: " `./jvs digital 1 $i`
done
echo ""

# And done..

echo "Output complete, you can release the buttons now"

