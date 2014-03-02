
alarmserver: alarmserver.c extsrc/mongoose.c extsrc/mongoose.h
	gcc -o alarmserver alarmserver.c extsrc/mongoose.c -g -O01 -Iextsrc -pthread