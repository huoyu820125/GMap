if [ $(echo $0 | grep '^/') ]; then
        exePath=$(dirname $0)
else
        exePath=$(pwd)/$(dirname $0)
fi

if [ $# == 0 ]; then
	pid=$(pidof gmap)
	if [ -n "$pid"  ]; then
		kill $pid
		echo "kill gmap $pid"
	fi
	$exePath/fix.sh
        exit 1;
fi


if [ $# != 1 ]; then
        echo commond:stop.sh [cfgfile];
        exit 1;
fi

cfg=$1
echo kill gmap $cfg

echo "" >> $exePath/cluster
while read line
do
        #split string
        i=1
        while((1==1));do
                split=`echo $line|cut -d " " -f$i`
                if [ "$split" = "" ];then
                        break;
                fi
                if [ 1 = $i ];then
                        cpid=$split
                fi
                if [ 3 = $i ];then
			if [ "$cfg" = "$split" ]; then
				echo kill $cpid
				kill $cpid
			fi	
                fi
                ((i++))
        done
done < cluster
$exePath/fix.sh
