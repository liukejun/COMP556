***********************Teammates**********************
******************************************************
Junru Xia(jx17) & Kejun Liu(kl50)


***********************Note**********************
******************************************************
Under pingpong mode, when user specify x bytes to send. Our client will read x bytes data from file: root/John1.txt.

***********************Testing strategy***************
******************************************************
netcat on clear & web browser on local machine



***********************Test Cases(www mode)***********
******************************************************
Brief introduction:
1.Testing on web browser. Support both chrome and Safari when server runs on local machine.
2.Testing using netcat
We use netcat on clear for testing www mode.
We have index.html and John1.txt under path /storage-home/k/kl50/COMP556/A1/P1.2/root
Server running on ring.clear.rice.edu(port 18200)
Client running on glass.clear.rice.edu

========================Case 1(../ appears in path)=============================
On server, input as:
	 ./server_num 18200 www /storage-home/k/kl50/COMP556/A1/P1.2/root
On client, input as:
	printf "GET /../../index.html HTTP/1.0\r\n\r\n" | nc ring.clear.rice.edu 18200
Result on client:
	HTTP/1.1 400 Bad Request 
	Content-Type: text/html
=======================Case 2(request /index.html)===============================
On server, input as:
	./server_num 18200 www /storage-home/k/kl50/COMP556/A1/P1.2/root
On client, input as:
	printf "GET /index.html HTTP/1.0\r\n\r\n" | nc ring.clear.rice.edu 18200
Result on client: 
	HTTP/1.1 200 OK 
	Content-Type: text/html

	<!DOCTYPE html>
	<html lang="en">
	<title>Dummy  web server</title>
	<BODY BGCOLOR="grey">
  		<center>
    			<h1>Welcome to the dummy homepage!</h1>
    			<p>Here's Rice COMP556 Course website. It is very cool. Check <a 	href="https://www.clear.rice.edu/comp556/556.html">here</a>.</p>
  		</center>
	</body>
	</html>
====================Case 3(request nonexistent file)================================
On server, input as:
	./server_num 18200 www /storage-home/k/kl50/COMP556/A1/P1.2/root
On client, input as:
	printf "GET /nonexit.html HTTP/1.0\r\n\r\n" | nc ring.clear.rice.edu 18200
Result on client:
	HTTP/1.1 404 Not Found 
	Content-Type: text/html
======================Case 4(methods other than GET)================================
On server, input as:
	./server_num 18200 www /storage-home/k/kl50/COMP556/A1/P1.2/root
On client, input as:
	printf “POST/index.html HTTP/1.0\r\n\r\n" | nc ring.clear.rice.edu 18200
Result on client:
	HTTP/1.1 501 Not Implemented 
	Content-Type: text/html
======================Case 5(root directory missing)================================
On server, input as:
	./server_num 18200 www
Server displays error info:
	Root directory not found for www mode. Please specify the directory where the server should look for your documents.


***********************Test Cases(pingpong mode)***********
******************************************************
Brief introduction:
Test pingpong mode on clear
Server running on ring.clear.rice.edu(port 18200).
Client1 running on glass.clear.rice.edu.
Client2 running on sky.clear.rice.edu.
Client3 running on water.clear.rice.edu.
========================Case 1(short message on Client1)=============================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num ring.clear.rice.edu 18200 50 1200
Result on Client1:
	It takes 0.143ms per iteration on average to transmit message between client and server.

========================Case 2(long message on Client1)=================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num ring.clear.rice.edu 18200 65535 10000
Result on Client1:
	It takes 1.678ms per iteration on average to transmit message between client and server.
========================Case 3(size out of bound on Client1)=================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num ring.clear.rice.edu 18200 65536 10
Result on Client1:
	Message size should be between 10 and 65535!

========================Case 4(count out of bound on Client1)=================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num ring.clear.rice.edu 18200 60 10001
Result on Client1:
	Count should be between 1 and 10000!

========================Case 5(concurrent on Client1,2,3)=================
On server(ring), input as:
	./server_num 18200
On Client1(glass), input as:
	./client_num ring.clear.rice.edu 18200 65535 10000
On Client2(sky), input as:
	./client_num ring.clear.rice.edu 18200 65535 7000
On Client3(water), input as:
	./client_num ring.clear.rice.edu 18200 65535 9000
Result:
	Server receive messages from Client1, Client2, Client3 concurrently.
	Client1(glass): It takes 1.848ms per iteration on average to transmit message between client and server.
	Client2(sky): It takes 3.321ms per iteration on average to transmit message between client and server.
	Client3(water): It takes 1.864ms per iteration on average to transmit message between client and server.
===================Case 6(Client 1 send messages to unknown port)=================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num ring.clear.rice.edu 18002 60 10
Result on Client1:
	connect to server failed: Connection refused

===================Case 7(Client 1 send messages to unknown host)=================
On server, input as:
	./server_num 18200
On Client1, input as:
	./client_num unknown.clear.rice.edu 18200 60 10
Result on Client1:
	Segmentation fault (core dumped)