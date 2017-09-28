#######################################################################
#check param
if [ $# != 1 ]; then
        echo commond:run.sh [cfgfile];
        exit 1;
fi
exe=gmap
cfg=$1

if [ $(echo $0 | grep '^/') ]; then
        exePath=$(dirname $0)
else
        exePath=$(pwd)/$(dirname $0)
fi

#######################################################################
echo check run state......
pids="pidof "$exe
#read file by line
echo "" >> $exePath/cluster
while read line; do
	#split string
	i=1  
	while((1==1)); do
        	split=`echo $line|cut -d " " -f$i`  
        	if [ "$split" = "" ];then
			break;
		fi

		if [ 3 = $i ];then
			if [ $split = $cfg ];then
                        	running="false"
				runpids=$(pidof $exe)
				
				runpids="null null "$runpids
				j=1
                        	while((1==1));do
                                	pid=`echo $runpids|cut -d " " -f$j` 
                                	if [ "$pid" = "" ]; then 
                                	        break;  
                                	fi      
                                	if [ "$pid" = "$cpid" ]; then 
                                        	running="true"
                                        	break;  
                                	fi      
                                	((j++)) 
                        	done    
                        	if [ "$running" = "true" ]; then
					echo "Error:The $exe program aleary running:" $line
					exit 1;
				else
					echo $line already stop
                        	fi
			fi
		fi
		if [ 1 = $i ];then
			cpid=$split
			pids=$pids" -o "$split
		fi
		((i++))
	done  
done < cluster

#######################################################################
echo start run......

echo exePath=$exePath exe=$exe cfg=$cfg
nohup $exePath/$exe $cfg >$exePath/null 2>&1 &
#pid=$(eval $pids)
pid=$($pids)

if [ -n "$pid"  ]; then
	echo $pid $exePath $cfg >> $exePath/cluster
	echo "$exe($pid) is start"
	echo "log is in $exePath/log/$exe"
	$exePath/fix.sh
fi

