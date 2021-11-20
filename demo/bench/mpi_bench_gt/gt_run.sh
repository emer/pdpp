#!/bin/bash
#

RUNXTERM=0
CMD="unset"
NP="unset"
MACHINEFILE="gtmachines"
RUNDIR=`pwd`
GTBIN="/scgt/linux-2.6/bin"
GTOFFSET="0"


printUsage()
{
    echo ""
    echo "USAGE:"
    echo "    $0 -np <numProcs> [-o offset][-machinefile <filename>] cmd"
    echo ""
    echo "    Runs specified command on numProcs after appending a process id"
    echo "    to the end of the command.  Processors are chosen from machinefile"
    echo "    (gtmachines by default)."
    echo ""
}

if [ $# -lt "2" ]
then
    printUsage;
    exit 1;
fi

while [ $# -gt 0 ];
do
    case $1 in
        -np)
            shift;
            NP=$1
            ;;
        -o)
            shift;
            GTOFFSET=$1
            ;;
        -machinefile)
            shift;
            MACHINEFILE=$1
            ;;
        *)
            CMD=$*
            break;
            ;;
    esac
    shift
done

if [ "$NP" = "unset" ];
then
    echo "ERROR: np not specified"
    printUsage;
    exit 2;
fi

if [ "$CMD" = "unset" ];
then
    echo "ERROR: No command specified"
    printUsage;
    exit 3;
fi



MACHINES=`cat $MACHINEFILE`
MACHINES=($MACHINES)     # make MACHINES an array

NUMMACHS=${#MACHINES[@]}


# initialize GT memory
# 1. write number of procs to first 32-bit value
# 2. clear $NP 32-bit values for synchronization (1 per process)
ssh ${MACHINES[0]} "cd $GTBIN && ./gtmem -c 100 -w 0 -o $GTOFFSET > /dev/null && ./gtmem -w $NP -o $GTOFFSET > /dev/null && ./gtmem -w 0 -o $((GTOFFSET+4)) -c $NP > /dev/null"

show=""; 

if [ $? != 0 ];
then
    echo FAILED initializing GT memory..
    echo exiting without launching command.
    exit 2;
fi

for ((i=0,j=0; i<$NP; i++,j++));
do
    if [ $j -ge $NUMMACHS ];   # then start over in machines array
    then
        j=0;
    fi

    MACH=${MACHINES[$j]}
    ((j++));
    GTUNIT=${MACHINES[$j]}

    CMDWID="$CMD $GTUNIT $GTOFFSET $i"  # command with ID appended
#    echo running \'$CMDWID\' on $MACH

    show=`echo $show $i:${MACH}_$GTUNIT`
 
    if [ $RUNXTERM == 1 ];
    then
#        echo xterm -e ssh $MACH "cd $RUNDIR; $CMDWID"
        XOPTIONS="-geometry 80x7 -sb -sl 500 -rightbar -fg black -bg #dddd80"
        xterm -T "$MACH: $CMDWID" $XOPTIONS -e ssh $MACH "cd $RUNDIR; $CMDWID"&  
    else
#        echo ssh $MACH "cd $RUNDIR; $CMDWID"
        ssh $MACH "cd $RUNDIR; $CMDWID" & 
    fi
done

echo running $CMD - $show

# wait until all children are done
wait

