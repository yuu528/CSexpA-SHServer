#!/bin/bash

NM_DATA_CSV='nm.test.csv'
SL_DATA_CSV='sl.test.csv'
MP_DATA_CSV='mp.test.csv'
TH_DATA_CSV='th.test.csv'
AP_DATA_CSV='ap.test.csv'

TIME_DATA_PNG='time.test.png'
ERROR_DATA_PNG='error.test.png'

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

# IP='192.168.100.1'
IP='192.168.0.45'
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
	echo -n "Running on the remote server: $1 ..." >&2
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
	gnuplot <<EOF
set terminal pngcairo
set output "$TIME_DATA_PNG"

set mono

set datafile separator ","

set logscale x
set logscale y

set key left top

set xlabel "Client Threads"
set ylabel "Time (sec.)"

plot "$NM_DATA_CSV" using 1:2 with linespoints pt 2 title "No multiplexing", \
	"$SL_DATA_CSV" using 1:2 with linespoints pt 7 title "select", \
	"$MP_DATA_CSV" using 1:2 with linespoints pt 5 title "fork", \
	"$TH_DATA_CSV" using 1:2 with linespoints pt 13 title "pthread", \
	"$AP_DATA_CSV" using 1:2 with linespoints pt 1 title "Apache"
EOF

	gnuplot <<EOF
set terminal pngcairo
set output "$ERROR_DATA_PNG"

set mono

set datafile separator ","

set logscale x

set key left top

set yrange [0:1]

set xlabel "Client Threads"
set ylabel "Error Rate"

plot "$NM_DATA_CSV" using 1:3 with linespoints pt 2 title "No multiplexing", \
	"$SL_DATA_CSV" using 1:3 with linespoints pt 7 title "select", \
	"$MP_DATA_CSV" using 1:3 with linespoints pt 5 title "fork", \
	"$TH_DATA_CSV" using 1:3 with linespoints pt 13 title "pthread", \
	"$AP_DATA_CSV" using 1:3 with linespoints pt 1 title "Apache"
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

read -sp 'Server SSH Password: ' ssh_pass

# check if sshpass is installed and server is accessible
run_remote 'true'

trap trap_sigint SIGINT

run_test "$NM_DATA_CSV" 'normal' "$PORT" "$REMOTE_NM_EXE"
run_test "$SL_DATA_CSV" 'select' "$PORT" "$REMOTE_SL_EXE"
run_test "$MP_DATA_CSV" 'fork' "$PORT" "$REMOTE_MP_EXE"
run_test "$TH_DATA_CSV" 'pthread' "$PORT" "$REMOTE_MP_EXE"
run_test "$AP_DATA_CSV" 'apache' "$PORT_APACHE" ""

plot_data
