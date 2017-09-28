pids=$(pidof gmap)
pids="null null "$pids
echo "" >> cluster
echo "" > running
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
				echo $line >> running
			fi
			#break
                fi
                ((i++))
        done
done < cluster
rm -f cluster
mv running cluster
