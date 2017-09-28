pids=$(pidof gmap)
echo running:$pids
pids="null null "$pids

if [ $(echo $0 | grep '^/') ]; then
        exePath=$(dirname $0)
else
        exePath=$(pwd)/$(dirname $0)
fi

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
			j=1
			running="false"
			while((1==1));do
				pid=`echo $pids|cut -d " " -f$j`
				if [ "$pid" = "" ]; then
					break;
				fi
				if [ "$pid" = "$cpid" ]; then
					running="true"
					break;
				fi
				((j++))
			done
			if [ "$running" = "false" ]; then
				echo $line already stop
			else
				echo $line
			fi
			#break
                fi
                ((i++))
        done
done < $exePath/cluster

