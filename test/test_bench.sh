#!/bin/bash

NM_DATA_CSV='nm.test.csv'
SL_DATA_CSV='sl.test.csv'
MP_DATA_CSV='mp.test.csv'
TH_DATA_CSV='th.test.csv'
AP_DATA_CSV='ap.test.csv'

AVE_TIME_DATA_PNG='avetime.test.png'
AVE_ERROR_DATA_PNG='aveerror.test.png'
# time.*.test.png
# error.*.test.png

PROGRAM_DIR='../bench/'
PROGRAM_EXE='bench.out'

REMOTE_DIR='~/git/yuu528/CSexpA-SHServer/src/'
REMOTE_NM_EXE='./shserver.out'
REMOTE_SL_EXE='./shserver_sl.out'
REMOTE_MP_EXE='./shserver_mp.out'
REMOTE_TH_EXE='./shserver_th.out'

SLEEP=1
SLEEP_RETRY=1
RETRY=10

PLOT_PTS=( 7 5 9 6 4 8 )

IP='192.168.100.1'
# IP='192.168.0.45'
PORT='10028'
PORT_APACHE='80'
SSH_USER='pi'

THREADS=(
	1
	2
	5
	10
	20
	50
	100
	200
	500
	1000
)

pid=''

function run_remote() {
	echo -n "Running on the remote server: $1 ... " >&2
	sshpass -p "$ssh_pass" ssh -o StrictHostKeyChecking=no "$SSH_USER"@"$IP" "$1"

	if [ $? -ne 0 ]; then
		if [ -z "$2" ] || [ "$2" != "true" ]; then
			echo 'Failed to connect to server or sshpass is not installed.'
			exit 1
		fi
	fi

	echo 'Done.' >&2
}

function run_remote_server() {
	pid=$(run_remote "cd $REMOTE_DIR; nohup $1 > /dev/null 2>&1 & echo \$!")
}

function kill_remote_server() {
	if [ -n "$pid" ]; then
		run_remote "kill $pid" true
	fi
}

function trap_sigint() {
	kill_remote_server
	exit
}

function print_msg() {
	sep=$(echo "$1" | wc -L | awk '{for(i=0;i<$1;i++) printf "="}')

	echo -e "\n$sep"
	echo "$1"
	echo -e "$sep\n"
}

function run_test() {
	TARGET_CSV="$1"
	TARGET_PROGRAM="$2"
	TARGET_PORT="$3"
	REMOTE_TARGET_EXE="$4"

	echo 'threads,time,error_ratio' > "$TARGET_CSV"

	print_msg "Testing $TARGET_PROGRAM"

	pid=''

	if [ -n "$REMOTE_TARGET_EXE" ]; then
		run_remote_server "$REMOTE_TARGET_EXE"
	fi

	sleep $SLEEP

	for thread in ${THREADS[@]}; do
		try=0
		while [ $try -lt $RETRY ]; do
			print_msg "Testing with $thread threads"

			result=$("$PROGRAM_DIR$PROGRAM_EXE" "$IP" "$thread" "$TARGET_PORT" 2>&1)

			exit_code=$?

			if [ $exit_code -eq 0 ]; then
				break
			else
				print_msg "Error! ($exit_code) Retrying..."

				if [ -n "$pid" ]; then
					kill_remote_server
					sleep $SLEEP
					run_remote_server "$REMOTE_TARGET_EXE"
				fi

				sleep $SLEEP_RETRY
			fi

			try=$(($try + 1))
		done

		if [ $try -ge $RETRY ]; then
			print_msg "Failed to get result for $thread threads"
			continue
		fi

		echo -n "$thread," >> "$TARGET_CSV"

		# get time
		# rev | cut -f1 | rev: get last field in the line
		echo "$result" | tee >(cat 1>&2) | tail -2 | head -1 | rev | cut -d' ' -f1 | rev | paste -sd, | tr -d '\n' >> "$TARGET_CSV"

		echo -n ',' >> "$TARGET_CSV"

		# error data rate (include size: 0)
		zero_b_cnt=$(echo "$result" | grep -c 'Recieved Size: 0 bytes')

		if [ $zero_b_cnt -eq 0 ]; then
			echo '0' >> "$TARGET_CSV"
		else
			echo "$zero_b_cnt / $thread" | bc -l >> "$TARGET_CSV"
		fi

		echo -e "\nZero Byte Count: $zero_b_cnt / $thread"

		sleep $SLEEP
	done

	kill_remote_server
}

function plot_data() {
	print_msg 'Plotting data'

	for basefile in "$NM_DATA_CSV" "$SL_DATA_CSV" "$MP_DATA_CSV" "$TH_DATA_CSV" "$AP_DATA_CSV"; do
		i=0
		plotquery=''

		for file in $(ls *."$basefile"); do
			if [[ "$file" =~ "ave." ]]; then
				continue;
			fi

			if [ $i -ne 0 ]; then
				plotquery+=', '
			fi

			plotquery+='"'"$file"'"'" using 1:2 with linespoints pt ${PLOT_PTS[$i]} title"'"'"Test $(( $i + 1 ))"'"'
			i=$(( $i + 1 ))
		done

		gnuplot <<EOF
set terminal pngcairo
set output "$(echo "time.$basefile" | sed 's/\.csv/\.png/')"

set mono

set datafile separator ","

set logscale x
set logscale y

set key left top

set xlabel "Client Threads"
set ylabel "Time (sec.)"

plot $plotquery
EOF

		gnuplot <<EOF
set terminal pngcairo
set output "$(echo "error.$basefile" | sed 's/\.csv/\.png/')"

set mono

set datafile separator ","

set logscale x

set key left top

set yrange [0:1]

set xlabel "Client Threads"
set ylabel "Error Rate"

plot $(echo "$plotquery" | sed 's/1:2/1:3/g')
EOF
	done
}

function plot_data_ave() {
	print_msg 'Plotting average data'

	titles=(
		'No multiplexing'
		'select'
		'fork'
		'pthread'
		'Apache'
	)
	i=0
	plotquery=''

	for file in "$NM_DATA_CSV" "$SL_DATA_CSV" "$MP_DATA_CSV" "$TH_DATA_CSV" "$AP_DATA_CSV"; do
		cat *."$file" | awk -F, '
		$1 ~ /[0-9]+/{
			time[$1] += $2;
			error [$1] += $3;
			count[$1] += 1
		}

		END{
			for(key in time) {
				print key "," (time[key] / count[$1]) "," (error[key] / count[$1]);
			}
		}' >"ave.$file"

		if [ $i -ne 0 ]; then
			plotquery+=', '
		fi

		plotquery+='"'"ave.$file"'"'" using 1:2 with linespoints pt ${PLOT_PTS[$i]} title "'"'${titles[$i]}'"'

		i=$(( $i + 1 ))
	done

	gnuplot <<EOF
set terminal pngcairo
set output "$AVE_TIME_DATA_PNG"

set mono

set datafile separator ","

set logscale x
set logscale y

set key left top

set xlabel "Client Threads"
set ylabel "Average Time (sec.)"

plot $plotquery
EOF

	gnuplot <<EOF
set terminal pngcairo
set output "$AVE_ERROR_DATA_PNG"

set mono

set datafile separator ","

set logscale x

set key left top

set yrange [0:1]

set xlabel "Client Threads"
set ylabel "Average Error Rate"

plot $(echo "$plotquery" | sed 's/1:2/1:3/g')
EOF
}

cd $(dirname $0)

echo 'Building benchmark program...'
pushd "$PROGRAM_DIR"
make

if [ $? -ne 0 ]; then
	echo 'Failed to build benchmark program'
	exit 1
fi
popd
echo 'Done'

trap trap_sigint SIGINT

onlyplot=false

# parse options
while getopts 'hpi:u:t:' opt; do
	case "$opt" in
		h)
			cat <<EOF
Usage: $0 [options]
Options:
	-h: Show this help message
	-p: Only plotting data
	-i: Override IP address (default: $IP)
	-u: Override SSH Username (default: $SSH_USER)
	-t: Override server port (default: $PORT)
EOF
			exit 0
			;;

		p)
			onlyplot=true
			;;

		i)
			IP="$OPTARG"
			;;

		u)
			SSH_USER="$OPTARG"
			;;

		t)
			PORT="$OPTARG"
			;;

		\?)
			exit 1
			;;
	esac
done

echo "Connect to $SSH_USER@$IP via SSH"
read -sp 'Server SSH Password: ' ssh_pass

# check if sshpass is installed and server is accessible
run_remote 'true'

i=1
if [ "$onlyplot" = false ]; then
	print_msg "Running Test #$i"

	for i in $(seq 5); do
		run_test "$i.$NM_DATA_CSV" 'normal' "$PORT" "$REMOTE_NM_EXE"
		run_test "$i.$SL_DATA_CSV" 'select' "$PORT" "$REMOTE_SL_EXE"
		run_test "$i.$MP_DATA_CSV" 'fork' "$PORT" "$REMOTE_MP_EXE"
		run_test "$i.$TH_DATA_CSV" 'pthread' "$PORT" "$REMOTE_MP_EXE"
		run_test "$i.$AP_DATA_CSV" 'apache' "$PORT_APACHE" ""
	done

	i=$(( $i + 1 ))
fi

plot_data
plot_data_ave
