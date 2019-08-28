all: traffic_controller.exe

traffic_controller.exe: traffic_controller.o
	 gcc -o traffic_controller.exe traffic_lights_template.o -lpthread

traffic_controller.o: traffic_lights_template.c
	 gcc -c traffic_lights_template.c -lpthread
     
clean:
	 rm traffic_lights_template.o traffic_controller.exe